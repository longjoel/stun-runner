"""Public ROM-free tests for tools/mask-memory-snapshot.

Exercises classification (explained/tracked/residue), range coalescing,
both modes, width-aware addressing, and all error paths using synthetic
snapshots and masks. No ROMs, no MAME, no network required.
"""
import json
import pathlib
import subprocess
import tempfile
import unittest


ROOT = pathlib.Path(__file__).resolve().parents[1]
TOOL = ROOT / "tools" / "mask-memory-snapshot"


def write_snapshot(directory, name, base=256, count=16, width=8,
                   values=None, device="d", space="s"):
    if values is None:
        values = [0] * count
    path = pathlib.Path(directory) / name
    path.write_text(json.dumps({
        "schema": "stunrun-memory-snapshot/v1",
        "device": device, "space": space, "base": base, "count": count,
        "width": width, "values": values,
    }), encoding="utf-8")
    return path


def write_mask(directory, regions, device="d", space="s", name="mask.json"):
    path = pathlib.Path(directory) / name
    path.write_text(json.dumps({
        "schema": "stunrun-known-regions/v1",
        "device": device, "space": space, "regions": regions,
    }), encoding="utf-8")
    return path


def region(label, base, last, category):
    return {"label": label, "base": hex(base), "last": hex(last),
            "category": category, "provenance": "t", "confidence": "t"}


