# contributing

thanks for looking. tele is telegram desktop kept as a queue of patches on top of upstream releases, so contributing works a bit differently from a normal fork: you change a patched tdesktop tree with commits, then export them back into `patches/`.

- [docs/architecture.md](docs/architecture.md): how the queue, `tele.py`, the fork code and ci fit together.
- [docs/development.md](docs/development.md): building, running a dev build safely, making a change.
- [docs/conventions.md](docs/conventions.md): code style, commits, patch hygiene, privacy rules.
- [docs/releasing.md](docs/releasing.md): how releases are built, signed and published (maintainers).
- [AGENTS.md](AGENTS.md): rules for ai coding agents. worth a read for humans too.

## what fits

- fixes for bugs in tele's patches, or in how they interact with upstream.
- conflict fixes when the queue stops applying to a new upstream release.
- new features that fit what tele is: client-side tweaks, privacy options, tools for power users and developers. each behind a setting that is off by default.
- docs fixes.

what doesn't fit:

- changes that would need big rewrites of upstream code. they break on every upstream release.
- features that send user data anywhere, or talk to new third-party services without a setting.
- bugs that upstream telegram desktop has too. report those [upstream](https://github.com/telegramdesktop/tdesktop/issues).

## proposing a feature

open an issue first, before writing code. describe what the user sees, where the setting would live (which page of settings → tele) and whether it touches the network. the maintainer decides whether it fits and what its default is. this saves you from building something that can't be merged.

## the development cycle

1. clone this repository and a tdesktop clone next to each other, then run `python tele.py checkout` on the `next` branch. see [getting the sources](docs/development.md#getting-the-sources).
2. build it with upstream's build docs for the tag in `UPSTREAM`, your own api id and hash, and `DESKTOP_APP_DISABLE_AUTOUPDATE=ON`. see [building](docs/development.md#building).
3. run the dev build with a test account or on the test server. see [running a dev build safely](docs/development.md#running-a-dev-build-safely).
4. make the change in the tdesktop tree, following [conventions](docs/conventions.md). a new setting is a `base::options` option in `tele_options`, a row in a group on a settings page, and an entry in the export table.
5. test it by hand in the dev build, with the setting on and off.
6. commit it in the tdesktop tree: one new commit per feature, or a fixup into the patch you're fixing. see [making a change](docs/development.md#making-a-change).
7. run `python tele.py export` in this repository.
8. add or update the patch's row in `docs/releases.md`. see [the patch row](docs/development.md#the-patch-row).
9. check that the queue applies to a clean upstream clone with `python tele.py --tdesktop <clean clone> apply`. see [proving the queue applies](docs/development.md#proving-the-queue-applies).
10. open a pull request against `next`.

## pull request checklist

- [ ] one feature or fix per pull request, one patch per feature.
- [ ] the diff only touches `patches/` and `docs/releases.md`, unless the change is about tooling or docs.
- [ ] no hand-edited `.patch` files: everything came from `python tele.py export`.
- [ ] `UPSTREAM` unchanged, `.github/` untouched.
- [ ] builds, and the feature was tested by hand with the setting on and off. say what you tested and on which system.
- [ ] off by default, and off behaves exactly like upstream.
- [ ] the setting is in `tele_options`, on a settings page in a fitting group, and in the `Specs()` export table.
- [ ] new network use has a setting and respects the launch flags.
- [ ] ui strings go through `Tele::Lower` as whole english literals (`.arg` after it, `Plural` for counts), and there are no code comments.
- [ ] commit subjects are one line, `feat:` or `fix:`, no body or trailers.
- [ ] a row in `docs/releases.md` in the right format, and the patch count updated.
- [ ] `python tele.py --tdesktop <clean clone> apply` succeeds.
