"""ROM-free checks for the human-play capture tooling.

No MAME run, no ROMs: byte-compiles mame/lua/play-capture.lua, exercises
tools/mame-play-capture argument handling, and asserts the lua/env
contract between wrapper and sidecar.
"""

import pathlib
import re
import shutil
import subprocess
import unittest


ROOT = pathlib.Path(__file__).resolve().parents[1]
WRAPPER = ROOT / "tools" / "mame-play-capture"
LUA = ROOT / "mame" / "lua" / "play-capture.lua"

REQUIRED_ENV = (
    "STUNRUN_PLAY_OUT",
    "STUNRUN_PLAY_EVERY",
    "STUNRUN_PLAY_FRAMES",
    "STUNRUN_PLAY_STATES",
)


class PlayCaptureTests(unittest.TestCase):
    def test_lua_compiles(self):
        luac = shutil.which("luac")
        if luac is None:
            self.skipTest("luac not installed")
        proc = subprocess.run([luac, "-p", str(LUA)],
                              capture_output=True, text=True)
        self.assertEqual(proc.returncode, 0, proc.stderr)

    def test_wrapper_help(self):
        proc = subprocess.run([str(WRAPPER), "--help"],
                              capture_output=True, text=True, cwd=ROOT)
        self.assertEqual(proc.returncode, 0)
        self.assertIn("rompath", proc.stdout)

    def test_wrapper_rejects_missing_roms(self):
        proc = subprocess.run(
            [str(WRAPPER), "/nonexistent-rompath", "/tmp/x"],
            capture_output=True, text=True, cwd=ROOT)
        self.assertNotEqual(proc.returncode, 0)

    def test_env_contract(self):
        wrapper_text = WRAPPER.read_text(encoding="utf-8")
        lua_text = LUA.read_text(encoding="utf-8")
        for name in REQUIRED_ENV:
            self.assertIn(name, wrapper_text, f"wrapper never sets {name}")
            self.assertIn(f"os.getenv('{name}'", lua_text,
                          f"sidecar never reads {name}")

    def test_dry_run_clean_boot_flags(self):
        import subprocess
        import tempfile
        with tempfile.TemporaryDirectory() as temp:
            rompath = pathlib.Path(temp) / "roms"
            (rompath / "stunrun").mkdir(parents=True)
            out = pathlib.Path(temp) / "out"
            base = [str(WRAPPER), str(rompath), str(out), "--dry-run"]
            plain = subprocess.run(base, capture_output=True, text=True,
                                   cwd=ROOT)
            self.assertEqual(plain.returncode, 0, plain.stderr)
            self.assertNotIn("-nvram_directory", plain.stdout)
            self.assertNotIn("-cfg_directory", plain.stdout)
            clean = subprocess.run(
                base + ["--fresh-nvram", "--clean-cfg"],
                capture_output=True, text=True, cwd=ROOT)
            self.assertEqual(clean.returncode, 0, clean.stderr)
            self.assertIn("-nvram_directory", clean.stdout)
            self.assertIn("-cfg_directory", clean.stdout)

    def test_propagates_mame_failure(self):
        import tempfile
        with tempfile.TemporaryDirectory() as temp:
            rompath = pathlib.Path(temp) / "roms"
            (rompath / "stunrun").mkdir(parents=True)
            out = pathlib.Path(temp) / "out"
            proc = subprocess.run(
                [str(WRAPPER), str(rompath), str(out), "--mame",
                 "/nonexistent/mame"],
                capture_output=True, text=True, cwd=ROOT)
            self.assertNotEqual(proc.returncode, 0)
            self.assertIn("No such file", proc.stderr)

    def test_no_scripted_inputs(self):
        lua_text = LUA.read_text(encoding="utf-8")
        self.assertNotIn("set_value", lua_text,
                         "play sidecar must not drive inputs")
        self.assertIn("machine:save", lua_text)
        self.assertIn("stunrun-memory-snapshot/v1", lua_text)

    def test_ignores_bus_fill_samples(self):
        lua_text = LUA.read_text(encoding="utf-8")
        self.assertIn("valid_gameplay_sample", lua_text)
        self.assertIn("course ~= 0xffff", lua_text)
        self.assertIn("score ~= 0xffffffff", lua_text)
        self.assertIn("if not valid_gameplay_sample(course, score)", lua_text)


if __name__ == "__main__":
    unittest.main()
