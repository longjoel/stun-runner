"""Verify that the C ADSP emitter produces the Python builder's bytes."""
import pathlib
import shutil
import subprocess
import tempfile
import unittest
import hashlib
import json


ROOT = pathlib.Path(__file__).resolve().parents[1]
SRC = ROOT / "reproduction" / "adsp"
FIXTURES = ("nop", "reset-loop", "init-prefix", "init-state")


class AdspCRuntimeImageTests(unittest.TestCase):
    def test_c_emitter_matches_python_image_bytes(self):
        cc = shutil.which("cc") or shutil.which("gcc") or shutil.which("clang")
        self.assertIsNotNone(cc, "no C compiler available")
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            emitter = root / "adsp-image-dump"
            compile_proc = subprocess.run(
                [cc, "-std=c99", "-Wall", "-Wextra", "-I", str(SRC),
                 "-o", str(emitter), str(SRC / "adsp_init_image.c"),
                 str(SRC / "adsp_image_dump.c")],
                capture_output=True, text=True, check=False)
            self.assertEqual(compile_proc.returncode, 0,
                             f"C compile failed:\n{compile_proc.stderr}")
            self.assertEqual(compile_proc.stderr, "",
                             f"C compile warnings:\n{compile_proc.stderr}")
            for fixture in FIXTURES:
                c_out = root / (fixture + "-c")
                py_dir = root / (fixture + "-py")
                c_dir = root / (fixture + "-manifest")
                run = subprocess.run([str(emitter), fixture, str(c_out)],
                                     capture_output=True, text=True,
                                     check=False)
                self.assertEqual(run.returncode, 0,
                                 f"C emitter failed for {fixture}: {run.stderr}")
                subprocess.run(
                    [str(ROOT / "tools" / "build-replacement-image"),
                     "--processor", "adsp2100", "--fixture", fixture,
                     "--output", str(py_dir)], check=True,
                    capture_output=True, text=True)
                # The wrapper is also checked independently; its manifest
                # must describe exactly the bytes emitted by the C utility.
                subprocess.run(
                    [str(ROOT / "tools" / "build-c-replacement-image"),
                     "--emitter", str(emitter), "--fixture", fixture,
                     "--output", str(c_dir)], check=True,
                    capture_output=True, text=True)
                self.assertEqual(c_out.read_bytes(),
                                 py_dir.joinpath("adsp2100.bin").read_bytes())
                manifest = json_load(c_dir / "image.json")
                self.assertEqual(manifest["sha256"], hashlib_sha256(c_out))


def json_load(path):
    return json.loads(path.read_text(encoding="utf-8"))


def hashlib_sha256(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


if __name__ == "__main__":
    unittest.main()
