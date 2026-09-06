"""Public ROM-free tests for the arcade-experiment/v1 C parser.

Compiles native/experiment.c with the system C compiler and runs its
self-check (valid, corrupt, and schema-violating fixtures). No ROMs,
no MAME, no network required.
"""
import pathlib
import shutil
import subprocess
import tempfile
import unittest


ROOT = pathlib.Path(__file__).resolve().parents[1]
NATIVE = ROOT / "native"


class ExperimentParserTests(unittest.TestCase):
    def test_c_self_check_passes(self):
        cc = shutil.which("cc") or shutil.which("gcc") or shutil.which("clang")
        self.assertIsNotNone(cc, "no C compiler available")
        with tempfile.TemporaryDirectory() as directory:
            binary = pathlib.Path(directory) / "experiment_test"
            compile_proc = subprocess.run(
                [cc, "-std=c99", "-Wall", "-Wextra",
                 "-o", str(binary),
                 str(NATIVE / "experiment.c"),
                 str(NATIVE / "experiment_test.c")],
                capture_output=True, text=True, check=False,
            )
            self.assertEqual(compile_proc.returncode, 0,
                             f"C compile failed:\n{compile_proc.stderr}")
            self.assertEqual(compile_proc.stderr, "",
                             f"C compile warnings:\n{compile_proc.stderr}")
            run = subprocess.run([str(binary)], capture_output=True,
                                 text=True, check=False)
            self.assertEqual(run.returncode, 0,
                             f"C self-check failed:\n{run.stdout}")
            self.assertIn("all checks passed", run.stdout)


if __name__ == "__main__":
    unittest.main()
