# KVS reset persistence regression

Run `python3 src/platform/silabs/tests/kvs_host/run.py` from the repository root
on Linux or WSL. Requires a C++17 compiler (`CXX`, default `g++`) and Python's
standard library. The factory workflow runs this test before building firmware.

The test compiles the production KVS implementation and header against an
in-memory storage backend. It covers 248 stale slots, preservation of live data,
200 immediate-reset cycles, storage read/delete/write failures, hash collisions,
and truncated maps. It does not simulate NVM3 timing, real cryptography, hardware
reset behavior, or a complete Matter commissioning exchange.

Hardware verification requires the factory workflow's new image. UART logging
is enabled via `--uart_log`; Matter progress/detail logging is restored, while
shell and OpenThread CLI remain disabled. The existing OTA-slot overlap check
must still pass; logging must not be enabled by relaxing the flash boundary.

For recovery validation, preserve the failing device's NVM3 area during the
application upgrade. Expect `KVS recovered 248 stale index slots` only when that
exact fault snapshot is present; the number varies on other devices. Then run
at least 200 commission/unpair cycles and confirm slot usage does not grow with
each cycle. A clean flash alone cannot validate recovery of an existing index.
