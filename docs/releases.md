# releases

every tele patch, grouped by the release that brought it, newest first. [features](features.md) has the same rows grouped by settings page. release notes and the in-app what's new are made from these rows, so their format is strict: see [the patch row](development.md#the-patch-row).

all 282 patches, newest release first.

### [tele 18](https://github.com/nitreojs/tele/releases/tag/v7.2.9-tele.18)

| # | what it does | where to toggle |
|---|---|---|
| [270](../patches/tdesktop/0270-fix-unbreakable-characters-no-longer-push-message-te.patch) | a long run of characters that can't wrap, like hangul fillers, no longer pushes the message text or its selection past the bubble | always on |
| [271](../patches/tdesktop/0271-fix-the-failed-send-badge-is-clock-sized-and-no-long.patch) | the red mark on a message that failed to send is the size of the sending clock and no longer covers the time, in the chat list and in the bubble | always on |
| [272](../patches/tdesktop/0272-fix-messages-with-relative-dates-no-longer-freeze-te.patch) | a message with a countdown or relative date, like "resets in 5 minutes", no longer makes tele freeze after a while | always on |
| [273](../patches/tdesktop/0273-fix-copy-peer-ids-as-plain-digits.patch) | copying a peer id from its profile row gives plain digits, even when the row shows them with spaces | with 14 |
| [274](../patches/tdesktop/0274-feat-send-quick-replies-by-their-exact-name.patch) | a message that is exactly /name sends your quick reply with that name instead of the text, and that quick reply is the first suggestion | tele → chats, off |
| [275](../patches/tdesktop/0275-feat-copy-file-paths-from-the-message-menu.patch) | copy the full path of a downloaded file from its message menu | tele → menus, off |
| [276](../patches/tdesktop/0276-feat-turn-on-a-menu-item-from-the-menu-editor.patch) | in the menu editor, clicking the eye on a greyed item turns on the setting it needs, and the hint saying which one is readable | with 174 |
| [277](../patches/tdesktop/0277-feat-leave-kept-deleted-messages-out-of-the-chat-lis.patch) | kept deleted messages can stay out of the chat list and unread counts: the chat shows its last message that wasn't deleted, and their unread marks, mentions and notifications go away | with 19 |
| [278](../patches/tdesktop/0278-fix-a-line-under-every-open-group-of-tele-settings.patch) | an open group of tele settings ends with a thin line, so you can see where its settings end | with 42 |
| [279](../patches/tdesktop/0279-feat-lowercase-the-default-title-bar-label.patch) | the title bar label is `tele #N` by default instead of `TELE #N` | with 10 |
| [280](../patches/tdesktop/0280-feat-forward-messages-one-by-one.patch) | forwarded messages go out one at a time instead of as one batch. albums stay together, and forwarding many at once can hit rate limits | tele → sending, off |
| [1002](../patches/Telegram/lib_ui/0002-fix-unbreakable-characters-no-longer-scroll-text-fie.patch) | a text field with a long run of characters that can't wrap no longer scrolls sideways and hides everything else | always on |

### [tele 17](https://github.com/nitreojs/tele/releases/tag/v7.2.9-tele.17)

| # | what it does | where to toggle |
|---|---|---|
| [265](../patches/tdesktop/0265-fix-center-the-new-pills-in-what-s-new.patch) | the new marks in what's new sit in the middle of their line, in line with the toggles | with 254 |
| [266](../patches/tdesktop/0266-fix-ghost-mode-preview-no-longer-crashes-and-fits-it.patch) | turning on ghost mode, or opening privacy while it's on, no longer crashes tele. the ghost mode preview fits its texts, marks hidden things on the userpic and ignores clicks while ghost mode is off | with 258 |
| [267](../patches/tdesktop/0267-fix-the-menu-editor-hides-support-only-items.patch) | the menu editor no longer lists items only telegram's support accounts get, like edit support info | with 174 |
| [268](../patches/tdesktop/0268-fix-rate-limit-toasts-only-for-your-own-actions-off-.patch) | rate limit toasts and the bar are off by default and only count waits on things you do, not on requests tele makes in the background | with 211 |
| [269](../patches/tdesktop/0269-fix-a-video-ending-in-the-media-viewer-no-longer-cra.patch) | a video ending in the media viewer no longer crashes tele | with 143 |

### [tele 16](https://github.com/nitreojs/tele/releases/tag/v7.2.9-tele.16)

