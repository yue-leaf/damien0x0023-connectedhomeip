# Repository Write Scope

For this TI Matter provisioning work, file writes are limited to these two
project roots:

- `E:\F\iot\damien0x0023-connectedhomeip`
- `E:\project\ti_provision_ota_poc\pc-tmp-tool`

Do not modify installed SDKs, toolchains, IDEs, or flashing tools outside those
roots. In particular, treat all content under `C:\ti` as read-only, including
the SimpleLink SDK, SysConfig, CCS, and UniFlash installations.

Reading external SDK and tool files for diagnosis is allowed. If a required fix
would modify anything outside the two authorized project roots, stop and ask
the user instead. Implement compatibility fixes, wrappers, or overrides inside
one of the authorized projects.