class MaskSnapshotTests(unittest.TestCase):
    def run_tool(self, *args):
        return subprocess.run([str(TOOL), *map(str, args)],
                              capture_output=True, text=True, check=False)

    def test_mask_only_classifies_and_coalesces(self):
        with tempfile.TemporaryDirectory() as directory:
            snap = write_snapshot(
                directory, "snap.json",
                values=[0, 0, 7, 8, 9, 10, 0, 0, 5, 0, 0, 0, 0, 0, 3, 0])
            mask = write_mask(directory, [
                region("known", 0x102, 0x105, "explained"),
                region("watched", 0x108, 0x108, "tracked"),
            ])
            out = pathlib.Path(directory) / "out"
            run = self.run_tool("--mask", mask, "--snapshot", snap,
                                "--output", out)
            self.assertEqual(run.returncode, 0, run.stderr)
            report = json.loads((out / "residue.json").read_text())
            self.assertEqual(report["schema"], "stunrun-masked-snapshot/v1")
            self.assertEqual(report["mode"], "mask-only")
            # 0x102-0x105 masked (4 cells), 0x108 tracked, 0x10E residue.
            residue = report["residue_ranges"]
            self.assertEqual(len(residue), 1)
            self.assertEqual(residue[0]["address"], 0x10E)
            self.assertEqual(residue[0]["length"], 1)
            self.assertEqual(report["tracked_ranges"][0]["address"], 0x108)
            self.assertEqual(report["summary"]["explained_cells"], 4)

    def test_consecutive_residue_merges_into_one_range(self):
        with tempfile.TemporaryDirectory() as directory:
            snap = write_snapshot(
                directory, "snap.json",
                values=[1, 2, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0])
            mask = write_mask(directory, [])
            out = pathlib.Path(directory) / "out"
            run = self.run_tool("--mask", mask, "--snapshot", snap,
                                "--output", out)
            self.assertEqual(run.returncode, 0, run.stderr)
            report = json.loads((out / "residue.json").read_text())
            self.assertEqual(len(report["residue_ranges"]), 1)
            self.assertEqual(report["residue_ranges"][0]["address"], 0x100)
            self.assertEqual(report["residue_ranges"][0]["length"], 3)
            self.assertEqual(report["residue_ranges"][0]["sample"],
                             [1, 2, 3])

    def test_diff_mode_masks_changes(self):
        with tempfile.TemporaryDirectory() as directory:
            before = write_snapshot(directory, "before.json")
            after = write_snapshot(
                directory, "after.json",
                values=[0, 0, 9, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0])
            mask = write_mask(directory, [region("k", 0x102, 0x102,
                                                 "explained")])
            out = pathlib.Path(directory) / "out"
            run = self.run_tool("--mask", mask, "--snapshot", after,
                                "--baseline", before, "--output", out)
            self.assertEqual(run.returncode, 0, run.stderr)
            report = json.loads((out / "residue.json").read_text())
            self.assertEqual(report["mode"], "diff-mask")
            self.assertEqual(len(report["residue_ranges"]), 1)
            self.assertEqual(report["residue_ranges"][0]["address"], 0x10E)

    def test_width16_addressing(self):
        with tempfile.TemporaryDirectory() as directory:
            snap = write_snapshot(directory, "snap.json", base=0x100,
                                  count=4, width=16,
                                  values=[0, 0x1234, 0, 0])
            mask = write_mask(directory, [region("k", 0x100, 0x103,
                                                 "explained")])
            out = pathlib.Path(directory) / "out"
            run = self.run_tool("--mask", mask, "--snapshot", snap,
                                "--output", out)
            self.assertEqual(run.returncode, 0, run.stderr)
            report = json.loads((out / "residue.json").read_text())
            # Cell 1 spans 0x102-0x103 and overlaps the mask.
            self.assertEqual(report["summary"]["residue_cells"], 0)

    def test_empty_residue_is_a_finding(self):
        with tempfile.TemporaryDirectory() as directory:
            snap = write_snapshot(directory, "snap.json")
            mask = write_mask(directory, [])
            out = pathlib.Path(directory) / "out"
            run = self.run_tool("--mask", mask, "--snapshot", snap,
                                "--output", out)
            self.assertEqual(run.returncode, 0, run.stderr)
            report = json.loads((out / "residue.json").read_text())
            self.assertEqual(report["residue_ranges"], [])
            self.assertIn("residue=0", run.stdout)

    def test_device_mismatch_is_rejected(self):
        with tempfile.TemporaryDirectory() as directory:
            snap = write_snapshot(directory, "snap.json", device="other")
            mask = write_mask(directory, [])
            out = pathlib.Path(directory) / "out"
            run = self.run_tool("--mask", mask, "--snapshot", snap,
                                "--output", out)
            self.assertNotEqual(run.returncode, 0)

    def test_bad_mask_schema_is_rejected(self):
        with tempfile.TemporaryDirectory() as directory:
            bad = pathlib.Path(directory) / "mask.json"
            bad.write_text(json.dumps({"schema": "nope"}), encoding="utf-8")
            snap = write_snapshot(directory, "snap.json")
            out = pathlib.Path(directory) / "out"
            run = self.run_tool("--mask", bad, "--snapshot", snap,
                                "--output", out)
            self.assertNotEqual(run.returncode, 0)

    def test_incompatible_baseline_is_rejected(self):
        with tempfile.TemporaryDirectory() as directory:
            before = write_snapshot(directory, "before.json", count=8)
            after = write_snapshot(directory, "after.json", count=16)
            mask = write_mask(directory, [])
            out = pathlib.Path(directory) / "out"
            run = self.run_tool("--mask", mask, "--snapshot", after,
                                "--baseline", before, "--output", out)
            self.assertNotEqual(run.returncode, 0)

    def test_real_manifest_parses(self):
        mask = json.loads(
            (ROOT / "analysis" / "known-regions.json").read_text(
                encoding="utf-8"))
        self.assertEqual(mask["schema"], "stunrun-known-regions/v1")
        explained = [r for r in mask["regions"]
                     if r["category"] == "explained"]
        tracked = [r for r in mask["regions"]
                   if r["category"] == "tracked"]
        self.assertGreater(len(explained), 5)
        self.assertGreaterEqual(len(tracked), 2)
        for entry in mask["regions"]:
            self.assertLessEqual(int(entry["base"], 0),
                                 int(entry["last"], 0))


if __name__ == "__main__":
    unittest.main()
