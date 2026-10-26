# AGENTS.md

instructions for ai coding agents working on tele. humans: start with [CONTRIBUTING.md](CONTRIBUTING.md). the details behind every rule are in [docs/](docs/).

tele is telegram desktop kept as a patch queue: this repository holds `UPSTREAM` (a tdesktop release tag), `patches/tdesktop/*.patch` and `tele.py`. the code you edit lives in a separate tdesktop checkout (`../tdesktop` by default) where `python tele.py checkout` applied the queue as commits on a branch called `tele`. you change that tree with commits, then export them back into patches here.

## hard rules

1. **fork code goes into new files.** `Telegram/SourceFiles/tele/tele_<feature>.{cpp,h}` in `namespace Tele`, settings rows in `settings/settings_tele.cpp`. add new files to the tele `nice_target_sources` block in `Telegram/CMakeLists.txt`.
2. **minimal upstream hunks.** in upstream files only add an include and a call into `Tele::` (an early return, a changed argument, a hook). don't reformat, reorder, rename or copy upstream code. avoid widely included headers (`data/data_peer.h`, `history/history_item.h`, the precompiled header): editing them rebuilds most of the tree and conflicts more often.
3. **no code comments.**
4. **every ui string goes through `Tele::Lower`** (`tele/tele_lowercase.h`), which translates it. strings are english literals in sentence case, never `lang.strings` keys. `.arg` goes after `Lower` (`Lower(u"Sent to %1"_q).arg(name)`), one sentence is one literal, counts use `Lower(Plural(one, other, count)).arg(count)`, names and server text use `LowerOnly`. add ru/uk entries for new strings to `Telegram/Resources/tele/translations/` (`python ci/translations.py --tdesktop <tree>` lists the missing ones).
   **descriptions only when needed.** an option, row or settings group gets a description only when its name can't fully explain it. then one short sentence with the missing fact (a side effect, a condition, what gets sent where). no filler ("this lets you…", "when enabled…"), no restating the name, no lists of examples.
5. **every setting is a `base::options` option**, and a new one touches all four places:
   - declared in `tele/tele_options.h` and defined in `tele/tele_options.cpp` (`kOption...` id `"tele-..."`, `base::options::toggle` or `option<QString>`, getter);
   - shown on a settings page in `settings/settings_tele.cpp`, inside a group (`AddGroupTitle` / `AddGroupDivider`), never appended loose, with search keywords where the name and description don't cover what people would search for;
   - listed in the `Specs()` table in `tele/tele_settings_io.cpp` (`Toggle`, `Choice`, `Text`, `People`, `Device` or `Internal`), or settings export silently skips it.
6. **toggles default to off**, unless the maintainer says otherwise. off must behave exactly like upstream.
7. **respect the launch flags** in any new network use, at the lowest level of the request: `Tele::OfflineForLaunch()` for third-party services, `Tele::ServerOffForLaunch()` for the tele server, `Tele::UpdatesOffForLaunch()` for github updates. no third-party request without a setting. never send account data to the tele server.
8. **line endings.** keep the endings of the file you edit. in a windows checkout of the tdesktop tree (`core.autocrlf=true`) files are crlf: write crlf there, including new files, and never mix endings in a file.
9. **commit messages** are one line: `feat: ...` or `fix: ...` in the tdesktop tree (`docs:`, `ci:`, `chore:` also in this repository), lowercase, imperative, what the user gets. no body, no trailers, no co-author lines, no tool or ai attribution. the subject becomes the patch file name.
10. **never edit `.patch` files by hand.** change the commits in the tdesktop tree and run `python tele.py export`.
11. **never touch `.github/`** in a feature change, and never copy upstream's `.github/` (or any other part of the upstream tree) into this repository.
12. **never bump `UPSTREAM`** in a feature change. `export` writes the checked-out tag into it: if your tree is on another tag than `UPSTREAM`, stop.
13. **never commit a moved submodule pointer** in the tdesktop root. submodule changes are committed inside the submodule and export into `patches/<submodule path>/`.
14. **don't run git write commands in this repository or push** unless you were asked to. don't rewrite published history.

## workflow

