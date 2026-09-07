import json
import pathlib
import subprocess
import tempfile
import unittest


ROOT = pathlib.Path(__file__).resolve().parents[1]


class ComparePpmTests(unittest.TestCase):
    def run_tool(self, left, right):
        return subprocess.run(
            [str(ROOT / "tools" / "compare-ppm"), str(left), str(right)],
            capture_output=True, text=True, check=False)

    @staticmethod
    def write(path, payload, width=2, height=1):
        path.write_bytes(f"P6\n{width} {height}\n255\n".encode() + payload)

    def test_equal_and_different_frames(self):
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            left, same, different = (root / name for name in
                                     ("left.ppm", "same.ppm", "different.ppm"))
            payload = bytes((0, 1, 2, 10, 20, 30))
            self.write(left, payload)
            self.write(same, payload)
            self.write(different, bytes((0, 1, 2, 10, 25, 30)))
            equal = self.run_tool(left, same)
            self.assertEqual(equal.returncode, 0)
            self.assertTrue(json.loads(equal.stdout)["equal"])
            mismatch = self.run_tool(left, different)
            self.assertEqual(mismatch.returncode, 1)
            report = json.loads(mismatch.stdout)
            self.assertFalse(report["equal"])
            self.assertEqual(report["changed_pixels"], 1)
            self.assertEqual(report["changed_channels"], 1)
            self.assertEqual(report["max_abs_error"], 5)

    def test_dimension_mismatch_is_reported(self):
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            left, right = root / "left.ppm", root / "right.ppm"
            self.write(left, bytes((0, 0, 0, 0, 0, 0)))
            self.write(right, bytes((0, 0, 0)), width=1, height=1)
            result = self.run_tool(left, right)
            self.assertEqual(result.returncode, 1)
            self.assertEqual(json.loads(result.stdout)["error"],
                             "dimension mismatch")


if __name__ == "__main__":
    unittest.main()
