"""ROM-free tests for play-capture comparison."""

import json
import pathlib
import subprocess
import tempfile
import unittest


ROOT = pathlib.Path(__file__).resolve().parents[1]
TOOL = ROOT / "tools" / "compare-play-capture"


class ComparePlayCaptureTests(unittest.TestCase):
    def make_capture(self, directory, suffix=""):
        directory.mkdir()
        (directory / "course-log.txt").write_text("frame course score_lo\n1 0 0\n", encoding="utf-8")
        (directory / "course-writes.txt").write_text("frame pc address data mask\n", encoding="utf-8")
        (directory / "snapshot-60.json").write_text(
            json.dumps({"frame": 60, "values": [1, suffix]}), encoding="utf-8")

    def run_tool(self, left, right, *args):
        return subprocess.run([str(TOOL), str(left), str(right), *args],
                              capture_output=True, text=True, cwd=ROOT)

    def test_identical_captures_pass(self):
        with tempfile.TemporaryDirectory() as temp:
            root = pathlib.Path(temp)
            left, right = root / "left", root / "right"
            self.make_capture(left)
            self.make_capture(right)
            proc = self.run_tool(left, right, "--require-snapshots")
            self.assertEqual(proc.returncode, 0, proc.stderr)
            self.assertTrue(json.loads(proc.stdout)["equal"])

    def test_snapshot_mismatch_fails(self):
        with tempfile.TemporaryDirectory() as temp:
            root = pathlib.Path(temp)
            left, right = root / "left", root / "right"
            self.make_capture(left)
            self.make_capture(right, 9)
            proc = self.run_tool(left, right)
            self.assertEqual(proc.returncode, 1)
            self.assertFalse(json.loads(proc.stdout)["equal"])

    def test_snapshot_set_mismatch_is_reported(self):
        with tempfile.TemporaryDirectory() as temp:
            root = pathlib.Path(temp)
            left, right = root / "left", root / "right"
            self.make_capture(left)
            self.make_capture(right)
            (right / "snapshot-120.json").write_text("{}", encoding="utf-8")
            proc = self.run_tool(left, right, "--require-snapshots")
            self.assertEqual(proc.returncode, 1)
            report = json.loads(proc.stdout)
            self.assertFalse(report["equal"])
            self.assertEqual(report["checks"][2]["right_only"], ["snapshot-120.json"])

    def test_replay_snapshot_superset_passes_by_default(self):
        with tempfile.TemporaryDirectory() as temp:
            root = pathlib.Path(temp)
            left, right = root / "left", root / "right"
            self.make_capture(left)
            self.make_capture(right)
            (right / "snapshot-120.json").write_text("{}", encoding="utf-8")
            proc = self.run_tool(left, right)
            self.assertEqual(proc.returncode, 0)
            report = json.loads(proc.stdout)
            self.assertTrue(report["equal"])
            self.assertFalse(report["checks"][2]["equal"])

    def test_missing_capture_file_fails(self):
        with tempfile.TemporaryDirectory() as temp:
            root = pathlib.Path(temp)
            left, right = root / "left", root / "right"
            self.make_capture(left)
            right.mkdir()
            proc = self.run_tool(left, right)
            self.assertEqual(proc.returncode, 1)
            self.assertIn("missing", proc.stdout)


if __name__ == "__main__":
    unittest.main()
