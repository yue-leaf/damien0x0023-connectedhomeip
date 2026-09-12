"""Compile and run the production KVS implementation against a fake NVM backend.

Requires Python standard library and a C++17 host compiler (CXX, default g++).
Run on Linux/WSL; no Silicon Labs SDK or connected hardware is needed.
"""

import os
from pathlib import Path
import subprocess
import tempfile


def main():
    here = Path(__file__).resolve().parent
    source = here.parents[1]
    with tempfile.TemporaryDirectory(prefix="silabs-kvs-test-") as temporary:
        build = Path(temporary)
        for name in (
            "crypto/CHIPCryptoPAL.h",
            "platform/CHIPDeviceLayer.h",
            "platform/silabs/SilabsConfig.h",
            "lib/core/CHIPPersistentStorageDelegate.h",
            "system/SystemClock.h",
            "system/SystemLayer.h",
        ):
            header = build / name
            header.parent.mkdir(parents=True, exist_ok=True)
            header.write_text('#include "FakePlatform.h"\n', encoding="ascii")
        (build / "platform/KeyValueStoreManager.h").write_text(
            '#include "FakePlatform.h"\n'
            f'#include "{source.as_posix()}/KeyValueStoreManagerImpl.h"\n',
            encoding="ascii",
        )
        executable = build / "kvs-test"
        subprocess.run(
            [os.environ.get("CXX", "g++"), "-std=c++17", "-Wall", "-Wextra",
             "-Wno-unused-parameter", "-Werror", "-I", str(build), "-I", str(here),
             "-I", str(source), str(here / "TestKvs.cpp"), "-o", str(executable)],
            check=True,
        )
        subprocess.run([str(executable)], check=True)


if __name__ == "__main__":
    main()
