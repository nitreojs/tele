# conventions

how fork code, commits and patches should look. the goal is a queue that reads well, reviews quickly and survives every upstream release with as few conflicts as possible.

## code

### where code goes

- new code goes into new files under `Telegram/SourceFiles/tele/`, named `tele_<feature>.cpp` and `tele_<feature>.h`, in `namespace Tele`. add every new file to the tele `nice_target_sources` block in `Telegram/CMakeLists.txt`.
- settings rows go into `settings/settings_tele.cpp`.
- upstream files only get hunks that hook into `Tele::`: an include, a call, an early return, a changed argument. a typical hunk:

  ```cpp
  #include "tele/tele_send_files.h"
  ...
  -		auto result = Core::App().settings().sendFilesWay();
  +		auto result = Tele::DefaultSendFilesWay();
  ```

- put new `#include "tele/..."` lines next to the tele includes already in the file, if there are any.
- prefer hooks over copies. call the upstream function and adjust its input or result from `Tele::`, or use an existing virtual hook, instead of copying an upstream function into fork code. a copy silently misses every upstream fix.
- avoid widely included upstream headers (`data/data_peer.h`, `history/history_item.h`, anything in the precompiled header). a new member or include there rebuilds most of the tree for everyone and conflicts more often. keep state in fork code, keyed by the upstream object if needed.
- changes to a submodule (`Telegram/lib_ui`, `Telegram/lib_base` and so on) are committed inside the submodule and exported into `patches/<submodule path>/`. only do that when the tdesktop side can't do it.

### style

follow upstream's style, since fork code sits next to it:

- tabs for indentation, the formatting of the surrounding upstream code.
- `[[nodiscard]]` on functions that return a value, `const auto` by default, `u"..."_q` for `QString` literals, `not_null<>` for pointers that can't be null.
- `rpl` for reactive values, `crl::async` / `crl::on_main` for threading, as upstream does.
- types and functions in `PascalCase`, locals in `camelCase`, members with a leading `_`.
- `#pragma once` in headers, namespaces closed with `} // namespace Tele`.
- **no comments.** names and small functions carry the meaning. the one exception is a short note on a real trap that the code can't express.

### options

every tele setting is a `base::options` option. adding a switch touches four places:

1. `tele/tele_options.h`: declare the id and the getter.

   ```cpp
   extern const char kOptionHdPhotos[];
   [[nodiscard]] bool HdPhotos();
   ```

   add an `rpl::producer` getter as well when the ui has to react while the option changes.
2. `tele/tele_options.cpp`: define the id and the option, in the anonymous namespace, and the getter.

   ```cpp
   const char kOptionHdPhotos[] = "tele-hd-photos";

   base::options::toggle HdPhotosOption({
   	.id = kOptionHdPhotos,
   	.name = "Send photos in HD by default",
   	.description = "Every send box starts with HD quality on, whatever was "
   		"picked last time.",
   	.defaultValue = true,
   });

   bool HdPhotos() {
   	return HdPhotosOption.value();
   }
   ```

   - ids are `tele-` plus kebab case, constants are `kOption` plus pascal case.
   - toggles default to off. only set `.defaultValue = true` when the maintainer asked for it.
   - `.restartRequired = true` when the change only takes effect after a restart. the settings row then offers one.
   - strings use `base::options::option<QString>`, with a `Set...` function and a `...Value()` producer next to the getter.
3. `settings/settings_tele.cpp`: show it on the right page, in a fitting group.

   ```cpp
   AddOptionToggle(builder, ::Tele::kOptionHdPhotos);
   ```

   an optional third argument adds search keywords, e.g. `AddOptionToggle(builder, id, { u"quality"_q })`. a row that only matters while another option is on takes that option as the fourth argument, e.g. `AddOptionToggle(builder, id, {}, ::Tele::kOptionQueueBehindMedia)`, and stays hidden until it's on.

   the pages are built by `FillInterface`, `FillChats`, `FillPrivacy`, `FillProfiles`, `FillBots` and `FillDebug`, the root page by `FillRoot`. each page is split into groups with `AddGroupTitle` and `AddGroupDivider`. put a new row into the group it belongs to, or start a new group. never append a loose row to the end of a page. the description and the optional keywords feed both searches, so add keywords for words people would search for that aren't in the name or description.
