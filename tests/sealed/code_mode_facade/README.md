# Code display facade scope

This target includes the actual `tele_sealed_codes.cpp` and compiles the actual `tele_sealed_codes_filter.cpp` from `TDESKTOP_SOURCE_DIR`. It uses synthetic messages and temporary directories. No implementation is copied into this folder, and no real login codes, Telegram sessions or account data are accessed.

The facade replaces Telegram application, session/history objects, reactive signals, timer scheduling, options and UI widgets. The fake history setter calls the real product filter; its preview/dependency and notification methods record calls. Tests cover bounded policy parsing and persistence, context and field validation, initial off behavior, loaded/new-message masking, incoming777000 scope, stable mappings across translations begun before activation and translations of already displayed decoys, code-bearing URL removal, re-filtering, duplicate activation, option/duration preservation, expiry restoration, cache cleanup and an injected policy-write failure.

These checks do not compile the real HistoryItem integration, render a view, exercise native notification or clipboard APIs, verify accessibility/search/export coverage, validate true session lifetimes, or test the passcode-protected repair dialog. Native build and manual checks remain required. Real API/export/log data and server-side forwarding remain outside this display filter.

Run only this target after configuring the parent test project:

```
cmake --build <build> --target sealed_code_mode_facade_test
ctest --test-dir <build> -R "^sealed_code_mode_facade$" --output-on-failure
```