| # | what it does | where to toggle |
|---|---|---|
| [214](../patches/tdesktop/0214-feat-show-usernames-for-invisible-names.patch) | people whose name is empty or invisible show their @username instead, or a phrase you choose when they have none | tele → profiles and ids, off |
| [215](../patches/tdesktop/0215-feat-show-hidden-characters.patch) | names, bios and other profile texts made only of invisible characters show those characters, so you can see something is there | tele → profiles and ids, off |
| [216](../patches/tdesktop/0216-feat-multiple-quotes-in-one-reply.patch) | quote several parts of a message in one reply: quoting again while replying with a quote adds the new part | tele → messages, on |
| [217](../patches/tdesktop/0217-feat-quote-voice-message-transcriptions.patch) | select part of a voice message's transcription and quote it in a reply | always on |
| [218](../patches/tdesktop/0218-feat-quote-sticker-captions.patch) | select part of a sticker's caption and quote it in a reply | with 179 |
| [219](../patches/tdesktop/0219-fix-rate-limit-notices-for-slowmode-and-download-thr.patch) | slowmode and download throttling count as rate limits too, and rate limits can be logged in the notification centre | tele → notifications, on |
| [220](../patches/tdesktop/0220-fix-gift-batches-survive-flood-waits-and-bad-items.patch) | bulk gift actions wait out telegram's limits and skip gifts that can't be done yet instead of stopping, then say what was skipped and why | with 194 |
| [221](../patches/tdesktop/0221-fix-say-why-a-download-couldn-t-be-saved.patch) | when a download can't be saved, tele says why, and a chat's download folder can be reset when it's gone | always on |
| [222](../patches/tdesktop/0222-fix-retry-failed-upload-parts.patch) | a failed piece of an upload is retried instead of failing the whole file | always on |
| [223](../patches/tdesktop/0223-fix-teleoffline-keeps-features-that-only-use-telegra.patch) | with -teleoffline, features that only talk to telegram (gif and video conversion, link previews, shots) keep working | always on |
| [224](../patches/tdesktop/0224-feat-local-password-for-bot-payments-and-paid-messag.patch) | the local password can also be asked before bot payments, paid messages and suggested posts | with 191 |
| [225](../patches/tdesktop/0225-fix-confirm-more-irreversible-methods-in-the-mtproto.patch) | the mtproto console asks before more methods that can't be undone | with 199 |
| [226](../patches/tdesktop/0226-fix-streamer-mode-hides-the-title-and-every-phone-nu.patch) | streamer mode also hides the window title and every phone number, not only yours | with 163 |
| [227](../patches/tdesktop/0227-fix-deletions-that-fail-on-the-server-come-back.patch) | messages whose deletion fails on the server come back with a note instead of silently disappearing | always on |
| [228](../patches/tdesktop/0228-feat-retry-failed-messages.patch) | failed messages can all be sent again at once from the menu of any of them | always on |
| [229](../patches/tdesktop/0229-feat-find-any-group-member-when-mentioning.patch) | typing @ in a big group finds any member on the server, not only the ones already loaded | always on |
| [230](../patches/tdesktop/0230-fix-load-the-call-log-in-the-right-page-sizes.patch) | the call log loads all of its pages | always on |
| [231](../patches/tdesktop/0231-fix-read-archived-stickers-from-cache-for-every-acco.patch) | archived sticker sets show for every account, not only the first | always on |
| [232](../patches/tdesktop/0232-fix-retry-loading-gift-collections.patch) | gift collections that fail to load are retried | always on |
| [233](../patches/tdesktop/0233-fix-shared-media-calendar-follows-the-open-sublist.patch) | the shared media calendar jumps within the saved messages chat you have open | always on |
| [234](../patches/tdesktop/0234-perf-keep-deleted-messages-without-stalling.patch) | kept deleted messages are saved without freezing tele. older tele versions can't read the kept history after this update | with 186 |
| [235](../patches/tdesktop/0235-fix-send-scheduled-messages-on-time-in-archived-chat.patch) | scheduled messages in archived chats are sent on time too, and one that can't be sent says why | with 97 |
| [236](../patches/tdesktop/0236-feat-see-and-release-messages-queued-behind-an-uploa.patch) | messages waiting behind an upload say what they wait for, and can be sent now or dropped from their menu | with 51 |
| [237](../patches/tdesktop/0237-feat-show-pending-ghost-scheduled-messages.patch) | messages ghost mode holds back to send later show in the chat: right-click one to send it now or cancel it | tele → privacy, on |
| [238](../patches/tdesktop/0238-fix-unsorted-folder-keeps-unread-chats.patch) | the unsorted folder keeps chats with unread messages | with 165 |
| [239](../patches/tdesktop/0239-feat-ignored-users-and-hidden-words-can-stay-silent.patch) | messages from ignored users and with hidden words don't notify or count as unread, and the chat list doesn't preview them | tele → privacy, on |
| [240](../patches/tdesktop/0240-fix-shots-without-time-keep-reactions-below-the-text.patch) | shots without the time keep reactions under the text | with 114 |
| [241](../patches/tdesktop/0241-fix-retry-tele-updates-sooner-and-resume-downloads.patch) | tele updates retry sooner after a failure and continue a broken download instead of starting over | with 6 |
| [242](../patches/tdesktop/0242-fix-invisible-stories-follow-ghost-settings-and-surv.patch) | watching stories invisibly follows per-chat ghost settings, keeps who you watched across restarts, and asks before a live story, which shows you to its author | with 4 |
| [243](../patches/tdesktop/0243-fix-tgs-stickers-telegram-would-reject.patch) | a .tgs sticker with the gzip header most windows tools write is fixed before sending, so telegram doesn't turn it into a file, and gift studio exports always load as stickers | always on |
| [244](../patches/tdesktop/0244-feat-show-the-start-parameter-on-the-start-button.patch) | a bot opened by a link shows the start parameter on its start button | tele → bots, off |
| [245](../patches/tdesktop/0245-feat-mentions-and-replies-in-the-notification-centre.patch) | mentions and replies from groups, muted ones included, are logged in the notification centre. click one to open the message | tele → notifications, on |
| [246](../patches/tdesktop/0246-feat-no-edited-mark-on-bot-messages.patch) | bot messages don't show the edited mark, since bots edit messages to update them | tele → bots, off |
| [247](../patches/tdesktop/0247-feat-more-gifts-and-models-in-the-gift-studio.patch) | the gift studio opens on a random gift, lists the newest collections first, and adds the original model, your own .tgs models, regular gifts, a teddy bear collection and renaming the model, backdrop and symbol on the card | with 201 |
| [248](../patches/tdesktop/0248-feat-your-own-reaction-strip.patch) | choose how many reactions sit above the message menu and their order: click a reaction in the live preview to replace and pin it, drag to reorder | tele → chats, off |
| [249](../patches/tdesktop/0249-feat-captioned-stickers-go-to-the-docked-media.patch) | stickers with a caption go to the media attached above the field, and stickers and gifs can be attached there from their panels | with 109 |
| [250](../patches/tdesktop/0250-feat-browse-every-tele-version-s-changes.patch) | browse and search every tele version's changes | with 112 |
| [251](../patches/tdesktop/0251-fix-late-sent-messages-show-their-real-sent-time.patch) | a message that went out late shows the time telegram actually sent it | always on |
| [252](../patches/tdesktop/0252-fix-notification-centre-shows-whole-entries.patch) | the notification centre shows whole entries, long ones with show more, and plays the press ripple | with 210 |
| [253](../patches/tdesktop/0253-feat-copied-values-in-the-notification-centre.patch) | copy toasts are logged in the notification centre with the copied value: click it to copy it again | tele → notifications, on |
| [254](../patches/tdesktop/0254-feat-what-s-new-as-a-settings-page-with-live-toggles.patch) | what's new is a settings page: every setting a version added is a live toggle right there | with 250 |
| [255](../patches/tdesktop/0255-feat-choose-how-silent-videos-are-sent.patch) | choose whether videos without sound are sent as gifs or as videos | tele → sending |
| [256](../patches/tdesktop/0256-feat-skip-the-mini-app-terms-box-for-trusted-apps.patch) | links to a mini app you've already opened skip its terms box | tele → bots, off |
| [257](../patches/tdesktop/0257-fix-last-code-line-number-sits-on-its-line.patch) | the last line number of a code block sits on its line | with 145 |
| [258](../patches/tdesktop/0258-feat-live-previews-in-settings.patch) | the ghost mode, invisible names and peer ids settings show a live preview of what they change | always on |
| [259](../patches/tdesktop/0259-feat-markdown-renders-as-you-type.patch) | markdown renders while you type: bold, italic, strike, spoilers, code and code blocks, with the markers kept and dimmed. lines starting with > become quotes, >! expandable quotes | tele → sending, off |
| [260](../patches/tdesktop/0260-feat-reply-and-mention-badges-in-the-notification-ce.patch) | replies and mentions in the notification centre have a reply or @ badge on the sender's userpic | with 245 |
| [261](../patches/tdesktop/0261-feat-open-anyone-s-gifts-in-the-gift-grid.patch) | open anyone's gifts, or one of their collections, in the gift grid, or import them into a grid. big grids scroll, rows can be left out of the export and single gifts hidden | with 209 |
| [262](../patches/tdesktop/0262-feat-rarity-tiers-and-pills-in-the-gift-studio.patch) | crafted models show their rarity tier, and rarities in the gift studio sit in coloured pills | with 201 |
| [263](../patches/tdesktop/0263-feat-never-open-web-apps-in-fullscreen.patch) | mini apps always open in a window and can't go fullscreen | tele → bots, off |
| [264](../patches/tdesktop/0264-fix-fullscreen-border-only-on-the-main-window.patch) | fullscreen windows like the media viewer no longer have a 1 px gap at the edges (windows) | with 36 |
| [1001](../patches/Telegram/lib_ui/0001-fix-fullscreen-mini-apps-cover-the-screen-with-their.patch) | a mini app that opens in fullscreen covers the screen from its corner, with its ⋮ and ✕ buttons, instead of hanging off the screen (windows) | always on |

### [tele 14](https://github.com/nitreojs/tele/releases/tag/v7.2.9-tele.14)