4. `tele/tele_settings_io.cpp`: add it to the `Specs()` table, so settings export and import know it. `Toggle(id)` for switches, `Choice`, `Text`, `People`, `Device` or `Internal` for the rest (see [architecture](architecture.md#settings-export-and-import)).

upstream call sites then only call the getter, e.g. `if (Tele::HdPhotos()) ...`.

off by default has to mean exactly upstream's behaviour. with the option off, the hunk must not change anything a user can see or anything that goes over the network.

### ui strings

- tele's strings are english in the code, translated through tele's own tables, never keys in `lang.strings`.
- write them in sentence case and pass every one through `Tele::Lower` (`tele/tele_lowercase.h`) where it's shown: it translates the exact english text, then applies the lowercase option. option names and descriptions shown through `AddOptionToggle` are already handled.
- give `Lower` the untouched english literal, so it can be looked up: `Lower(u"Deleted %1"_q).arg(name)`, never `Lower(u"Deleted %1"_q.arg(name))` or a sentence glued from pieces. one sentence is one literal with `%1`, `%2` for the values.
- text that depends on a number goes through `Tele::Plural` (`tele/tele_translations.h`) with both english forms: `Lower(Plural(u"%1 file"_q, u"%1 files"_q, count)).arg(count)`. the count decides the form in languages with more of them.
- names, server errors and other text that isn't tele's own english go through `Tele::LowerOnly`, which lowercases without looking anything up.
- new strings get their translations in `Telegram/Resources/tele/translations/<language>.json`. `python ci/translations.py --tdesktop <tree>` lists what each table lacks; a missing string just shows in english.
- a description only when the name can't fully explain the item, for options, rows and the texts under settings groups alike. if the name says it all, leave it out. when it's needed: one short sentence with the missing fact (a side effect, a condition, what gets sent where). no filler ("this lets you…", "when enabled…"), no restating the name, no lists of examples.
- call the app tele, and the service telegram.

## commits

- one line, conventional prefix, lowercase, imperative, no trailing period: `feat: send photos in hd by default`, `fix: ignore reply counters on scheduled messages`.
- no body, no trailers, no co-author lines, no tool or ai attribution.
- in the tdesktop tree the subject becomes the patch subject and its file name, and ci matches patches across releases by subject. describe what the user gets, and don't rename a released patch without a reason: a new subject shows up as one dropped and one new patch in the release notes.
- in the tele repository the same format applies, with the usual prefixes: `feat:`, `fix:`, `docs:`, `ci:`, `chore:`.

## patch hygiene

- one feature per patch. fixes to an unreleased feature go into its commit (`git commit --fixup`, `git rebase -i --autosquash`).
- small hunks survive upstream syncs. every extra context line is a chance to conflict.
- don't reformat, reorder or rename upstream code, and don't fix upstream whitespace in passing.
- keep the line endings of the file you edit. a windows checkout with `core.autocrlf=true` has crlf in the tdesktop tree: new files there get crlf too, and a file must never mix both.
- never edit a `.patch` file by hand. change the commits in the tdesktop tree and run `python tele.py export`.
- never bump `UPSTREAM` in a feature change. moving to a new upstream tag is its own change.
- never copy upstream's `.github/` or any other part of the upstream tree into this repository. its workflows would start running here.

## privacy rules for features

tele is a client for people who care what leaves their machine.

- no new third-party network requests without a setting that controls them. the setting defaults to off.
- every network request tele adds checks the launch flags at its lowest level: `Tele::OfflineForLaunch()` for third-party services, `Tele::ServerOffForLaunch()` for the tele server, `Tele::UpdatesOffForLaunch()` for github updates and what's new. update [launch flags](launch-flags.md) when a flag starts covering something new.
- never send account data to the tele server: no ids, usernames, contacts or messages. the client downloads a public list and does the matching locally. the only exception is a crash report, which leaves only after the user clicks send, shows its text first and lets the user untick their username.
- the tele server protocol is public in [tele server](server.md). a change to it is a documented, backwards compatible change there, not a hidden one in the code.