1. **checkout.** `python tele.py checkout` (tdesktop on the `UPSTREAM` tag, queue applied as commits on `tele`). if it refuses, there is unexported work: report it instead of passing `--force`.
2. **edit and commit in the tdesktop tree.** a new feature is one new commit at the end of the queue. a fix to a patch that isn't released yet goes into that patch's commit (`git commit --fixup <commit>`, `git rebase -i --autosquash <UPSTREAM tag>`).
3. **build.** follow upstream's `docs/building-*.md` from the checked-out tag. configure with the developer's own `TDESKTOP_API_ID`/`TDESKTOP_API_HASH`, `-D DESKTOP_APP_DISABLE_AUTOUPDATE=ON`, and `TELE_BUILD` left at `0`. see [docs/development.md](docs/development.md#building).
4. **run and test.** launch the dev build, exercise the feature by hand with the option on and off. if you can't run a gui or log in, say so plainly and hand over exact steps to test; never claim a feature works because it compiles.
5. **export.** `python tele.py export` in this repository. `git status` here should show only the patch(es) of your change.
6. **patch row.** add `| [N](../patches/tdesktop/NNNN-subject.patch) | what it does | where to toggle |` to the table of the next release in `docs/releases.md` and update the patch count at the top. don't edit `docs/features.md`: it's generated from `docs/releases.md` with `python ci/features.py`. format and categories: [docs/development.md](docs/development.md#the-patch-row).
7. **dry-run apply** on a pristine upstream clone:

   ```
   git clone --depth 1 --branch <tag from UPSTREAM> --recurse-submodules --shallow-submodules https://github.com/telegramdesktop/tdesktop.git <scratch>
   python tele.py --tdesktop <scratch> apply
   ```

8. **pull request** against `next`, with `patches/` and `docs/releases.md`.

## known pitfalls

- **rpl copies `on_next` handlers before every call** (`lib_rpl/rpl/consumer.h`: `auto handler = this->_next;`). state kept in a `mutable` lambda resets on every event: keep it in `lifetime().make_state<...>()` or a member. never hand a widget a reference to something the handler captured by value: `const auto &st = st::x;` captured with `[=]` is a copy inside a temporary, and the widget ends up with a dangling style that crashes on paint. reference the global style (`st::x`) directly.
- **`history->localDraft()` is stale while a chat is open.** the compose field holds the live text. call `Core::App().materializeLocalDrafts()` before reading or changing drafts, or the field and the draft diverge (restoring text on top of it doubled it).
- **`RpWidget::setVisible` is `final`.** override `setVisibleHook(bool)` instead.
- **tl bare vs boxed.** `MTP_xxx(...)` returns the bare type, and its `write()` writes no constructor id. `MTPXxx` (boxed) `read()` expects one. when you store tl bytes, write and read the same form, or read bare with `MTPxxx().read(from, end, mtpc_xxx)`. mixing them made every stored rich draft unreadable after a restart.
- **widely included headers** rebuild most of the tree when touched. keep new state in `tele/` files.
- **`findChildren<T>()` needs `Q_OBJECT` in `T`.** most lib_ui widgets (`Ui::PopupMenu` and friends) have none, so qobject_cast falls back to the nearest base that has it (`RpWidget`) and returns any widget as a `T*`. walk `children()` with `dynamic_cast` instead.
- **lib_ui buttons react to space and enter** when focused. a focused send button sends on space: think about focus when you add buttons next to text input.
- **`export` rewrites all of `patches/`.** a commit inserted in the middle renumbers every later patch and breaks their links in `docs/releases.md`. new features go at the end.
- **renaming a commit subject renames its patch** and shows up in release notes as one dropped and one new patch.

## definition of done

- it builds, and the dev build runs.
- the feature was exercised by hand, with the option on and off.
- off by default, and off is exactly upstream's behaviour.
- the option is in `tele_options`, on a settings page in a group, has search keywords where useful, and is in the `Specs()` export table.
- network use respects the launch flags.
- `python tele.py export` done; only the patch(es) of this change differ.
- a row in `docs/releases.md` in the right format, and the patch count updated.
- the queue applies to a pristine upstream clone with `tele.py --tdesktop <clean clone> apply`.
