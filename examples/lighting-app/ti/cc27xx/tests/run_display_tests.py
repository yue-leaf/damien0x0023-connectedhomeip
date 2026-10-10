#!/usr/bin/env python3
# Copyright 2026 Project CHIP Authors
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
# Unless required by applicable law or agreed to in writing, software distributed
# under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
# CONDITIONS OF ANY KIND, either express or implied. See the License for the
# specific language governing permissions and limitations under the License.

"""Host-only LCD tests. No TI SDK, target compiler, or board is used."""

import argparse
import os
import subprocess
from pathlib import Path


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--cc", default="gcc")
    parser.add_argument("--cxx", default="g++")
    parser.add_argument("--output-dir", type=Path, default=Path("tmp/ti-lcd-tests"))
    args = parser.parse_args()
    tests = Path(__file__).resolve().parent
    source = tests.parent / "src"
    qr = tests.parent.parents[3] / "examples/common/QRCode/repo/c"
    output = args.output_dir.resolve()
    output.mkdir(parents=True, exist_ok=True)
    qr_object = output / "qrcodegen.o"
    subprocess.run([args.cc, "-std=c99", "-c", str(qr / "qrcodegen.c"), "-o", str(qr_object)], check=True)
    flags = ["-std=c++17", "-Wall", "-Wextra", "-Wconversion", "-Werror"]
    if os.name == "nt":
        flags.append("-static")
    flags += ["-I", str(source), "-I", str(qr)]
    suffix = ".exe" if os.name == "nt" else ""
    renderer = output / ("test-onboarding-screen" + suffix)
    manager = output / ("test-display-manager" + suffix)
    status = output / ("test-status-screen" + suffix)
    subprocess.run([args.cxx, *flags, str(tests / "TestOnboardingScreen.cpp"), str(source / "OnboardingScreen.cpp"),
                    str(qr_object), "-o", str(renderer)], check=True)
    subprocess.run([str(renderer), str(output / "onboarding.ppm"), str(output / "invalid-factory-data.ppm")], check=True)
    subprocess.run([args.cxx, *flags, str(tests / "TestStatusScreen.cpp"), str(source / "StatusScreen.cpp"),
                    "-o", str(status)], check=True)
    subprocess.run([str(status), *[str(output / (name + ".ppm")) for name in
                                  ["status-connected", "status-offline", "status-identify", "status-long-fields", "status-unavailable"]]],
                   check=True)
    subprocess.run([args.cxx, *flags, "-I", str(tests / "stubs"), str(tests / "TestDisplayManager.cpp"),
                    str(source / "DisplayManager.cpp"), str(source / "OnboardingScreen.cpp"), str(source / "StatusScreen.cpp"),
                    str(qr_object), "-o", str(manager)], check=True)
    for scenario in ["uncommissioned", "commissioned", "invalid", "failure", "status-failure", "auto-retry",
                     "retry-exhausted", "partial-frame", "retry-update"]:
        subprocess.run([str(manager), scenario], check=True)


if __name__ == "__main__":
    main()
