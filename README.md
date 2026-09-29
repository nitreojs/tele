# tele

a telegram desktop fork. it isn't a real fork of the code: this repo only holds a queue of patches that get applied on top of every stable [tdesktop](https://github.com/telegramdesktop/tdesktop) release and built here automatically.

## download

grab the latest build from [releases](https://github.com/nitreojs/tele/releases/latest).

- **windows x64**: `tele-<version>-win64.zip`. unpack it anywhere and run `tele.exe`. it keeps its data next to the exe.
- **linux x64**: `tele-<version>-linux64.zip`. unpack it somewhere you can write to (like `~/.local/opt/tele`) and run `./tele`. it keeps its data in `~/.local/share/tele` and adds itself to the app menu.
- **macos** (apple silicon and intel): `tele-<version>-macos.dmg`. open it, drag tele into applications, then run this once in the terminal, since the build isn't notarized by apple (the `.zip` next to it is what the self-updater downloads):

  ```
  xattr -dr com.apple.quarantine /Applications/tele.app
  ```

  it keeps its data in `~/Library/Application Support/tele`.

none of them touch an installed telegram or its data.

tele updates itself: it checks these releases every 3 hours, downloads new builds in the background and asks you to restart. the update feed is signed, so a build that isn't from here won't be installed. you can turn it off in settings → tele → updates.

every zip and dmg has a [build provenance attestation](https://github.com/nitreojs/tele/attestations): github signs that the file was built by this repo's workflow from a given commit and hasn't changed since. check a download with the [github cli](https://cli.github.com):

```
gh attestation verify tele-<version>-win64.zip --repo nitreojs/tele
```

## patches

everything tele adds lives in settings → tele, right below the language row, grouped into pages: interface, chats and messages, privacy, profiles and ids, bots and debug, with server, backup and updates on the page itself. the search at the top of the page, and the main settings search, find every tele setting. backup exports your tele settings to a file you can give to anyone, and imports one with a preview of what changes.

<details>
<summary>all 128 patches, newest release first</summary>

### [tele 12](https://github.com/nitreojs/tele/releases/tag/v7.2.9-tele.12)

| # | what it does | where to toggle |
|---|---|---|
| [128](patches/tdesktop/0128-feat-hide-phone-numbers-in-profiles.patch) | the mobile row is hidden in every profile, yours included | tele → profiles and ids, off |

### [tele 11](https://github.com/nitreojs/tele/releases/tag/v7.2.9-tele.11)

| # | what it does | where to toggle |
|---|---|---|
| [109](patches/tdesktop/0109-feat-attach-media-above-the-field-beta.patch) | attach media above the input field instead of in a full-window box: reorder, remove, group or send as files right there, edit photos, see the album as a grid, and the unsent media stays in the chat | tele → chats and messages, off |
| [110](patches/tdesktop/0110-fix-show-server-badges-on-the-personal-channel-in-pr.patch) | checkmarks and custom verification from the tele server show on a profile's personal channel too | always on |
| [111](patches/tdesktop/0111-fix-explain-a-missing-changelog-instead-of-doing-not.patch) | when a version's changelog can't be found, the tele chat says so instead of doing nothing | always on |
| [112](patches/tdesktop/0112-feat-show-new-settings-after-an-update.patch) | after an update, a new in tele page lists the settings that version added. the what's new message links to it | always on |
| [113](patches/tdesktop/0113-feat-never-block-screen-capture.patch) | tele never blocks screenshots, screen recording or screen sharing, protected chats and disappearing media included | tele → privacy, on |
| [114](patches/tdesktop/0114-feat-choose-what-shots-show.patch) | choose what shots show: names, userpics, reactions, time and checks, replies, forwarded headers, buttons and link previews | with 89 |
| [115](patches/tdesktop/0115-fix-trim-settings-descriptions.patch) | setting descriptions only say what their names can't | always on |
| [116](patches/tdesktop/0116-feat-forward-protected-messages-as-copies.patch) | forward messages from chats that forbid it: tele sends copies and uploads their media again | tele → chats and messages, off |
| [117](patches/tdesktop/0117-fix-stop-videos-at-their-end.patch) | videos stop at their end, and play starts them again | always on |
| [118](patches/tdesktop/0118-feat-caption-docked-media-and-bring-it-to-parity-wit.patch) | attached media above the field get a caption each and everything the send box has: self-destruct timer, caption above, hd, price, effects and the send menu. they can be saved as drafts, which keep video edits and price, and an edited draft video says its edits apply when sent | with 109 |
| [119](patches/tdesktop/0119-feat-hide-settings-until-their-parent-is-on.patch) | settings that only matter while another one is on stay hidden until it is | always on |
| [120](patches/tdesktop/0120-feat-hide-communities.patch) | hide communities: their chats stay in the list as regular chats | tele → interface, off |
| [121](patches/tdesktop/0121-feat-send-replies-to-older-messages-past-the-queue.patch) | replies to messages older than the uploading media don't wait in the queue | with 51 |
| [122](patches/tdesktop/0122-feat-send-right-away-on-double-enter.patch) | enter twice sends a queued message right away | with 51 |
| [123](patches/tdesktop/0123-feat-regroup-the-sending-settings.patch) | the sending settings are split into sending order, uploads, message field and scheduled messages | always on |
| [124](patches/tdesktop/0124-feat-use-aliases-for-inline-bots.patch) | username aliases work for inline bots too: `@alias query` asks the real bot | with 72 |
| [125](patches/tdesktop/0125-feat-hide-mention-and-reaction-badges-in-the-chat-li.patch) | the options that hide the mentions and reactions buttons also hide the @ and ❤️ badges in the chat list and topic tabs | with 73 |
| [126](patches/tdesktop/0126-feat-pin-chats-past-the-limit.patch) | pin more chats than telegram allows: the extra pins stay on this device and have a hollow pin icon | tele → interface, off |
| [127](patches/tdesktop/0127-feat-folders-past-the-limit.patch) | more folders and more chats per folder than telegram allows: what doesn't fit stays on this device | tele → interface, off |

### [tele 10](https://github.com/nitreojs/tele/releases/tag/v7.2.9-tele.10)

| # | what it does | where to toggle |
|---|---|---|
| [84](patches/tdesktop/0084-feat-move-chats-instantly-in-the-chat-list.patch) | chats move to their new place in the chat list right away, without the pause while the mouse moves over the list | tele → interface, off |
| [85](patches/tdesktop/0085-feat-show-collectible-prices-at-the-purchase-date.patch) | collectible usernames and numbers show their dollar price at the purchase date, not today's | tele → profiles and ids, off |
| [86](patches/tdesktop/0086-feat-set-your-own-app-and-tray-icons.patch) | your own app and tray icons from any image, and the tray icon can follow the app icon | tele → interface → app icon |
| [87](patches/tdesktop/0087-feat-choose-when-links-get-previews-and-set-your-own.patch) | choose when links get previews: for all links, not for pasted links, or only ones you add. add a preview of any link to any message, with its size and position | tele → chats and messages |
| [88](patches/tdesktop/0088-feat-turn-username-into-a-t.me-link-in-the-link-box.patch) | in the link box (ctrl+k), @username becomes a t.me link | always on |
| [89](patches/tdesktop/0089-feat-save-messages-as-a-picture.patch) | shot: save selected messages, or one message, as a picture from the message menu, with or without the chat background and dates | tele → chats and messages, off |
| [90](patches/tdesktop/0090-feat-debug-logs-and-an-mtproto-inspector.patch) | a debug page: debug logs on or off, open the logs or tele folder, clear logs, and an mtproto inspector for the logs that filters requests, expands objects, opens entries on [schema.jppgr.am](https://schema.jppgr.am) and opens things from message and chat menus | tele → debug |
| [91](patches/tdesktop/0091-feat-new-tele-app-and-tray-icons.patch) | tele's own app and tray icons | always on |
| [92](patches/tdesktop/0092-fix-name-the-item-Copy-Callback-Data-like-telegram-d.patch) | the menu item is called copy callback data, like in telegram | with 28 |
| [93](patches/tdesktop/0093-feat-usernames-from-the-tele-server.patch) | the tele server can give accounts extra @usernames, see [running your own server](#running-your-own-server). a server username wins over the real one, your own aliases win over both | tele → server, on |
| [94](patches/tdesktop/0094-fix-open-custom-emoji-in-rich-and-emoji-only-message.patch) | custom emoji in rich messages and in emoji-only messages open their pack on click, and every pack is counted | always on |
| [95](patches/tdesktop/0095-feat-send-gifs-as-videos-and-videos-as-gifs.patch) | send gifs as videos and videos as gifs: in the send box, from a message's menu, and with right-click in the gif panel | always on |
| [96](patches/tdesktop/0096-feat-keep-drafts-on-this-device.patch) | drafts stay on this device and never go to the cloud. a button clears the ones already there | tele → privacy, off |
| [97](patches/tdesktop/0097-feat-send-scheduled-messages-on-time.patch) | scheduled messages are sent by tele itself at the set time while it's online | tele → chats and messages, off |
| [98](patches/tdesktop/0098-feat-show-id-and-dc-in-one-row.patch) | when both are shown, the dc goes next to the id in one profile row | tele → profiles and ids, off |
| [99](patches/tdesktop/0099-feat-redesign-the-title-template-editor.patch) | a new title template editor: variables as chips, a palette with modifiers and presets | with 10 |
| [100](patches/tdesktop/0100-fix-ignore-reply-counters-on-scheduled-messages.patch) | edited scheduled messages don't show junk reply counters | always on |
| [101](patches/tdesktop/0101-feat-save-drafts-for-later.patch) | saved drafts: keep several drafts per chat, with media or rich messages, in a drafts section that looks like a chat. edit them like messages, send them now or to another chat, and bring back drafts you discarded | tele → chats and messages → drafts |
| [102](patches/tdesktop/0102-feat-send-photos-in-hd-by-default.patch) | photos go out in hd by default | tele → chats and messages, on |
| [103](patches/tdesktop/0103-feat-find-chats-by-their-aliases.patch) | chat search finds chats by their aliases, yours and the server's | with 72 |
| [104](patches/tdesktop/0104-fix-show-seen-and-reacted-counts-separately.patch) | the seen and reacted menu shows both counts separately | always on |
| [105](patches/tdesktop/0105-feat-send-an-inline-bot-query-as-text.patch) | an inline bot query can be sent as plain text | always on |
| [106](patches/tdesktop/0106-feat-choose-which-app-opens-each-service.patch) | choose which app opens each service's links: the official app, an alternative like spotifast, the browser, or any program | tele → chats and messages → open links in apps |
| [107](patches/tdesktop/0107-feat-scale-shots-up-to-4x-and-show-your-messages-as-.patch) | shots render at 1x to 4x with sharp text, bubbles, userpics, icons, media and animated emoji, can show your own messages as incoming, and the selection clears when a shot opens | with 89 |
| [108](patches/tdesktop/0108-feat-export-and-import-tele-settings.patch) | export tele settings to a file or as text and import them on any account, system or tele version, with a preview first. people and device settings are optional | tele → backup |

### [tele 9](https://github.com/nitreojs/tele/releases/tag/v7.2.9-tele.9)

| # | what it does | where to toggle |
|---|---|---|
| [59](patches/tdesktop/0059-feat-keep-messages-you-delete-yourself.patch) | messages you delete yourself are kept too, not only ones deleted by others | with 19 |
| [60](patches/tdesktop/0060-feat-show-server-and-updates-on-the-tele-settings-pa.patch) | the server and updates rows sit right on the tele settings page | always on |
| [61](patches/tdesktop/0061-feat-pick-brush-colors-from-the-image.patch) | an eyedropper in the photo editor picks the brush color from the image | always on |
| [62](patches/tdesktop/0062-feat-show-edit-history-as-a-chat.patch) | edit history opens as its own chat: every version as a full message with its media and time | with 18 |
| [63](patches/tdesktop/0063-feat-hide-or-confirm-call-and-voice-chat-buttons.patch) | call and voice chat buttons can each be shown, hidden, or ask before calling | tele → interface |
| [64](patches/tdesktop/0064-feat-group-the-tele-interface-and-profile-settings.patch) | the interface and the profiles and ids pages are split into groups | always on |
| [65](patches/tdesktop/0065-feat-more-ghost-mode-options.patch) | more ghost mode: its main menu toggle is optional, sending a message while the online status is hidden sets you offline again right away, and reaction animations play once instead of over and over | tele → privacy |
| [66](patches/tdesktop/0066-feat-show-ids-on-regular-gifts.patch) | a copyable id on regular gifts too, not only on collectible ones | tele → profiles and ids, off |
| [67](patches/tdesktop/0067-feat-send-inline-results-without-via.patch) | inline bot results go out as your own messages, without via @bot | tele → bots, off |
| [68](patches/tdesktop/0068-feat-skip-the-rich-message-prompt-on-paste.patch) | no rich message prompt when you paste formatted text | tele → chats and messages, off |
| [69](patches/tdesktop/0069-feat-set-or-remove-chat-backgrounds-only-for-you.patch) | set your own background for any chat, or remove its background, only for you, from the chat menu | tele → chats and messages → chat backgrounds |
| [70](patches/tdesktop/0070-feat-skip-or-quote-deleted-messages-when-replying.patch) | replying to a message that got deleted meanwhile either drops the reply or quotes the deleted text | tele → chats and messages |
| [71](patches/tdesktop/0071-feat-add-test-server-accounts-with-a-plain-right-cli.patch) | a plain right-click on add account offers the test server | always on |
| [72](patches/tdesktop/0072-feat-link-usernames-to-profiles-locally.patch) | username aliases: your own @alias for any profile, in any letters, clickable and in autocomplete, only for you. aliases in sent messages become real mentions, a toast says what changed | tele → chats and messages → username aliases |
| [73](patches/tdesktop/0073-feat-hide-the-mentions-and-reactions-buttons.patch) | the jump to mentions and jump to reactions buttons can be hidden | tele → interface, off |
| [74](patches/tdesktop/0074-feat-pick-ignored-users-and-ghost-chats-like-privacy.patch) | ignored users and ghost chats are picked from a searchable list, like privacy exceptions | tele → privacy |
| [75](patches/tdesktop/0075-feat-show-what-s-new-in-a-local-tele-chat.patch) | what's new comes from a local tele chat with its own profile, grouped by settings page, once per account the first time you open it after an update. a button opens the full changelog | with 39 |
| [76](patches/tdesktop/0076-feat-show-names-instead-of-phone-numbers-in-chat-hea.patch) | people who shared their number with you but aren't in your contacts keep their name in the chat header instead of the number | tele → profiles and ids, off |
| [77](patches/tdesktop/0077-feat-open-the-emoji-panel-and-attach-menu-by-click-o.patch) | the emoji, sticker and gif panel and the attach menu open on click only, not on hover | tele → interface, off |
| [78](patches/tdesktop/0078-feat-rewrite-pasted-links-with-your-own-rules.patch) | links you paste are rewritten by your own rules, like x.com to fixupx.com. ctrl+z brings the original back | tele → chats and messages → link rewrites |
| [79](patches/tdesktop/0079-feat-edit-link-rewrite-rules-and-presets.patch) | an editor for link rewrite rules: domains or regex patterns, presets for x, instagram, tiktok, reddit, bluesky and pixiv, and a field to test a link | with 78 |
| [80](patches/tdesktop/0080-feat-name-people-only-for-you.patch) | give people a name only you see, from their chat menu. their profile keeps the real one | tele → profiles and ids → custom names |
| [81](patches/tdesktop/0081-feat-add-launch-flags-to-turn-off-tele-s-own-network.patch) | launch flags that keep tele off the network for one launch, see [below](#launch-flags) | always on |
| [82](patches/tdesktop/0082-feat-preview-calcmula-results-while-typing.patch) | the calcmula result shows above the input field while you type, like an inline bot. a failed calcmula message goes back into the field once instead of doubling | tele → chats and messages, off |
| [83](patches/tdesktop/0083-feat-clear-server-data-and-explain-custom-verificati.patch) | clear the tele server's saved data, badges included. click a custom verification icon to see what it means: its description, with the icon next to it | tele → server → clear cached data |

### [tele 8](https://github.com/nitreojs/tele/releases/tag/v7.2.9-tele.8)

| # | what it does | where to toggle |
|---|---|---|
| [38](patches/tdesktop/0038-feat-turn-off-animated-userpics.patch) | animated userpics can stay still: separately in the chat list and in chats and profiles | tele → interface, off |
| [39](patches/tdesktop/0039-feat-show-what-s-new-in-tele-after-an-update.patch) | after an update, you get a message with what's new in that tele version, visible only to you. fresh installs skip it | tele → updates, on |
| [40](patches/tdesktop/0040-feat-call-the-app-tele-everywhere.patch) | the app calls itself tele everywhere: tray menu, crash window, system menus and more | always on |
| [41](patches/tdesktop/0041-feat-customize-the-window-title-and-format-template-.patch) | the window title (taskbar, alt+tab, system window frame) can be a template too, or follow the title bar label. every [template](#title-bar-template) variable takes modifiers like `{weekday|short|lower}` or `{chat|max:20}` | tele → interface |
| [42](patches/tdesktop/0042-feat-group-and-search-tele-settings.patch) | tele settings are grouped into pages like the main settings, with short descriptions and their own search field, and the main settings search finds them too. old links to tele settings open the right page | always on |
| [43](patches/tdesktop/0043-feat-show-times-on-service-messages.patch) | service messages like joins and pins show their time too, with seconds | with 17 |
| [44](patches/tdesktop/0044-feat-upload-media-in-several-chats-at-once.patch) | media uploads in several chats at once instead of waiting for each other. files in one chat still go one after another | tele → chats and messages, off |
| [45](patches/tdesktop/0045-feat-reply-timestamps-for-media-in-rich-messages.patch) | time codes like 1:23 in a reply to a rich message link to its video or audio, the first one if there are several | always on |
| [46](patches/tdesktop/0046-feat-compute-messages-starting-with-via-calcmula.patch) | messages and captions starting with `= ` are computed with [calcmula](https://calcmula.app) and sent as the quoted query with `= result` below it. if it fails, nothing is sent and the text goes back into the field | tele → chats and messages, off |
| [47](patches/tdesktop/0047-feat-move-show-peer-ids-into-tele-settings.patch) | show peer ids moved from experimental settings into tele | tele → profiles and ids |
| [48](patches/tdesktop/0048-feat-show-the-tele-build-in-the-main-menu.patch) | the main menu shows the tele build next to the version | always on |
| [49](patches/tdesktop/0049-feat-open-collectible-gifts-in-see.tg.patch) | an open in see.tg link on collectible gift cards | tele → profiles and ids, off |
| [50](patches/tdesktop/0050-feat-move-late-sent-messages-to-the-bottom.patch) | a message that took long to send moves to the bottom of the chat once it's sent, so it's clear when it went out | tele → chats and messages, off |
| [51](patches/tdesktop/0051-feat-queue-messages-behind-an-uploading-media.patch) | messages sent while a media is uploading wait for it and go out after it, in order | tele → chats and messages, off |
| [52](patches/tdesktop/0052-feat-open-links-in-their-desktop-apps.patch) | spotify, steam, discord, zoom, teams, notion, slack and epic links open in their desktop apps when they're installed | tele → chats and messages, off |
| [53](patches/tdesktop/0053-feat-clean-tracking-parameters-from-links.patch) | the SUPER MAGA PALANTIR ICE PETER THIEL AI DATA HARVESTER 9000 remover: opened links lose their tracking parameters (utm, fbclid, si, gclid and [more](#link-cleaner)). when it would change a link, its right-click menu offers open without cleaning. the same remover works on links in sent messages and captions, the rest of the text stays as it is | tele → chats and messages, off |
| [54](patches/tdesktop/0054-feat-reveal-spoilers-automatically.patch) | text and media spoilers are revealed right away, in chats and the chat list | tele → chats and messages, off |
| [55](patches/tdesktop/0055-feat-move-tele-tools-to-the-bottom-of-the-message-me.patch) | tele's items sit at the bottom of the message menu: view as tl, then the message id | always on |
| [56](patches/tdesktop/0056-feat-copy-custom-emoji-ids-from-the-message-menu.patch) | right-click a custom emoji in a message to copy its id | tele → chats and messages, off |
| [57](patches/tdesktop/0057-fix-change-the-speed-instead-of-moving-the-media-vie.patch) | dragging while holding a video to speed it up changes the speed instead of moving the media viewer window, and the speedup no longer stops by itself | always on |
| [58](patches/tdesktop/0058-feat-reorder-and-hide-message-menu-items.patch) | reorder and hide items of the message menu, items it doesn't know keep their place | tele → chats and messages → message menu |

### [tele 7](https://github.com/nitreojs/tele/releases/tag/v7.2.9-tele.7)

| # | what it does | where to toggle |
|---|---|---|
| [17](patches/tdesktop/0017-feat-show-seconds-in-message-times.patch) | message times show seconds, like 14:03:21 | tele → chats and messages, off |
| [18](patches/tdesktop/0018-feat-keep-the-edit-history-of-messages.patch) | remembers what edited messages looked like while tele runs: right-click an edited message → edit history | tele → chats and messages, off |
| [19](patches/tdesktop/0019-feat-keep-deleted-messages.patch) | messages deleted by others or from your other devices stay in the chat until tele restarts, service messages too, faded as a whole or with a trash icon by the time. each chat gets a page with its kept deleted messages and a clear all button: chat menu → deleted messages | tele → chats and messages, off |
| [20](patches/tdesktop/0020-fix-fade-every-unsupported-experimental-option.patch) | experimental options your system doesn't support are faded completely, title included | always on |
| [21](patches/tdesktop/0021-feat-hide-call-buttons.patch) | no call button in private chats and profiles | tele → interface, off |
| [22](patches/tdesktop/0022-feat-lowercase-every-interface-text.patch) | lowercases the whole interface. messages and names stay as they are | tele → interface, off, needs a restart |
| [23](patches/tdesktop/0023-feat-show-the-data-center-in-profiles.patch) | a dc row in profiles: the data center the account or chat lives in | tele → profiles and ids, off |
| [24](patches/tdesktop/0024-feat-hide-sponsored-messages.patch) | no ads: no sponsored messages in channels and bots, no video ads, no sponsored search results | tele → chats and messages, off |
| [25](patches/tdesktop/0025-feat-open-links-without-confirmation.patch) | links with custom text open right away, without the confirmation | tele → chats and messages, off |
| [26](patches/tdesktop/0026-feat-open-disappearing-media-without-burning-it.patch) | view-once and timed media open without burning, and the sender still sees them unopened. message menu → mark as viewed burns them | tele → chats and messages, off |
| [27](patches/tdesktop/0027-feat-allow-screenshots-of-disappearing-media.patch) | view-once and timed media can be screenshotted and recorded | tele → chats and messages, off |
| [28](patches/tdesktop/0028-feat-copy-the-callback-data-of-bot-buttons.patch) | right-click over a bot button to copy its callback data, inline query, web app url and so on | always on |
| [29](patches/tdesktop/0029-feat-show-the-message-id-in-the-message-menu.patch) | the message menu ends with the message id, click it to copy | tele → chats and messages, off |
| [33](patches/tdesktop/0033-feat-view-messages-and-telegram-objects-as-tl.patch) | view as tl in the message menu, and for chats, profiles, members, topics, stickers and sets, custom emoji, gifts, stories and folders: fetches the object from the server and opens it on [schema.jppgr.am](https://schema.jppgr.am) | tele → chats and messages, off |
| [30](patches/tdesktop/0030-feat-hide-the-all-chats-folder.patch) | hides the all chats folder when you have other folders, the list opens on your first one | tele → interface, off |
| [31](patches/tdesktop/0031-feat-jump-to-the-first-message-of-a-chat.patch) | jump to the first message, in the chat menu | tele → chats and messages, off |
| [32](patches/tdesktop/0032-feat-add-ghost-mode.patch) | ghost mode: no read receipts, typing, online status or story views, each switchable, and per chat always or never from the chat menu. optionally reads a chat when you reply, and sends messages as scheduled a few seconds ahead so sending doesn't put you online. the message menu has mark as read up to here, and the side menu has a quick toggle | tele → privacy, off |
| [34](patches/tdesktop/0034-feat-hide-the-mtproxy-sponsor-channel.patch) | no sponsor channel pinned to the chat list when you connect through an mtproxy | tele → chats and messages, off |
| [35](patches/tdesktop/0035-feat-lowercase-tele-s-own-texts-too.patch) | lowercase covers tele's own texts and the crash window too | with 22 |
| [36](patches/tdesktop/0036-fix-stop-maximized-windows-jittering-on-monitors-wit.patch) | a maximized window no longer jitters on a monitor without a taskbar (windows) | always on |
| [37](patches/tdesktop/0037-feat-send-crash-reports-to-the-tele-server.patch) | when tele crashed, the next start offers to send the crash report to the tele server instead of telegram. nothing leaves without your click | tele → server, on with the server |

### [tele 5](https://github.com/nitreojs/tele/releases/tag/v7.2.9-tele.5)

| # | what it does | where to toggle |
|---|---|---|
| [13](patches/tdesktop/0013-feat-show-gift-ids.patch) | a copyable gift id at the top of the collectible gift card, which only says a gift is on the ton blockchain while it really is there | tele → profiles and ids, off |
| [14](patches/tdesktop/0014-feat-show-the-peer-id-as-its-own-profile-row.patch) | the peer id gets its own profile row instead of trailing the bio, optionally without spaces or in bot api style (-100… for channels) | tele → profiles and ids, off (see 47) |
| [15](patches/tdesktop/0015-feat-copy-links-to-tele-settings.patch) | right-click any tele setting to copy a link that opens it | always on |
| [16](patches/tdesktop/0016-feat-ignore-users-by-hiding-or-fading-their-messages.patch) | ignore users from their userpic menu: their messages fade or disappear, the list lives in settings | tele → privacy |

### [tele 4](https://github.com/nitreojs/tele/releases/tag/v7.2.9-tele.4)

| # | what it does | where to toggle |
|---|---|---|
| [8](patches/tdesktop/0008-feat-send-quick-replies-in-groups-and-channels.patch) | business quick replies in groups and channels too. telegram only sends them in private chats, so tele posts the messages as ordinary ones (no bot keyboards, via-bot labels or effects) | tele → chats and messages, off |
| [9](patches/tdesktop/0009-feat-apply-verification-and-marks-from-the-tele-serv.patch) | checkmarks, custom verification, scam, fake and support marks from the [tele server](#tele-server), shown exactly like telegram's own. custom verification icons animate, member lists update as soon as the data arrives, and refresh now fetches it right away | tele → server, on |
| [10](patches/tdesktop/0010-feat-make-the-title-bar-label-a-live-template.patch) | the title bar label is a template with live variables, see below | tele → interface |
| [11](patches/tdesktop/0011-feat-remove-the-account-limit.patch) | no account limit (well, 1536) | always on |
| [12](patches/tdesktop/0012-feat-show-checkmarks-and-custom-verification-everywh.patch) | checkmarks and custom verification in the account list, the main menu, the settings header and, optionally, next to sender names in messages. badges go in telegram's order everywhere: custom verification, name, emoji status, checkmark, and the premium star stays visible next to checkmarks | always on, in messages: tele → interface, off |

### [tele 3](https://github.com/nitreojs/tele/releases/tag/v7.2.9-tele.3)

| # | what it does | where to toggle |
|---|---|---|
| [1](patches/tdesktop/0001-feat-show-the-tele-build-number-in-the-title-bar.patch) | the title bar shows which tele build you're on | always on, see 10 |
| [2](patches/tdesktop/0002-feat-add-a-tele-section-to-the-settings.patch) | the tele section in settings | always on |
| [3](patches/tdesktop/0003-feat-add-an-option-to-hide-stories-everywhere.patch) | hides stories everywhere: no stories bar, no rings around userpics, no stories in profiles | tele → privacy, off, needs a restart |
| [4](patches/tdesktop/0004-feat-add-an-option-to-watch-stories-invisibly.patch) | watch stories without showing up among the viewers. reactions and replies still reveal you | tele → privacy, off |
| [5](patches/tdesktop/0005-feat-show-bot-button-payloads-in-tooltips.patch) | hovering a bot button shows what it carries: callback data, links, inline queries, web apps and more | tele → bots, off |
| [6](patches/tdesktop/0006-feat-update-from-the-tele-github-releases.patch) | self-updates from these releases on windows, linux and macos, signed with ed25519 | tele → updates, on |
| [7](patches/tdesktop/0007-feat-rename-the-app-to-tele.patch) | the app is called tele: `tele.exe`, its own taskbar entry, links point here. linux and macos builds are their own app, so they don't clash with an installed telegram | always on |

</details>

### title bar template

the default is `TELE {build}`. empty hides the label.

- `{build}`, `{version}`
- `{time}`, `{date}`, `{weekday}`, `{day}`, `{month}`, or any qt format like `{time:HH:mm:ss}` and `{date:dd MMM}`
- `{name}`, `{username}`, `{id}`, `{accounts}`
- `{unread}`, `{chat}`, `{chat_unread}`, `{status}`

any variable takes modifiers after `|`, applied left to right: `{weekday|short|lower}` is `sun`, `{date:dddd|upper}` is `SUNDAY`.

- `lower`, `upper`: `{weekday|lower}` is `sunday`
- `title` capitalizes every word, `cap` only the first letter
- `short` is the compact form, wherever it sits in the chain: `Sun` and `Sep` for weekday and month, the first name for `{name}` and for people in `{chat}`, `42` for `{build}`, `7.2` for `{version}`, `1.2k` for counters, `…` for `{status}`. other variables stay as they are
- `first`, `last`: the first or last word, `{name|first}`
- `max:N` cuts to N characters with `…`, `{chat|max:20}`
- `pad:N` pads to N characters, with zeros for numbers: `{day|pad:2}` is `07`
- `k` shortens numbers: `1234` is `1.2k`, `15000` is `15k`, `2500000` is `2.5m`
- `default:text` shows text when the value is empty or 0, `{status|default:online}`

`[ … ]` hides its part when a variable inside is empty or 0, so `TELE {build}[ · {unread} unread]` doesn't show "· 0 unread". a `default` counts as filled. doubled brackets are literal. a `|` that isn't followed by modifiers stays part of a qt format, and `'|'` in quotes always does. a variable or modifier with a typo is shown as typed. account and activity variables stay empty while the app is locked with a passcode.

the editor highlights variables in the field and points out mistakes under the preview. below are all the variables as chips: click one to insert it, right-click for its other forms with their current values, hover to see the value now. presets has a few ready templates.

the window title (what the taskbar, alt+tab and the system window frame show) takes the same template, set next to the label. `{label}` puts the rendered label there, so `{label}` alone keeps both in sync. empty keeps telegram's own title, and a template that renders to nothing leaves the title blank.

### link cleaner

on every site: `utm*`, `mtm_*`, `pk_*`, `ga_*`, `_ga`, `_gl`, `gclid`, `gclsrc`, `gbraid`, `wbraid`, `dclid`, `gad_source`, `gad_campaignid`, `fbclid`, `fb_action_*`, `fb_source`, `fb_ref`, `action_*_map`, `msclkid`, `twclid`, `ttclid`, `li_fat_id`, `epik`, `yclid`, `ysclid`, `_openstat`, `mc_cid`, `mc_eid`, `mc_tc`, `ml_subscriber*`, `mkt_tok`, `igshid`, `igsh`, `_hsenc`, `_hsmi`, `__hsfp`, `__hssc`, `__hstc`, `hsctatracking`, `srsltid`, `s_kwcid`, `s_cid`, `oly_*_id`, `rb_clickid`, `vero_*`, `wickedid`, `_kx`, `wt_mc`, `wtrid`, `hmb_*`, `itm_*`, `otm_*`, `cmpid`, `os_ehash`, `__twitter_impression`, `tracking_source`, `echobox`, `spm`, `_branch_match_id`, `_branch_referrer`, `si`. fragments like `#utm_source=…` go too.

per site, on top of that: youtube (`feature`, `pp`, `kw`), spotify (`context`, `nd`, `dl_branch`), twitter / x and fx/vx mirrors (`s`, `t`, `src`, `ref_src`, `ref_url`, `cn`), threads (`xmt`, `slof`), tiktok (`_r`, `_t`, `is_from_webapp`, `sender_device`, `share_*` and more), facebook (`mibextid`, `__tn__`, `__cft__`, `ref*`, `notif_*` and more), reddit (`share_id`, `ref*`, `correlation_id`, `rdt`), amazon (`pd_rd_*`, `qid`, `ref_`, `tag`, `linkCode`, `/ref=…` in the path and more), aliexpress (`aff_*`, `algo_*`, `pvid`, `scm*` and more), vk (`from`, `ref`, `ref_domain`), yandex (`from`, `clid`, `redircnt`), google search (`ved`, `ei`, `sa`, `usg`, `oq`, `aqs`, `gs_*` and more), google docs / drive (`usp`), linkedin (`trk*`, `refId`, `lipi` and more), ebay (`_trk*`, `mk*`, `campid` and more), medium (`source`), github (`email_token`, `email_source`), steam (`snr`), netflix (`trackId`, `tctx`), twitch (`tt_medium`, `tt_content`), imdb (`ref_`, `pf_rd_*`), bing, msn, apple (`itsct`, `itscg`), pinterest, hh.ru, ozon. t.me links are never touched.
### launch flags

start tele with any of these to keep it off the network on its own for that launch. telegram's own connection works as usual, and your settings stay as they are.

- `-noteleserver`: nothing goes to the tele server: no checkmark or other mark updates (the saved ones still show) and no crash reports.
- `-noteleupdate`: no update checks, no downloads, no what's new.
- `-teleoffline`: both of the above, and calcmula is off too.

the flags stay on when tele restarts itself, like after an update. on windows, add them to a shortcut after `tele.exe`; on linux, `./tele -teleoffline`; on macos, `open -a tele --args -teleoffline`. a shortcut with `-noteleserver` keeps even the very first launch away from the tele server. to turn the server off for good, empty its field in settings.

### tele server

tele can pull extra account data, like checkmarks, custom verification, scam / fake marks and support marks, from an optional server (settings → tele → server). it only ever downloads one public list and never tells the server which accounts you look at. the list is hashed, so it can't just be read off as a list of accounts. tele checks it every 10 minutes, and "refresh now" fetches it right away. leave the field empty to turn it off. "clear cached data" drops the list tele saved, so the marks disappear until the next fetch.

the same server takes crash reports. after a crash, tele offers to send the report: a short text with the version, platform and the crash reason, plus a minidump of the crashed process. you can look at it first and untick your username. with the server field empty, nothing is offered.

#### running your own server

any server that serves the same format works: point settings → tele → server at it. the list never contains account ids, only keys that can't be turned back into ids without trying them one by one.

tele asks `GET <server>/v1/badges` and expects json like this:

```json
{
  "salt": "AAECAwQFBgcICQoLDA0ODw==",
  "argon2": { "memory": 8192, "passes": 1, "lanes": 1 },
  "peers": {
    "iguxbtP_SnLvK69aBff2OQ": { "checkmark": true, "icon": "5368324170671202286", "description": "official" },
    "w5Zbi-HfwlqkJ6S4wjwqcw": { "scam": true }
  }
}
```

- `salt` is 8 to 64 random bytes in standard base64 with padding.
- `argon2` are the argon2id costs: `memory` in KiB (1024 to 65536), `passes` (1 to 8), `lanes` (1 to 8), and `memory × passes` at most 65536. the official server uses 8192 / 1 / 1.
- each key in `peers` is `base64url` without padding of `argon2id(password, salt)` with those costs, a 16-byte output, version `0x13`, no secret and no associated data.
- the password is the utf-8 string `<scope>:<id>`:
  - `scope` is `prod`, or `test` for accounts on telegram's test servers;
  - `id` is the bot api style id: users as is (`777000`), channels and supergroups as `-100` followed by the channel id (`-1001234567890`). basic groups can't have entries.
- every field of an entry is optional: `checkmark`, `scam`, `fake` and `support` are booleans (false by default), `icon` is a custom emoji document id as a decimal string, `description` is plain text shown under the verification in the profile, and `supportText` (1 to 64 characters, users only) replaces the word "support" under the name of a support account. tele ignores fields it doesn't know, but rejects the whole list when a known field has the wrong type or length.

with the salt above, `prod:777000` gives `iguxbtP_SnLvK69aBff2OQ` and `prod:-1001234567890` gives `w5Zbi-HfwlqkJ6S4wjwqcw`. check your implementation against these two before anything else. in python:

```python
from argon2.low_level import hash_secret_raw, Type
import base64

raw = hash_secret_raw(b"prod:777000", bytes(range(16)), time_cost=1, memory_cost=8192,
                      parallelism=1, hash_len=16, type=Type.ID, version=0x13)
print(base64.urlsafe_b64encode(raw).rstrip(b"=").decode())  # iguxbtP_SnLvK69aBff2OQ
```

the list can also give accounts extra usernames. they're public anyway, so they come as plain text, in an optional `usernames` object next to `peers`:

```json
"usernames": {
  "tele": { "username": "teleAppUpdates", "position": 0 },
  "teleNews": { "username": "teleAppUpdates" }
}
```

- each key is the extra username, 2 to 32 latin letters, digits and underscores. `username` is the real public username of the account that gets it, 4 to 32 of them. both start with a letter and are compared ignoring case.
- `position` is optional: the place of the extra username among the account's usernames, counting from 0, so 0 makes it the main one in the profile. without it, the username goes after the real ones. a position past the end puts it last.
- tele skips a bad entry on its own and keeps the rest: a wrong name, an extra username equal to its account's username, the same key twice in different case, or a `position` that isn't a whole number from 0 to 1000. only the first 1000 good entries count. `usernames` never makes tele reject the list, and `salt`, `argon2` and `peers` are still required (`peers` can be empty).
- they only apply to accounts on telegram's main servers, not the test ones.

with settings → tele → usernames from the server on (the default), the profile of @teleAppUpdates shows @tele, and `@tele` in messages, `t.me/tele` and `tg://resolve?domain=tele` open @teleAppUpdates, even when someone really owns @tele. the menu on such a link can still open the real one. your own aliases come first. `@tele` in a message you send becomes `@teleAppUpdates`, so other apps see the real username.

to keep it that way:

- serve the keys sorted, so their order says nothing about the accounts behind them.
- change the salt from time to time (the official server derives a new one every month), so lists from different times can't be compared key by key. every key has to be recomputed with the new salt.
- keep the costs within the limits above: tele computes one key for every account it shows, and rejects a list that asks for more.

tele also rejects a list over 4 MiB or with more than 100000 entries, and then keeps using the last good one. it sends `If-None-Match` with `"<hex sha256 of the body it has>"`, so answering `304` when that matches the current body saves traffic, and a server can't tag clients with its own etags.

crash reports are optional. tele speaks the same protocol as telegram's own crash server, at `<server>/v1/crash.php`:

- `GET ?act=query_report&apiid=…&version=…&dmp=0|1&platform=…` answers `Report` as plain text when the server wants the report. anything else makes tele say thanks and send nothing.
- `POST ?act=report` is `multipart/form-data` with `platform` (like `Windows64Bit`, `Linux`, `MacOS`), `version` (like `7002009`), `report` (the report text) and, when there is one, `dump` (a zip with one `.dmp` minidump, under 20 MiB). answer `Done`.
- answer `404` to both if you don't collect crashes.

## contributing

bug fixes, conflict fixes and new features are welcome. for a feature, open an issue first. [CONTRIBUTING.md](CONTRIBUTING.md) walks through the whole cycle: getting the patched sources with `tele.py`, building, testing, exporting patches and opening a pull request.

- [docs/architecture.md](docs/architecture.md): the patch queue, `tele.py`, where the fork code lives and what ci does.
- [docs/development.md](docs/development.md): building, running a dev build safely and making a change.
- [docs/conventions.md](docs/conventions.md): code style, commits, patch hygiene and privacy rules.
- [docs/releasing.md](docs/releasing.md): how releases are built, signed and published.
- [AGENTS.md](AGENTS.md): rules for ai coding agents.