| # | what it does | where to toggle |
|---|---|---|
| [198](../patches/tdesktop/0198-fix-show-oversized-stickers-as-files-like-the-server.patch) | a .tgs sticker telegram would turn into a file (over 64 kb, not 512×512, wrong fps or too long) shows as a file to you too, like everyone else sees it | always on |
| [199](../patches/tdesktop/0199-feat-mtproto-console.patch) | an mtproto console: call any api method with your own session in text, json or json5, with autocomplete, schema hints, validation, a result tree and history. destructive methods ask first, star methods ask for the local password | tele → debug, ctrl+alt+m |
| [200](../patches/tdesktop/0200-feat-show-the-media-s-data-center-in-the-media-viewe.patch) | the media viewer shows which data center the open photo, video or file is stored on | with 23 |
| [201](../patches/tdesktop/0201-feat-gift-studio.patch) | a gift studio: build any collectible from real parts (collection, model, backdrop, symbol, number) on a live cover in several layouts, and export png, mp4, gif or a tgs sticker that passes telegram's checks, with text as outlines. favorites, sweeps and contact sheets, parts from @GiftChanges (api.changes.tg) | tele → profiles and ids, gift menus |
| [202](../patches/tdesktop/0202-feat-fake-gift-upgrades.patch) | fake upgrades: the real upgrade box and spin for any upgradable gift, with the model, backdrop and symbol rolled locally by rarity and the real next number. nothing is sent and no stars are spent | tele → profiles and ids, on |
| [203](../patches/tdesktop/0203-feat-quote-rich-messages.patch) | select part of a rich message and pick quote and reply: the rich editor opens with the selection as a blockquote. a quote without an author no longer disappears when it's sent | tele → messages, on |
| [204](../patches/tdesktop/0204-feat-select-service-messages.patch) | select service messages (joins, pins, photo changes) like normal ones and delete them in bulk | always on |
| [205](../patches/tdesktop/0205-feat-mark-gift-studio-tgs-exports.patch) | tgs stickers from the gift studio carry an invisible mark saying they were made with tele's gift studio. it holds no user, account, device or time data | with 201 |
| [206](../patches/tdesktop/0206-fix-hide-the-upgraded-gift-s-number-and-model-until-.patch) | while a gift upgrade spins, its number and model roll and stay hidden until it lands, like on mobile | always on |
| [207](../patches/tdesktop/0207-feat-send-crash-reports-without-a-dump.patch) | after a crash, sending the report is offered even without a dump, from another version or after a graphics crash, with the reason in the window and in the report | always on |
| [208](../patches/tdesktop/0208-feat-report-freezes.patch) | when tele freezes for 15 seconds it writes a dump, and the next launch offers to send a freeze report (windows and linux) | always on |
| [209](../patches/tdesktop/0209-feat-gift-grid-builder.patch) | a gift grid builder: fill a profile-like grid cell by cell with collectibles or regular gifts, custom ribbons, pins and hidden marks, drag to reorder, save grids, and export png, gif or mp4 with an optional profile frame | tele → profiles and ids |
| [210](../patches/tdesktop/0210-feat-notification-centre.patch) | a notification centre in the main menu: tele toasts, updates, sent crash reports, online alerts and server notices in one list, kept per account for 30 days | tele → notifications, on |
| [211](../patches/tdesktop/0211-feat-rate-limit-notices.patch) | rate limits show a toast with a countdown from 5 seconds, a bar above the chat list from a minute, and a notice when they end, instead of a silent hang | with 210 |
| [212](../patches/tdesktop/0212-feat-server-notices.patch) | notices from the tele server in the notification centre, optionally as a one-time coloured toast | tele → server, on |
| [213](../patches/tdesktop/0213-feat-copy-a-gift-collection-s-link.patch) | right-click a gift collection in a profile to copy its link or share it | always on |

### [tele 13](https://github.com/nitreojs/tele/releases/tag/v7.2.9-tele.13)

| # | what it does | where to toggle |
|---|---|---|
| [179](../patches/tdesktop/0179-feat-sticker-captions.patch) | show the text attached to stickers in a bubble under them, and send or edit stickers with a caption | tele → chats, off |
| [180](../patches/tdesktop/0180-feat-inline-calcmula-results-after.patch) | type an expression and = to see the calcmula result greyed out after it, → or tab inserts it | with 46 |
| [181](../patches/tdesktop/0181-fix-stop-the-chat-list-repainting-while-idle.patch) | the chat list no longer repaints all the time while idle, which froze tele after a few hours | always on |
| [182](../patches/tdesktop/0182-fix-send-large-crash-dumps-zipped.patch) | crash dumps over 20 mb are sent zipped, and a report says why when its dump couldn't be attached | always on |
| [183](../patches/tdesktop/0183-feat-copy-or-delete-aliases-from-their-menu.patch) | right-click an alias in a profile or a message to copy or delete it, or open the profile | with 72 |
| [184](../patches/tdesktop/0184-fix-userpics-in-compact-chat-list-rows.patch) | userpics with a story ring or a badge show correctly in compact chat list rows | always on |
| [185](../patches/tdesktop/0185-feat-preview-chats-in-the-chat-area.patch) | alt+click or holding a userpic opens the chat read-only in the chat area, leaving messages unread | tele → interface, off |
| [186](../patches/tdesktop/0186-feat-keep-deleted-messages-and-edit-history-after-a-.patch) | kept deleted messages and edit history survive a restart, encrypted, for 1, 7 or 30 days or forever | with 19 |
| [187](../patches/tdesktop/0187-fix-who-reacted-list-matches-the-reaction-count.patch) | the list of who reacted matches the reaction count | always on |
| [188](../patches/tdesktop/0188-feat-send-several-gifts-at-once.patch) | send several copies of a gift at once, each with the same caption | tele → profiles and ids, off |
| [189](../patches/tdesktop/0189-feat-dim-hidden-gifts-and-select-several.patch) | hidden gifts are dimmed with a clear badge, and a select mode shows, hides, pins and transfers several gifts at once | tele → profiles and ids, off |
| [190](../patches/tdesktop/0190-feat-search-gifts.patch) | search a profile's collectible gifts by name, number, model, backdrop or symbol, with filter chips | tele → profiles and ids, off |
| [191](../patches/tdesktop/0191-feat-local-password-for-stars-and-gift-actions.patch) | a local password before transferring, converting, selling, upgrading, buying or sending gifts, paid reactions, paid media and subscriptions, each switchable | tele → privacy |
| [192](../patches/tdesktop/0192-fix-who-read-list-follows-the-read-state.patch) | the list of who read your message follows the read state, and readers that aren't loaded yet show up instead of nobody viewed | always on |
| [193](../patches/tdesktop/0193-fix-show-a-chat-s-data-center-from-its-first-photo.patch) | a group's or channel's data center comes from its first photo, marked uncertain when its photos disagree | with 23 |
| [194](../patches/tdesktop/0194-feat-more-bulk-gift-actions.patch) | the gift select mode also sells, removes from sale, converts to stars and adds to a collection, and says why an action doesn't fit the selection | with 189 |
| [195](../patches/tdesktop/0195-feat-page-keys-in-lists-and-safer-box-edges.patch) | page up, page down, home and end scroll lists and boxes, and a click just outside a box no longer closes it | tele → interface, off |
| [196](../patches/tdesktop/0196-feat-search-filter-and-export-star-transactions.patch) | search, filter, total and export star and ton transactions to csv | tele → interface, off |
| [197](../patches/tdesktop/0197-feat-filter-long-lists-by-date.patch) | filter star and ton transactions, shared media and profile gifts by a day or a date range | tele → interface, off |

