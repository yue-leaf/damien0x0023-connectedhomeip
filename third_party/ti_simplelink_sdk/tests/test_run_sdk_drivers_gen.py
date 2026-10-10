# Copyright 2026 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import contextlib
import io
import os
import runpy
import subprocess
import sys
import unittest
from pathlib import Path
from unittest.mock import patch


SCRIPT = Path(__file__).resolve().parents[1] / "run_sdk_drivers_gen.py"


class RunSdkDriversGenTest(unittest.TestCase):
    def run_script(self, returncode=0, missing=None, secure=False):
        argv = [str(SCRIPT), "--sdk", "/sdk", "--chip-root", "/chip",
                "--src-path-drivers", "/drivers.a", "--src-path-driverlib", "/driverlib.a",
                "--dest-path", "/out"]
        if secure:
            argv += ["--src-path-hsm", "/hsm.a", "--secure-src-file", "/secure.a",
                     "--secure-dest-path", "/out"]
        output = io.StringIO()
        with patch.object(sys, "argv", argv), \
                patch.dict(os.environ, {"TI_ARM_GCC_ROOT": "/compiler"}), \
                patch("os.path.isdir", return_value=True), \
                patch("os.path.exists", side_effect=lambda path: path != missing), \
                patch("subprocess.run", return_value=subprocess.CompletedProcess(
                    [], returncode, "make stdout", "make stderr")) as build, \
                patch("shutil.copy") as copy, contextlib.redirect_stdout(output):
            status = 0
            try:
                runpy.run_path(str(SCRIPT), run_name="__main__")
            except SystemExit as error:
                status = error.code
        return status, build, copy, output.getvalue()

    def test_build_failure_does_not_copy_stale_archives(self):
        status, build, copy, output = self.run_script(returncode=2, secure=True)
        self.assertEqual(status, 1)
        build.assert_called_once()
        copy.assert_not_called()
        self.assertIn("make stderr", output)
        self.assertIn("res.returncode: 2", output)

    def test_success_copies_drivers(self):
        status, build, copy, _ = self.run_script()
        self.assertEqual(status, 0)
        self.assertEqual(copy.call_count, 2)
        self.assertIn("GENERATOR=Ninja", build.call_args.args[0])
        self.assertIn("GCC_ARMCOMPILER=/compiler", build.call_args.args[0])
        copy.assert_any_call("/sdk/drivers.a", "/out")
        copy.assert_any_call("/sdk/driverlib.a", "/out")

    def test_success_copies_secure_archives(self):
        status, _, copy, _ = self.run_script(secure=True)
        self.assertEqual(status, 0)
        self.assertEqual(copy.call_count, 4)
        copy.assert_any_call("/sdk/hsm.a", "/out")
        copy.assert_any_call("/sdk/secure.a", "/out")

    def test_missing_archive_fails(self):
        for archive in ["drivers.a", "driverlib.a", "hsm.a", "secure.a"]:
            with self.subTest(archive=archive):
                status, _, _, _ = self.run_script(missing="/sdk/" + archive, secure=True)
                self.assertEqual(status, 2)


if __name__ == "__main__":
    unittest.main()
