"""Compare the native checkpoint emitter with the checked-in M1 fixture."""
import json
import pathlib
import re
import shutil
import subprocess
import tempfile
import unittest


ROOT = pathlib.Path(__file__).resolve().parents[1]


class CheckpointGoldenTests(unittest.TestCase):
    def test_c_emitter_matches_m1_fixture(self):
        cc = shutil.which("cc") or shutil.which("gcc") or shutil.which("clang")
        self.assertIsNotNone(cc, "no C compiler available")
        with tempfile.TemporaryDirectory() as directory:
            binary = pathlib.Path(directory) / "checkpoint-test"
            compile_proc = subprocess.run(
                [cc, "-std=c99", "-Wall", "-Wextra", "-o", str(binary),
                 str(ROOT / "native" / "checkpoint.c"),
                 str(ROOT / "native" / "checkpoint_test.c")],
                capture_output=True, text=True, check=False)
            self.assertEqual(compile_proc.returncode, 0,
                             f"C compile failed:\n{compile_proc.stderr}")
            self.assertEqual(compile_proc.stderr, "",
                             f"C compile warnings:\n{compile_proc.stderr}")
            run = subprocess.run([str(binary)], capture_output=True,
                                 text=True, check=False)
            self.assertEqual(run.returncode, 0,
                             f"checkpoint self-check failed:\n{run.stdout}")
            match = re.search(
                r"--- document begins ---\n(\{.*\})\n--- document ends ---",
                run.stdout, re.DOTALL)
            self.assertIsNotNone(match, "checkpoint JSON markers missing")
            emitted = json.loads(match.group(1))
            expected = json.loads(
                (ROOT / "reference/checkpoints/m1-machine-map/state.json")
                .read_text(encoding="utf-8"))
            self.assertEqual(emitted, expected)


if __name__ == "__main__":
    unittest.main()