### [tele 12](https://github.com/nitreojs/tele/releases/tag/v7.2.9-tele.12)

| # | what it does | where to toggle |
|---|---|---|
| [128](../patches/tdesktop/0128-feat-hide-the-gift-button-in-the-message-field.patch) | hide the gift button in the message field | tele → sending, off |
| [129](../patches/tdesktop/0129-feat-bookmark-messages-and-return-to-where-you-were-.patch) | bookmark messages from the message menu and jump back to them from the chat menu. after you send a message while scrolled up, a button returns you to where you were reading | tele → messages, off |
| [130](../patches/tdesktop/0130-feat-pin-sticker-and-emoji-sets.patch) | pin sticker and emoji sets to the front of the panel | tele → chats, off |
| [131](../patches/tdesktop/0131-feat-send-silently-by-default.patch) | send silently by default, everywhere or per chat from the chat menu. the send menu offers sending with sound instead | tele → notifications, off |
| [132](../patches/tdesktop/0132-feat-hide-message-previews-in-the-chat-list.patch) | hide message previews in the chat list, with compact rows | tele → interface, off |
| [133](../patches/tdesktop/0133-feat-confirm-stickers-gifs-and-voice-messages-before.patch) | stickers, gifs and voice messages wait for a second click before sending | tele → sending, off |
| [134](../patches/tdesktop/0134-feat-turn-off-touchpad-gestures.patch) | turn off touchpad gestures: swipe to reply and swipe back, each switchable | tele → chats, off |
| [135](../patches/tdesktop/0135-feat-hide-similar-channels.patch) | hide similar channels after joining and in profiles, each switchable | tele → chats, off |
| [136](../patches/tdesktop/0136-feat-hide-unread-counters.patch) | hide unread counters on folder tabs and the taskbar or tray, leave out the archive, and leave out accounts from their right-click menu | tele → notifications, off |
| [137](../patches/tdesktop/0137-feat-hide-and-show-the-pinned-bar-from-the-chat-menu.patch) | hide or show the pinned bar from the chat menu in chats where you can't unpin | tele → chats, on |
| [138](../patches/tdesktop/0138-feat-record-voice-without-fading.patch) | voice messages record without the fade-in at the start | tele → sending, off |
| [139](../patches/tdesktop/0139-fix-close-the-tele-chat-profile-card-after-a-resize.patch) | the tele chat's profile card closes after a window resize on macos | always on |
| [140](../patches/tdesktop/0140-feat-turn-off-ai-features.patch) | turn off telegram's ai features with one switch | tele → chats, off |
| [141](../patches/tdesktop/0141-feat-choose-the-map-service-for-locations.patch) | choose the map service that opens locations | tele → messages → open links in apps |
| [142](../patches/tdesktop/0142-feat-copy-messages-as-markdown-or-html.patch) | copy messages as markdown or html from the message menu | tele → menus, off |
| [143](../patches/tdesktop/0143-feat-loop-videos-in-the-media-viewer.patch) | r loops the video in the media viewer | always on |
| [144](../patches/tdesktop/0144-fix-count-typed-emoji-in-recent-emoji.patch) | emoji you type count in recent emoji | always on |
| [145](../patches/tdesktop/0145-feat-number-the-lines-of-long-code-blocks.patch) | long code blocks get line numbers, in regular and rich messages | tele → messages, off |
| [146](../patches/tdesktop/0146-feat-hide-the-collectible-status-tooltip-in-profiles.patch) | no tooltip for collectible statuses in profiles | tele → profiles and ids, off |
| [147](../patches/tdesktop/0147-feat-rewrite-only-the-link-preview.patch) | link rewrites can change only the link preview and keep your link as it is | with 78 |
| [148](../patches/tdesktop/0148-feat-open-the-message-menu-next-to-the-bubble.patch) | right-clicking beside a bubble opens its message menu | tele → menus, off |
| [149](../patches/tdesktop/0149-fix-taller-shot-box-docked-shot-sending-and-a-center.patch) | the shot box is taller, shots send from attached media above the field, and the draft button is centered | always on |
| [150](../patches/tdesktop/0150-feat-remember-when-your-messages-were-read.patch) | the message menu keeps showing when a private chat read your message, even after telegram stops telling | tele → messages, on |
| [151](../patches/tdesktop/0151-feat-calcmula-suggestions-and-sending-without-calcul.patch) | calcmula suggestions while typing, each can be sent as its own result, and alt+enter sends without calculating | with 82 |
| [152](../patches/tdesktop/0152-feat-open-view-as-tl-inside-tele.patch) | view as tl opens inside tele, titled with the object's constructor. with -teleoffline it opens in the browser | with 33 |
| [153](../patches/tdesktop/0153-feat-make-build-the-bare-build-number.patch) | {build} in the title bar label is the bare build number, the default label is TELE #{build}, and custom labels are updated once | with 10 |
| [154](../patches/tdesktop/0154-feat-choose-which-deleted-messages-to-keep.patch) | choose which deleted messages to keep: in bot chats, in saved messages, and the ones you delete yourself | with 19 |
| [155](../patches/tdesktop/0155-feat-interface-scale-in-1-steps.patch) | the interface scale slider moves in 1% steps | tele → interface, off |
| [156](../patches/tdesktop/0156-fix-send-mkv-files-as-videos.patch) | .mkv files are sent as videos | always on |
| [157](../patches/tdesktop/0157-feat-reduce-motion.patch) | reduce motion: every animation limit except calls on, no sticker loops, reaction bursts or message effects. turning it off restores your settings | tele → interface, off |
| [158](../patches/tdesktop/0158-feat-notify-when-people-come-online.patch) | get a notification when chosen people come online, from their profile or chat menu | tele → notifications, off |
| [159](../patches/tdesktop/0159-feat-mute-folders-and-silence-chats-completely.patch) | mute a whole folder on this device, and silence chats completely, mentions and replies included | tele → notifications, on |
| [160](../patches/tdesktop/0160-feat-send-an-audio-file-as-a-voice-message.patch) | send an audio file as a voice message with a waveform, from its right-click menu in the send box | tele → sending, on |
| [161](../patches/tdesktop/0161-feat-download-folders-per-chat.patch) | downloads go into a folder per chat, and any chat can get its own folder from the chat menu | tele → sending, off |
| [162](../patches/tdesktop/0162-feat-resize-the-message-field.patch) | drag the edge above the message field to set its minimum height | tele → sending, off |
| [163](../patches/tdesktop/0163-feat-streamer-mode.patch) | streamer mode: tele is hidden from screen capture, notifications show no names or text, and your phone number is hidden | tele → privacy, off |
| [164](../patches/tdesktop/0164-feat-translate-your-message-before-sending.patch) | translate your message before sending from the field's right-click menu or the send menu. ctrl+z brings the original back | tele → sending, on |
| [165](../patches/tdesktop/0165-feat-unsorted-folder.patch) | an unsorted folder with every chat that isn't in another folder, archived ones aside | tele → interface, off |
| [166](../patches/tdesktop/0166-fix-gif-frame-timing-and-same-name-downloads.patch) | gif frames show for their own duration, and downloads with the same name no longer overwrite each other | always on |
| [167](../patches/tdesktop/0167-feat-keep-saved-gifs-and-stickers-past-the-limit.patch) | saved gifs and favorite stickers past telegram's limit stay on this device | tele → chats, off |
| [168](../patches/tdesktop/0168-feat-select-several-chats-in-the-list.patch) | alt+click selects chats, shift+click a range. a bar marks them read, archives, mutes, adds them to a folder, deletes or leaves them | tele → interface, off |
| [169](../patches/tdesktop/0169-feat-hide-messages-by-keyword-or-regex.patch) | hide messages by keyword or regex, everywhere and per chat, like ignored users | tele → privacy |
| [170](../patches/tdesktop/0170-fix-show-tele-menu-items-only-when-their-feature-is-.patch) | tele's menu items only show while their feature is on, and each one can be turned off | always on |
| [171](../patches/tdesktop/0171-feat-regex-search-in-loaded-history.patch) | a .* button in the chat search finds messages by regex among the loaded ones, without asking the server | tele → chats, off |
| [172](../patches/tdesktop/0172-feat-regroup-tele-settings-pages.patch) | tele settings are split into interface, chats, messages, sending, notifications, menus, privacy, profiles and ids, bots and debug. old links still work | always on |
| [173](../patches/tdesktop/0173-feat-collapsible-sub-toggles-in-tele-settings.patch) | a setting with sub-settings folds them away, shows how many are on and switches them all off without forgetting them. turning one on with nothing selected turns all of them on | always on |
| [174](../patches/tdesktop/0174-feat-rearrange-menus-with-groups-and-submenus-in-a-m.patch) | tele's own layout for the message, chat, profile, chat list, folder, send and field menus, with groups, submenus and thin or thick separators. an editor rearranges, hides and groups items with a live preview and presets, alt+right-click opens it from a menu | tele → menus |
| [175](../patches/tdesktop/0175-feat-hide-phone-numbers-in-profiles.patch) | the mobile row is hidden in every profile, yours included | tele → profiles and ids, off |
| [176](../patches/tdesktop/0176-feat-choose-how-quoted-names-of-deleted-messages-lin.patch) | the name in a quote of a deleted message can link to the author's profile or mention them | with 19 |
| [177](../patches/tdesktop/0177-fix-land-the-send-animation-where-the-message-really.patch) | the sending animation lands on the message even when another message arrives during it | always on |
| [178](../patches/tdesktop/0178-feat-attach-media-above-the-field-is-out-of-beta.patch) | attaching media above the field is out of beta | with 109 |

