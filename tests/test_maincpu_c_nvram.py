"""Public ROM-free tests for the Agent 2 C port of the NVRAM scores slice.

Compiles reproduction/maincpu/nvram_scores.c with the system C compiler
and runs its self-check (10-entry golden table decode). No ROMs, no MAME,
no network required.
"""
import pathlib
import shutil
import subprocess
import tempfile
import unittest


ROOT = pathlib.Path(__file__).resolve().parents[1]
SRC = ROOT / "reproduction" / "maincpu"


class NvramScoresCTests(unittest.TestCase):
    def test_c_self_check_passes(self):
        cc = shutil.which("cc") or shutil.which("gcc") or shutil.which("clang")
        self.assertIsNotNone(cc, "no C compiler available")
        with tempfile.TemporaryDirectory() as directory:
            binary = pathlib.Path(directory) / "nvram_scores_test"
            compile_proc = subprocess.run(
                [cc, "-std=c99", "-Wall", "-Wextra", "-o", str(binary),
                 str(SRC / "nvram_scores.c"),
                 str(SRC / "nvram_scores_test.c")],
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
