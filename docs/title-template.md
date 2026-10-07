# title bar template

settings → tele → interface. the title bar label and the window title both take a template.

the default is `tele #{build}`. empty hides the label.

- `{build}` (the build number, `DEV` for local builds), `{version}`
- `{time}`, `{date}`, `{weekday}`, `{day}`, `{month}`, or any qt format like `{time:HH:mm:ss}` and `{date:dd MMM}`
- `{name}`, `{username}`, `{id}`, `{accounts}`
- `{unread}`, `{chat}`, `{chat_unread}`, `{status}`

any variable takes modifiers after `|`, applied left to right: `{weekday|short|lower}` is `sun`, `{date:dddd|upper}` is `SUNDAY`.

- `lower`, `upper`: `{weekday|lower}` is `sunday`
- `title` capitalizes every word, `cap` only the first letter
- `short` is the compact form, wherever it sits in the chain: `Sun` and `Sep` for weekday and month, the first name for `{name}` and for people in `{chat}`, `7.2` for `{version}`, `1.2k` for counters, `…` for `{status}`. other variables stay as they are
- `first`, `last`: the first or last word, `{name|first}`
- `max:N` cuts to N characters with `…`, `{chat|max:20}`
- `pad:N` pads to N characters, with zeros for numbers: `{day|pad:2}` is `07`
- `k` shortens numbers: `1234` is `1.2k`, `15000` is `15k`, `2500000` is `2.5m`
- `default:text` shows text when the value is empty or 0, `{status|default:online}`

`[ … ]` hides its part when a variable inside is empty or 0, so `tele #{build}[ · {unread} unread]` doesn't show "· 0 unread". a `default` counts as filled. doubled brackets are literal. a `|` that isn't followed by modifiers stays part of a qt format, and `'|'` in quotes always does. a variable or modifier with a typo is shown as typed. account and activity variables stay empty while the app is locked with a passcode.

the editor highlights variables in the field and points out mistakes under the preview. below are all the variables as chips: click one to insert it, right-click for its other forms with their current values, hover to see the value now. presets has a few ready templates.

the window title (what the taskbar, alt+tab and the system window frame show) takes the same template, set next to the label. `{label}` puts the rendered label there, so `{label}` alone keeps both in sync. empty keeps telegram's own title, and a template that renders to nothing leaves the title blank.
