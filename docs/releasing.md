# releasing

for maintainers. releases are built and published by github actions and need the repository's secrets, so contributors never release anything themselves. this page explains how it works, so everyone knows what a release is made of and how to check one.

## versions

- a release is tagged `<UPSTREAM>-tele.<N>`, for example `v7.2.9-tele.10`, and titled `tele N · <upstream tag>`.
- `N` is one above the highest existing release or tag, whatever upstream it was built on: numbers go on across upstream bumps (`v7.2.9-tele.18` is followed by `v7.3.0-tele.19`), because the app compares build numbers. build.yml picks it, or takes the `number` input when it's above every existing one.
- `N` is compiled in as `TELE_BUILD`. the app shows it as `tele #N` in the title bar and `build #N` in the main menu.
- the self-updater orders builds by upstream `AppVersion` first and `N` second, so a release on a newer upstream tag always wins.

## branches

- work lands on `next`. pull requests go there.
- `main` is what gets released: a push to `main` that touches `UPSTREAM`, `patches/`, `tele.py`, `ci/` or `build.yml` starts a build that publishes when every platform succeeds.

## before a release

1. **fold.** follow-up fixes and improvements of a feature are folded into the feature's patch where the result is the same tree, so the queue keeps one patch per feature. anything that can't be folded cleanly stays its own patch.
2. **baseline.** after a fold, record the previous release folded the same way in `ci/baselines.json`: its tag mapped to `{subject: sha256 of the changed lines}` of each patch. the release notes then compare like with like, and folded patches don't show up as dropped. the next release doesn't need it any more.
3. **patch rows.** every new patch has its row in `docs/releases.md`, every `with N` points at the right number, and the patch count at the top is right. `docs/features.md` is regenerated from it with `python ci/features.py`.
4. **dry run.** the exported queue applies to a fresh clone of the `UPSTREAM` tag with `python tele.py --tdesktop <clean clone> apply`.
5. **test.** the new and changed features were exercised in a local build. a ci run takes hours, and a broken release reaches every user through the updater.

## building and publishing

a release is built once and published from that finished build, so what was tested in ci is exactly what ships:

1. start build.yml on the branch with `publish=false`. it builds windows, linux and macos and keeps the files as artifacts for one day.
2. once it succeeds, move `main` to the same commit without starting a second build (a push to `main` normally starts one), and publish that run:

   ```
   gh workflow run build.yml --ref main -f from_run=<run id> -f publish=true
   ```

   `publish` checks that the run succeeded and built the same `UPSTREAM`, `patches/` and `tele.py` as `main`, and uses the release number that run picked.

promote.yml can do step 2 on its own: when the repository variable `PROMOTE_RUN` holds a run id, the publish starts as soon as that run succeeds.

## what gets published

| file | what it is |
|---|---|
| `tele-<tag>-win64.zip` | `tele.exe` |
| `tele-<tag>-linux64.zip` | the `tele` binary |
| `tele-<tag>-macos.zip` | `tele.app`, signed ad hoc, used by the updater |
| `tele-<tag>-macos.dmg` | the same app for manual installs |
| `tele-update-win64.json`, `tele-update-linux64.json`, `tele-update-macos.json` | the signed update feeds |
| `tele-changelog.json` | what's new, for the in-app chat |

the release notes list the new, changed and dropped patches with their rows from `docs/releases.md`, the downloads and the full patch table. when the announcement secrets are set, the release is also announced in the project's telegram channel.

## signing and attestation

- **update feeds.** each feed names the release, its build number, the zip url and the sha256 of the zip and of the executable inside it. it's signed with an ed25519 key that only exists as a repository secret, and the signature is verified against `ci/update-public-key.pem` before the release is created. the app has the matching public key built in and refuses a feed that doesn't verify, a download that doesn't match the hashes, and urls outside this repository's releases.
- **debug symbols.** linux and macos releases also carry `tele-<tag>-<platform>-symbols.zip`, the symbol table of the unstripped binary. nobody needs them to run tele; they turn the addresses in a crash dump into function names. windows has none: github's runners run out of disk or memory building them.
- **build provenance.** every zip and the dmg are attested in the job that compiled them, with github's `actions/attest`. the attestation stays valid when the files are published later from that run. anyone can check a download:

  ```
  gh attestation verify tele-<version>-win64.zip --repo nitreojs/tele
  ```

## new upstream releases

sync.yml checks for a new stable tdesktop release every 3 hours. betas are skipped.

- when the queue applies cleanly, it moves `UPSTREAM`, commits `chore: bump upstream to <tag>`, starts a build with `publish=false` and opens a `new-upstream` issue with the command that publishes that run.
- when it doesn't apply, it opens a `patch-conflict` issue with the log. the conflict is resolved locally, as described in [development](development.md#syncing-with-a-new-upstream-release), and never in ci.

canary.yml applies the queue to upstream's `dev` branch every day and opens a `canary-conflict` issue when it fails, so most conflicts are known before the release that brings them.

a failed or cancelled build on `main` opens a `build-failure` issue. the next successful publish closes it.