### [tele 11](https://github.com/nitreojs/tele/releases/tag/v7.2.9-tele.11)

| # | what it does | where to toggle |
|---|---|---|
| [109](../patches/tdesktop/0109-feat-attach-media-above-the-field-beta.patch) | attach media above the input field instead of in a full-window box: reorder, remove, group or send as files right there, edit photos, see the album as a grid, and the unsent media stays in the chat | tele → sending, off |
| [110](../patches/tdesktop/0110-fix-show-server-badges-on-the-personal-channel-in-pr.patch) | checkmarks and custom verification from the tele server show on a profile's personal channel too | always on |
| [111](../patches/tdesktop/0111-fix-explain-a-missing-changelog-instead-of-doing-not.patch) | when a version's changelog can't be found, the tele chat says so instead of doing nothing | always on |
| [112](../patches/tdesktop/0112-feat-show-new-settings-after-an-update.patch) | after an update, a new in tele page lists the settings that version added. the what's new message links to it | always on |
| [113](../patches/tdesktop/0113-feat-never-block-screen-capture.patch) | tele never blocks screenshots, screen recording or screen sharing, protected chats and disappearing media included | tele → privacy, on |
| [114](../patches/tdesktop/0114-feat-choose-what-shots-show.patch) | choose what shots show: names, userpics, reactions, time and checks, replies, forwarded headers, buttons and link previews | with 89 |
| [115](../patches/tdesktop/0115-fix-trim-settings-descriptions.patch) | setting descriptions only say what their names can't | always on |
| [116](../patches/tdesktop/0116-feat-forward-protected-messages-as-copies.patch) | forward messages from chats that forbid it: tele sends copies and uploads their media again | tele → messages, off |
| [117](../patches/tdesktop/0117-fix-stop-videos-at-their-end.patch) | videos stop at their end, and play starts them again | always on |
| [118](../patches/tdesktop/0118-feat-caption-docked-media-and-bring-it-to-parity-wit.patch) | attached media above the field get a caption each and everything the send box has: self-destruct timer, caption above, hd, price, effects and the send menu. they can be saved as drafts, which keep video edits and price, and an edited draft video says its edits apply when sent | with 109 |
| [119](../patches/tdesktop/0119-feat-hide-settings-until-their-parent-is-on.patch) | settings that only matter while another one is on stay hidden until it is | always on |
| [120](../patches/tdesktop/0120-feat-hide-communities.patch) | hide communities: their chats stay in the list as regular chats | tele → interface, off |
| [121](../patches/tdesktop/0121-feat-send-replies-to-older-messages-past-the-queue.patch) | replies to messages older than the uploading media don't wait in the queue | with 51 |
| [122](../patches/tdesktop/0122-feat-send-right-away-on-double-enter.patch) | enter twice sends a queued message right away | with 51 |
| [123](../patches/tdesktop/0123-feat-regroup-the-sending-settings.patch) | the sending settings are split into sending order, uploads, message field and scheduled messages | always on |
| [124](../patches/tdesktop/0124-feat-use-aliases-for-inline-bots.patch) | username aliases work for inline bots too: `@alias query` asks the real bot | with 72 |
| [125](../patches/tdesktop/0125-feat-hide-mention-and-reaction-badges-in-the-chat-li.patch) | the options that hide the mentions and reactions buttons also hide the @ and ❤️ badges in the chat list and topic tabs | with 73 |
| [126](../patches/tdesktop/0126-feat-pin-chats-past-the-limit.patch) | pin more chats than telegram allows: the extra pins stay on this device, have a hollow pin icon and can be dragged anywhere among the server pins | tele → interface, off |
| [127](../patches/tdesktop/0127-feat-folders-past-the-limit.patch) | more folders and more chats per folder than telegram allows: what doesn't fit stays on this device | tele → interface, off |

### [tele 10](https://github.com/nitreojs/tele/releases/tag/v7.2.9-tele.10)

| # | what it does | where to toggle |
|---|---|---|
| [84](../patches/tdesktop/0084-feat-move-chats-instantly-in-the-chat-list.patch) | chats move to their new place in the chat list right away, without the pause while the mouse moves over the list | tele → interface, off |
| [85](../patches/tdesktop/0085-feat-show-collectible-prices-at-the-purchase-date.patch) | collectible usernames and numbers show their dollar price at the purchase date, not today's | tele → profiles and ids, off |
| [86](../patches/tdesktop/0086-feat-set-your-own-app-and-tray-icons.patch) | your own app and tray icons from any image, and the tray icon can follow the app icon | tele → interface → app icon |
| [87](../patches/tdesktop/0087-feat-choose-when-links-get-previews-and-set-your-own.patch) | choose when links get previews: for all links, not for pasted links, or only ones you add. add a preview of any link to any message, with its size and position | tele → messages |
| [88](../patches/tdesktop/0088-feat-turn-username-into-a-t.me-link-in-the-link-box.patch) | in the link box (ctrl+k), @username becomes a t.me link | always on |
| [89](../patches/tdesktop/0089-feat-save-messages-as-a-picture.patch) | shot: save selected messages, or one message, as a picture from the message menu, with or without the chat background and dates | tele → menus, off |
| [90](../patches/tdesktop/0090-feat-debug-logs-and-an-mtproto-inspector.patch) | a debug page: debug logs on or off, open the logs or tele folder, clear logs, and an mtproto inspector for the logs that filters requests, expands objects, opens entries on [schema.jppgr.am](https://schema.jppgr.am) and opens things from message and chat menus | tele → debug |
| [91](../patches/tdesktop/0091-feat-new-tele-app-and-tray-icons.patch) | tele's own app and tray icons | always on |
| [92](../patches/tdesktop/0092-fix-name-the-item-Copy-Callback-Data-like-telegram-d.patch) | the menu item is called copy callback data, like in telegram | with 28 |
| [93](../patches/tdesktop/0093-feat-usernames-from-the-tele-server.patch) | the tele server can give accounts extra @usernames, see [running your own server](server.md#running-your-own-server). a server username wins over the real one, your own aliases win over both | tele → server, on |
| [94](../patches/tdesktop/0094-fix-open-custom-emoji-in-rich-and-emoji-only-message.patch) | custom emoji in rich messages and in emoji-only messages open their pack on click, and every pack is counted | always on |
| [95](../patches/tdesktop/0095-feat-send-gifs-as-videos-and-videos-as-gifs.patch) | send gifs as videos and videos as gifs: in the send box, from a message's menu, and with right-click in the gif panel. a gif becomes a video without re-encoding, a video becomes a gif by dropping its sound | tele → sending, on |
| [96](../patches/tdesktop/0096-feat-keep-drafts-on-this-device.patch) | drafts stay on this device and never go to the cloud. a button clears the ones already there | tele → privacy, off |
| [97](../patches/tdesktop/0097-feat-send-scheduled-messages-on-time.patch) | scheduled messages are sent by tele itself at the set time while it's online | tele → sending, off |
| [98](../patches/tdesktop/0098-feat-show-id-and-dc-in-one-row.patch) | when both are shown, the dc goes next to the id in one profile row | tele → profiles and ids, off |
| [99](../patches/tdesktop/0099-feat-redesign-the-title-template-editor.patch) | a new title template editor: variables as chips, a palette with modifiers and presets | with 10 |
| [100](../patches/tdesktop/0100-fix-ignore-reply-counters-on-scheduled-messages.patch) | edited scheduled messages don't show junk reply counters | always on |
| [101](../patches/tdesktop/0101-feat-save-drafts-for-later.patch) | saved drafts: keep several drafts per chat, with media or rich messages, in a drafts section that looks like a chat. edit them like messages, send them now or to another chat, and bring back drafts you discarded | tele → sending → drafts |
| [102](../patches/tdesktop/0102-feat-send-photos-in-hd-by-default.patch) | photos go out in hd by default | tele → sending, on |
| [103](../patches/tdesktop/0103-feat-find-chats-by-their-aliases.patch) | chat search finds chats by their aliases, yours and the server's | with 72 |
| [104](../patches/tdesktop/0104-fix-show-seen-and-reacted-counts-separately.patch) | the seen and reacted menu shows both counts separately | always on |
| [105](../patches/tdesktop/0105-feat-send-an-inline-bot-query-as-text.patch) | an inline bot query can be sent as plain text | always on |
| [106](../patches/tdesktop/0106-feat-choose-which-app-opens-each-service.patch) | choose which app opens each service's links: the official app, an alternative like spotifast, the browser, or any program | tele → messages → open links in apps |
| [107](../patches/tdesktop/0107-feat-scale-shots-up-to-4x-and-show-your-messages-as-.patch) | shots render at 1x to 4x with sharp text, bubbles, userpics, icons, media and animated emoji, can show your own messages as incoming, and the selection clears when a shot opens | with 89 |
| [108](../patches/tdesktop/0108-feat-export-and-import-tele-settings.patch) | export tele settings to a file or as text and import them on any account, system or tele version, with a preview first. people and device settings are optional | tele → backup |

### [tele 9](https://github.com/nitreojs/tele/releases/tag/v7.2.9-tele.9)

| # | what it does | where to toggle |
|---|---|---|
| [59](../patches/tdesktop/0059-feat-keep-messages-you-delete-yourself.patch) | messages you delete yourself are kept too, not only ones deleted by others | with 19 |
| [60](../patches/tdesktop/0060-feat-show-server-and-updates-on-the-tele-settings-pa.patch) | the server and updates rows sit right on the tele settings page | always on |
| [61](../patches/tdesktop/0061-feat-pick-brush-colors-from-the-image.patch) | an eyedropper in the photo editor picks the brush color from the image | always on |
| [62](../patches/tdesktop/0062-feat-show-edit-history-as-a-chat.patch) | edit history opens as its own chat: every version as a full message with its media and time | with 18 |
| [63](../patches/tdesktop/0063-feat-hide-or-confirm-call-and-voice-chat-buttons.patch) | call and voice chat buttons can each be shown, hidden, or ask before calling | tele → chats |
| [64](../patches/tdesktop/0064-feat-group-the-tele-interface-and-profile-settings.patch) | the interface and the profiles and ids pages are split into groups | always on |
| [65](../patches/tdesktop/0065-feat-more-ghost-mode-options.patch) | more ghost mode: its main menu toggle is optional, sending a message while the online status is hidden sets you offline again right away, and reaction animations play once instead of over and over | tele → privacy |
| [66](../patches/tdesktop/0066-feat-show-ids-on-regular-gifts.patch) | a copyable id on regular gifts too, not only on collectible ones | tele → profiles and ids, off |
| [67](../patches/tdesktop/0067-feat-send-inline-results-without-via.patch) | inline bot results go out as your own messages, without via @bot | tele → bots, off |
| [68](../patches/tdesktop/0068-feat-skip-the-rich-message-prompt-on-paste.patch) | no rich message prompt when you paste formatted text | tele → sending, off |
| [69](../patches/tdesktop/0069-feat-set-or-remove-chat-backgrounds-only-for-you.patch) | set your own background for any chat, or remove its background, only for you, from the chat menu | tele → chats → chat backgrounds |
| [70](../patches/tdesktop/0070-feat-skip-or-quote-deleted-messages-when-replying.patch) | replying to a message that got deleted meanwhile either drops the reply or quotes the deleted text | tele → messages |
| [71](../patches/tdesktop/0071-feat-add-test-server-accounts-with-a-plain-right-cli.patch) | a plain right-click on add account offers the test server | always on |
| [72](../patches/tdesktop/0072-feat-link-usernames-to-profiles-locally.patch) | username aliases: your own @alias for any profile, in any letters, clickable and in autocomplete, only for you. aliases in sent messages become real mentions, a toast says what changed | tele → chats → username aliases |
| [73](../patches/tdesktop/0073-feat-hide-the-mentions-and-reactions-buttons.patch) | the jump to mentions and jump to reactions buttons can be hidden | tele → chats, off |
| [74](../patches/tdesktop/0074-feat-pick-ignored-users-and-ghost-chats-like-privacy.patch) | ignored users and ghost chats are picked from a searchable list, like privacy exceptions | tele → privacy |
| [75](../patches/tdesktop/0075-feat-show-what-s-new-in-a-local-tele-chat.patch) | what's new comes from a local tele chat with its own profile, grouped by settings page, once per account the first time you open it after an update. a button opens the full changelog | with 39 |
| [76](../patches/tdesktop/0076-feat-show-names-instead-of-phone-numbers-in-chat-hea.patch) | people who shared their number with you but aren't in your contacts keep their name in the chat header instead of the number | tele → profiles and ids, off |
| [77](../patches/tdesktop/0077-feat-open-the-emoji-panel-and-attach-menu-by-click-o.patch) | the emoji, sticker and gif panel and the attach menu open on click only, not on hover | tele → chats, off |
| [78](../patches/tdesktop/0078-feat-rewrite-pasted-links-with-your-own-rules.patch) | links you paste are rewritten by your own rules, like x.com to fixupx.com. ctrl+z brings the original back | tele → messages → link rewrites |
| [79](../patches/tdesktop/0079-feat-edit-link-rewrite-rules-and-presets.patch) | an editor for link rewrite rules: domains or regex patterns, presets for x, instagram, tiktok, reddit, bluesky and pixiv, and a field to test a link | with 78 |
| [80](../patches/tdesktop/0080-feat-name-people-only-for-you.patch) | give people a name only you see, from their chat menu. their profile keeps the real one | tele → profiles and ids → custom names |
| [81](../patches/tdesktop/0081-feat-add-launch-flags-to-turn-off-tele-s-own-network.patch) | launch flags that keep tele off the network for one launch, see [launch flags](launch-flags.md) | always on |
| [82](../patches/tdesktop/0082-feat-preview-calcmula-results-while-typing.patch) | the calcmula result shows above the input field while you type, like an inline bot. a failed calcmula message goes back into the field once instead of doubling | tele → sending, off |
| [83](../patches/tdesktop/0083-feat-clear-server-data-and-explain-custom-verificati.patch) | clear the tele server's saved data, badges included. click a custom verification icon to see what it means: its description, with the icon next to it | tele → server → clear cached data |

### [tele 8](https://github.com/nitreojs/tele/releases/tag/v7.2.9-tele.8)

| # | what it does | where to toggle |
|---|---|---|
| [38](../patches/tdesktop/0038-feat-turn-off-animated-userpics.patch) | animated userpics can stay still: separately in the chat list and in chats and profiles | tele → interface, off, in chats and profiles: tele → chats, off |
| [39](../patches/tdesktop/0039-feat-show-what-s-new-in-tele-after-an-update.patch) | after an update, you get a message with what's new in that tele version, visible only to you. fresh installs skip it | tele → updates, on |
| [40](../patches/tdesktop/0040-feat-call-the-app-tele-everywhere.patch) | the app calls itself tele everywhere: tray menu, crash window, system menus and more | always on |
| [41](../patches/tdesktop/0041-feat-customize-the-window-title-and-format-template-.patch) | the window title (taskbar, alt+tab, system window frame) can be a template too, or follow the title bar label. every [template](title-template.md) variable takes modifiers like `{weekday|short|lower}` or `{chat|max:20}` | tele → interface |
| [42](../patches/tdesktop/0042-feat-group-and-search-tele-settings.patch) | tele settings are grouped into pages like the main settings, with short descriptions and their own search field, and the main settings search finds them too. old links to tele settings open the right page | always on |
| [43](../patches/tdesktop/0043-feat-show-times-on-service-messages.patch) | service messages like joins and pins show their time too, with seconds | with 17 |
| [44](../patches/tdesktop/0044-feat-upload-media-in-several-chats-at-once.patch) | media uploads in several chats at once instead of waiting for each other. files in one chat still go one after another | tele → sending, off |
| [45](../patches/tdesktop/0045-feat-reply-timestamps-for-media-in-rich-messages.patch) | time codes like 1:23 in a reply to a rich message link to its video or audio, the first one if there are several | always on |
| [46](../patches/tdesktop/0046-feat-compute-messages-starting-with-via-calcmula.patch) | messages and captions starting with `= ` are computed with [calcmula](https://calcmula.app) and sent as the quoted query with `= result` below it. if it fails, nothing is sent and the text goes back into the field | tele → sending, off |
| [47](../patches/tdesktop/0047-feat-move-show-peer-ids-into-tele-settings.patch) | show peer ids moved from experimental settings into tele | tele → profiles and ids |
| [48](../patches/tdesktop/0048-feat-show-the-tele-build-in-the-main-menu.patch) | the main menu shows the tele build next to the version | always on |
| [49](../patches/tdesktop/0049-feat-open-collectible-gifts-in-see.tg.patch) | an open in see.tg link on collectible gift cards | tele → profiles and ids, off |
| [50](../patches/tdesktop/0050-feat-move-late-sent-messages-to-the-bottom.patch) | a message that took long to send moves to the bottom of the chat once it's sent, so it's clear when it went out | tele → sending, off |
| [51](../patches/tdesktop/0051-feat-queue-messages-behind-an-uploading-media.patch) | messages sent while a media is uploading wait for it and go out after it, in order | tele → sending, off |
| [52](../patches/tdesktop/0052-feat-open-links-in-their-desktop-apps.patch) | spotify, steam, discord, zoom, teams, notion, slack and epic links open in their desktop apps when they're installed | tele → messages → open links in apps |
| [53](../patches/tdesktop/0053-feat-clean-tracking-parameters-from-links.patch) | the SUPER MAGA PALANTIR ICE PETER THIEL AI DATA HARVESTER 9000 remover: opened links lose their tracking parameters (utm, fbclid, si, gclid and [more](link-cleaner.md)). when it would change a link, its right-click menu offers open without cleaning. the same remover works on links in sent messages and captions, the rest of the text stays as it is | tele → messages, off |
| [54](../patches/tdesktop/0054-feat-reveal-spoilers-automatically.patch) | text and media spoilers are revealed right away, in chats and the chat list | tele → messages, off |
| [55](../patches/tdesktop/0055-feat-move-tele-tools-to-the-bottom-of-the-message-me.patch) | tele's items sit at the bottom of the message menu: view as tl, then the message id | always on |
| [56](../patches/tdesktop/0056-feat-copy-custom-emoji-ids-from-the-message-menu.patch) | right-click a custom emoji in a message to copy its id | tele → menus, off |
| [57](../patches/tdesktop/0057-fix-change-the-speed-instead-of-moving-the-media-vie.patch) | dragging while holding a video to speed it up changes the speed instead of moving the media viewer window, and the speedup no longer stops by itself | always on |
| [58](../patches/tdesktop/0058-feat-reorder-and-hide-message-menu-items.patch) | reorder and hide items of the message menu, items it doesn't know keep their place | tele → menus |

### [tele 7](https://github.com/nitreojs/tele/releases/tag/v7.2.9-tele.7)

| # | what it does | where to toggle |
|---|---|---|
| [17](../patches/tdesktop/0017-feat-show-seconds-in-message-times.patch) | message times show seconds, like 14:03:21 | tele → messages, off |
| [18](../patches/tdesktop/0018-feat-keep-the-edit-history-of-messages.patch) | remembers what edited messages looked like while tele runs: right-click an edited message → edit history | tele → messages, off |
| [19](../patches/tdesktop/0019-feat-keep-deleted-messages.patch) | messages deleted by others or from your other devices stay in the chat until tele restarts, service messages too, faded as a whole or with a trash icon by the time. each chat gets a page with its kept deleted messages and a clear all button: chat menu → deleted messages | tele → messages, off |
| [20](../patches/tdesktop/0020-fix-fade-every-unsupported-experimental-option.patch) | experimental options your system doesn't support are faded completely, title included | always on |
| [21](../patches/tdesktop/0021-feat-hide-call-buttons.patch) | no call button in private chats and profiles | tele → chats |
| [22](../patches/tdesktop/0022-feat-lowercase-every-interface-text.patch) | lowercases the whole interface. messages and names stay as they are | tele → interface, off, needs a restart |
| [23](../patches/tdesktop/0023-feat-show-the-data-center-in-profiles.patch) | a dc row in profiles: the data center the account or chat lives in | tele → profiles and ids, off |
| [24](../patches/tdesktop/0024-feat-hide-sponsored-messages.patch) | no ads: no sponsored messages in channels and bots, no video ads, no sponsored search results | tele → chats, off |
| [25](../patches/tdesktop/0025-feat-open-links-without-confirmation.patch) | links with custom text open right away, without the confirmation | tele → messages, off |
| [26](../patches/tdesktop/0026-feat-open-disappearing-media-without-burning-it.patch) | view-once and timed media open without burning, and the sender still sees them unopened. message menu → mark as viewed burns them | tele → messages, off |
| [27](../patches/tdesktop/0027-feat-allow-screenshots-of-disappearing-media.patch) | view-once and timed media can be screenshotted and recorded | tele → privacy, off |
| [28](../patches/tdesktop/0028-feat-copy-the-callback-data-of-bot-buttons.patch) | right-click over a bot button to copy its callback data, inline query, web app url and so on | always on |
| [29](../patches/tdesktop/0029-feat-show-the-message-id-in-the-message-menu.patch) | the message menu ends with the message id, click it to copy | tele → menus, off |
| [33](../patches/tdesktop/0033-feat-view-messages-and-telegram-objects-as-tl.patch) | view as tl in the message menu, and for chats, profiles, members, topics, stickers and sets, custom emoji, gifts, stories and folders: fetches the object from the server and opens it on [schema.jppgr.am](https://schema.jppgr.am) | tele → menus, off |
| [30](../patches/tdesktop/0030-feat-hide-the-all-chats-folder.patch) | hides the all chats folder when you have other folders, the list opens on your first one | tele → interface, off |
| [31](../patches/tdesktop/0031-feat-jump-to-the-first-message-of-a-chat.patch) | jump to the first message, in the chat menu | tele → menus, off |
| [32](../patches/tdesktop/0032-feat-add-ghost-mode.patch) | ghost mode: no read receipts, typing, online status or story views, each switchable, and per chat always or never from the chat menu. optionally reads a chat when you reply, and sends messages as scheduled a few seconds ahead so sending doesn't put you online. the message menu has mark as read up to here, and the side menu has a quick toggle | tele → privacy, off |
| [34](../patches/tdesktop/0034-feat-hide-the-mtproxy-sponsor-channel.patch) | no sponsor channel pinned to the chat list when you connect through an mtproxy | tele → chats, off |
| [35](../patches/tdesktop/0035-feat-lowercase-tele-s-own-texts-too.patch) | lowercase covers tele's own texts and the crash window too | with 22 |
| [36](../patches/tdesktop/0036-fix-stop-maximized-windows-jittering-on-monitors-wit.patch) | a maximized window no longer jitters on a monitor without a taskbar (windows) | always on |
| [37](../patches/tdesktop/0037-feat-send-crash-reports-to-the-tele-server.patch) | when tele crashed, the next start offers to send the crash report to the tele server instead of telegram. nothing leaves without your click | tele → server, on with the server |

### [tele 5](https://github.com/nitreojs/tele/releases/tag/v7.2.9-tele.5)

| # | what it does | where to toggle |
|---|---|---|
| [13](../patches/tdesktop/0013-feat-show-gift-ids.patch) | a copyable gift id at the top of the collectible gift card, which only says a gift is on the ton blockchain while it really is there | tele → profiles and ids, off |
| [14](../patches/tdesktop/0014-feat-show-the-peer-id-as-its-own-profile-row.patch) | the peer id gets its own profile row instead of trailing the bio, optionally without spaces or in bot api style (-100… for channels) | tele → profiles and ids, off (see 47) |
| [15](../patches/tdesktop/0015-feat-copy-links-to-tele-settings.patch) | right-click any tele setting to copy a link that opens it | always on |
| [16](../patches/tdesktop/0016-feat-ignore-users-by-hiding-or-fading-their-messages.patch) | ignore users from their userpic menu: their messages fade or disappear, the list lives in settings | tele → privacy |

### [tele 4](https://github.com/nitreojs/tele/releases/tag/v7.2.9-tele.4)

| # | what it does | where to toggle |
|---|---|---|
| [8](../patches/tdesktop/0008-feat-send-quick-replies-in-groups-and-channels.patch) | business quick replies in groups and channels too. telegram only sends them in private chats, so tele posts the messages as ordinary ones (no bot keyboards, via-bot labels or effects) | tele → chats, off |
| [9](../patches/tdesktop/0009-feat-apply-verification-and-marks-from-the-tele-serv.patch) | checkmarks, custom verification, scam, fake and support marks from the [tele server](server.md), shown exactly like telegram's own. custom verification icons animate, member lists update as soon as the data arrives, and refresh now fetches it right away | tele → server, on |
| [10](../patches/tdesktop/0010-feat-make-the-title-bar-label-a-live-template.patch) | the title bar label is a template with live variables, see [title bar template](title-template.md) | tele → interface |
| [11](../patches/tdesktop/0011-feat-remove-the-account-limit.patch) | no account limit (well, 1536) | always on |
| [12](../patches/tdesktop/0012-feat-show-checkmarks-and-custom-verification-everywh.patch) | checkmarks and custom verification in the account list, the main menu, the settings header and, optionally, next to sender names in messages. badges go in telegram's order everywhere: custom verification, name, emoji status, checkmark, and the premium star stays visible next to checkmarks | always on, in messages: tele → chats, off |

### [tele 3](https://github.com/nitreojs/tele/releases/tag/v7.2.9-tele.3)

| # | what it does | where to toggle |
|---|---|---|
| [1](../patches/tdesktop/0001-feat-show-the-tele-build-number-in-the-title-bar.patch) | the title bar shows which tele build you're on | always on, see 10 |
| [2](../patches/tdesktop/0002-feat-add-a-tele-section-to-the-settings.patch) | the tele section in settings | always on |
| [3](../patches/tdesktop/0003-feat-add-an-option-to-hide-stories-everywhere.patch) | hides stories everywhere: no stories bar, no rings around userpics, no stories in profiles | tele → privacy, off, needs a restart |
| [4](../patches/tdesktop/0004-feat-add-an-option-to-watch-stories-invisibly.patch) | watch stories without showing up among the viewers. reactions and replies still reveal you | tele → privacy, off |
| [5](../patches/tdesktop/0005-feat-show-bot-button-payloads-in-tooltips.patch) | hovering a bot button shows what it carries: callback data, links, inline queries, web apps and more | tele → bots, off |
| [6](../patches/tdesktop/0006-feat-update-from-the-tele-github-releases.patch) | self-updates from these releases on windows, linux and macos, signed with ed25519 | tele → updates, on |
| [7](../patches/tdesktop/0007-feat-rename-the-app-to-tele.patch) | the app is called tele: `tele.exe`, its own taskbar entry, links point here. linux and macos builds are their own app, so they don't clash with an installed telegram | always on |
