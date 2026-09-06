"""Public ROM-free tests for the residue classifier pipeline.

Covers tools/classify-temporal, classify-writers, classify-modes,
classify-adjacency, classify-shape, and classify-width with synthetic
snapshots, traces, masks, and residue reports — including one
end-to-end composition proving the annotate-in-place contract chains.
No ROMs, no MAME, no network required.
"""
import json
import pathlib
import subprocess
import tempfile
import unittest


ROOT = pathlib.Path(__file__).resolve().parents[1]
TOOLS = ROOT / "tools"


def snapshot(directory, name, values, base=0x100, width=8):
    path = pathlib.Path(directory) / name
    path.write_text(json.dumps({
        "schema": "stunrun-memory-snapshot/v1",
        "device": "d", "space": "s", "base": base,
        "count": len(values), "width": width, "values": values,
    }), encoding="utf-8")
    return path


def residue_doc(directory, name, ranges, tags=None):
    path = pathlib.Path(directory) / name
    path.write_text(json.dumps({
        "schema": "stunrun-masked-snapshot/v1",
        "mode": "mask-only", "snapshot": "s", "baseline": None,
        "mask": "m",
        "residue_ranges": [
            {"address": address, "length": length, "cells": length,
             "sample": [], "tags": dict(tags or {})}
            for address, length in ranges
        ],
        "tracked_ranges": [],
        "summary": {},
    }), encoding="utf-8")
    return path


def run_tool(tool, *args):
    return subprocess.run([str(TOOLS / tool), *map(str, args)],
                          capture_output=True, text=True, check=False)


class TemporalTests(unittest.TestCase):
    def test_behaviors(self):
        with tempfile.TemporaryDirectory() as directory:
            shots = [
                snapshot(directory, f"s{i}.json", values)
                for i, values in enumerate([
                    [1, 10, 5, 0, 7, 0],
                    [1, 11, 9, 0, 7, 4],
                    [1, 12, 5, 9, 7, 0],
                ])
            ]
            res = residue_doc(directory, "res.json", [
                (0x100, 1), (0x101, 1), (0x102, 1),
                (0x103, 1), (0x104, 1), (0x105, 1),
            ])
            out = pathlib.Path(directory) / "out"
            run = run_tool("classify-temporal", "--residue", res,
                           "--snapshots", *shots, "--output", out)
            self.assertEqual(run.returncode, 0, run.stderr)
            report = json.loads((out / "residue.json").read_text())
            got = [r["tags"]["temporal"]["behavior"]
                   for r in report["residue_ranges"]]
            self.assertEqual(got, ["constant", "monotonic-up",
                                   "oscillating", "write-once",
                                   "constant", "transient"])
            self.assertIn("temporal", report["classifiers"])

    def test_needs_two_snapshots(self):
        with tempfile.TemporaryDirectory() as directory:
            shot = snapshot(directory, "s.json", [1])
            res = residue_doc(directory, "res.json", [(0x100, 1)])
            out = pathlib.Path(directory) / "out"
            run = run_tool("classify-temporal", "--residue", res,
                           "--snapshots", shot, "--output", out)
            self.assertNotEqual(run.returncode, 0)

    def test_geometry_mismatch_rejected(self):
        with tempfile.TemporaryDirectory() as directory:
            first = snapshot(directory, "a.json", [1, 2])
            second = snapshot(directory, "b.json", [1, 2, 3])
            res = residue_doc(directory, "res.json", [(0x100, 1)])
            out = pathlib.Path(directory) / "out"
            run = run_tool("classify-temporal", "--residue", res,
                           "--snapshots", first, second, "--output", out)
            self.assertNotEqual(run.returncode, 0)


class WriterTests(unittest.TestCase):
    def trace(self, directory, events):
        path = pathlib.Path(directory) / "trace.json"
        path.write_text(json.dumps({
            "schema": "stunrun-ram-write-trace-result/v1",
            "events": events,
        }), encoding="utf-8")
        return path

    def test_ownership(self):
        with tempfile.TemporaryDirectory() as directory:
            res = residue_doc(directory, "res.json", [
                (0x100, 2), (0x110, 2), (0x120, 1),
            ])
            trace = self.trace(directory, [
                {"frame": 1, "pc": 0x1000, "address": 0x100,
                 "data": 1, "mask": 0xFF},
                {"frame": 2, "pc": 0x1000, "address": 0x101,
                 "data": 2, "mask": 0xFF},
                {"frame": 3, "pc": 0x1000, "address": 0x110,
                 "data": 3, "mask": 0xFF},
                {"frame": 4, "pc": 0x2000, "address": 0x111,
                 "data": 4, "mask": 0xFF},
            ])
            out = pathlib.Path(directory) / "out"
            run = run_tool("classify-writers", "--residue", res,
                           "--trace", trace, "--output", out)
            self.assertEqual(run.returncode, 0, run.stderr)
            report = json.loads((out / "residue.json").read_text())
            got = [(r["tags"]["writers"]["ownership"],
                    r["tags"]["writers"]["writer_pcs"])
                   for r in report["residue_ranges"]]
            self.assertEqual(got, [
                ("single-writer", [0x1000]),
                ("multi-writer", [0x1000, 0x2000]),
                ("silent", []),
            ])

    def test_bad_trace_rejected(self):
        with tempfile.TemporaryDirectory() as directory:
            res = residue_doc(directory, "res.json", [(0x100, 1)])
            bad = pathlib.Path(directory) / "trace.json"
            bad.write_text(json.dumps({"schema": "nope"}), encoding="utf-8")
            out = pathlib.Path(directory) / "out"
            run = run_tool("classify-writers", "--residue", res,
                           "--trace", bad, "--output", out)
            self.assertNotEqual(run.returncode, 0)


class ModeTests(unittest.TestCase):
    def test_kinds(self):
        with tempfile.TemporaryDirectory() as directory:
            first = residue_doc(directory, "a.json", [
                (0x100, 4), (0x200, 4), (0x300, 4),
            ])
            second = residue_doc(directory, "b.json", [
                (0x100, 4), (0x300, 4),
            ])
            out = pathlib.Path(directory) / "out"
            run = run_tool("classify-modes", "--residues", first, second,
                           "--labels", "drive", "weapon",
                           "--output", out)
            self.assertEqual(run.returncode, 0, run.stderr)
            report = json.loads((out / "residue.json").read_text())
            got = [(r["tags"]["modes"]["kind"],
                    r["tags"]["modes"]["modes"])
                   for r in report["residue_ranges"]]
            self.assertEqual(got, [
                ("shared", ["drive", "weapon"]),
                ("primary-only", ["drive"]),
                ("shared", ["drive", "weapon"]),
            ])

    def test_label_mismatch_rejected(self):
        with tempfile.TemporaryDirectory() as directory:
            first = residue_doc(directory, "a.json", [(0x100, 1)])
            out = pathlib.Path(directory) / "out"
            run = run_tool("classify-modes", "--residues", first, first,
                           "--labels", "only-one", "--output", out)
            self.assertNotEqual(run.returncode, 0)


class AdjacencyTests(unittest.TestCase):
    def mask(self, directory, regions):
        path = pathlib.Path(directory) / "mask.json"
        path.write_text(json.dumps({
            "schema": "stunrun-known-regions/v1",
            "device": "d", "space": "s", "regions": regions,
        }), encoding="utf-8")
        return path

    def test_nearest(self):
        with tempfile.TemporaryDirectory() as directory:
            res = residue_doc(directory, "res.json", [
                (0x110, 4), (0x500, 2),
            ])
            mask = self.mask(directory, [
                {"label": "score", "base": "0x100", "last": "0x103",
                 "category": "explained", "provenance": "t",
                 "confidence": "t"},
                {"label": "other", "base": "0x200", "last": "0x20F",
                 "category": "explained", "provenance": "t",
                 "confidence": "t"},
            ])
            out = pathlib.Path(directory) / "out"
            run = run_tool("classify-adjacency", "--residue", res,
                           "--mask", mask, "--output", out)
            self.assertEqual(run.returncode, 0, run.stderr)
            report = json.loads((out / "residue.json").read_text())
            first = report["residue_ranges"][0]["tags"]["adjacency"]
            # 0x110 is 12 bytes past the 0x100-0x103 region.
            self.assertEqual(first, {"nearest": "score", "gap_bytes": 12})
            second = report["residue_ranges"][1]["tags"]["adjacency"]
            self.assertEqual(second["nearest"], "other")

    def test_adjacent_range_has_zero_gap(self):
        with tempfile.TemporaryDirectory() as directory:
            res = residue_doc(directory, "res.json", [(0x104, 2)])
            mask = self.mask(directory, [
                {"label": "score", "base": "0x100", "last": "0x103",
                 "category": "explained", "provenance": "t",
                 "confidence": "t"},
            ])
            out = pathlib.Path(directory) / "out"
            run = run_tool("classify-adjacency", "--residue", res,
                           "--mask", mask, "--output", out)
            self.assertEqual(run.returncode, 0, run.stderr)
            report = json.loads((out / "residue.json").read_text())
            self.assertEqual(
                report["residue_ranges"][0]["tags"]["adjacency"],
                {"nearest": "score", "gap_bytes": 0})

    def test_empty_mask_has_no_neighbor(self):
        with tempfile.TemporaryDirectory() as directory:
            res = residue_doc(directory, "res.json", [(0x100, 1)])
            mask = self.mask(directory, [])
            out = pathlib.Path(directory) / "out"
            run = run_tool("classify-adjacency", "--residue", res,
                           "--mask", mask, "--output", out)
            self.assertEqual(run.returncode, 0, run.stderr)
            report = json.loads((out / "residue.json").read_text())
            self.assertEqual(
                report["residue_ranges"][0]["tags"]["adjacency"]["nearest"],
                None)


class ShapeTests(unittest.TestCase):
    def test_shapes(self):
        with tempfile.TemporaryDirectory() as directory:
            shot = snapshot(directory, "s.json", [
                ord("H"), ord("i"), 0, 0,       # ascii-ish
                0, 0, 0, 9,                     # sparse
                0x10, 0x20, 0x10, 0x20,          # periodic
                0xDE, 0xAD, 0xBE, 0xEF,          # dense
            ], base=0x100)
            res = residue_doc(directory, "res.json", [
                (0x100, 4), (0x104, 4), (0x108, 4), (0x10C, 4),
            ])
            out = pathlib.Path(directory) / "out"
            run = run_tool("classify-shape", "--residue", res,
                           "--snapshot", shot, "--output", out)
            self.assertEqual(run.returncode, 0, run.stderr)
            report = json.loads((out / "residue.json").read_text())
            got = [r["tags"]["shape"]["shape"]
                   for r in report["residue_ranges"]]
            # "Hi\0\0" is only 2/4 printable (below the ascii bar) and
            # 2/4 nonzero (above the sparse bar): dense.
            self.assertEqual(got, ["dense", "sparse", "periodic", "dense"])
            periodic = report["residue_ranges"][2]["tags"]["shape"]
            self.assertEqual(periodic["period"], 2)

    def test_ascii_shape(self):
        with tempfile.TemporaryDirectory() as directory:
            shot = snapshot(directory, "s.json",
                            [ord(c) for c in "SCORE123"], base=0x100)
            res = residue_doc(directory, "res.json", [(0x100, 8)])
            out = pathlib.Path(directory) / "out"
            run = run_tool("classify-shape", "--residue", res,
                           "--snapshot", shot, "--output", out)
            self.assertEqual(run.returncode, 0, run.stderr)
            report = json.loads((out / "residue.json").read_text())
            self.assertEqual(
                report["residue_ranges"][0]["tags"]["shape"]["shape"],
                "ascii")


class WidthTests(unittest.TestCase):
    def pair(self, directory, before_values, after_values):
        before = snapshot(directory, "before.json", before_values)
        after = snapshot(directory, "after.json", after_values)
        return before, after

    def test_suggested_widths(self):
        with tempfile.TemporaryDirectory() as directory:
            before, after = self.pair(
                directory,
                [0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0],
                [9, 9, 9, 9, 0, 0, 7, 7, 1, 1, 1, 1, 0, 0, 5, 0])
            res = residue_doc(directory, "res.json", [
                (0x100, 4), (0x106, 2), (0x10E, 1),
            ])
            out = pathlib.Path(directory) / "out"
            run = run_tool("classify-width", "--residue", res,
                           "--before", before, "--after", after,
                           "--output", out)
            self.assertEqual(run.returncode, 0, run.stderr)
            report = json.loads((out / "residue.json").read_text())
            got = [r["tags"]["width"]["suggested_width"]
                   for r in report["residue_ranges"]]
            self.assertEqual(got, [4, 2, 1])

    def test_incompatible_pair_rejected(self):
        with tempfile.TemporaryDirectory() as directory:
            before = snapshot(directory, "before.json", [0, 0])
            after = snapshot(directory, "after.json", [0, 0, 0])
            res = residue_doc(directory, "res.json", [(0x100, 1)])
            out = pathlib.Path(directory) / "out"
            run = run_tool("classify-width", "--residue", res,
                           "--before", before, "--after", after,
                           "--output", out)
            self.assertNotEqual(run.returncode, 0)

    def test_non8_width_rejected(self):
        with tempfile.TemporaryDirectory() as directory:
            before = snapshot(directory, "before.json", [0], width=16)
            after = snapshot(directory, "after.json", [0], width=16)
            res = residue_doc(directory, "res.json", [(0x100, 2)])
            out = pathlib.Path(directory) / "out"
            run = run_tool("classify-width", "--residue", res,
                           "--before", before, "--after", after,
                           "--output", out)
            self.assertNotEqual(run.returncode, 0)


class CompositionTests(unittest.TestCase):
    def test_stages_chain_annotations(self):
        with tempfile.TemporaryDirectory() as directory:
            work = pathlib.Path(directory)
            shots = [
                snapshot(directory, f"s{i}.json", values)
                for i, values in enumerate([[0, 5], [0, 6], [0, 7]])
            ]
            res = residue_doc(directory, "res.json", [(0x101, 1)])
            trace = work / "trace.json"
            trace.write_text(json.dumps({
                "schema": "stunrun-ram-write-trace-result/v1",
                "events": [{"frame": 1, "pc": 0x3000, "address": 0x101,
                            "data": 7, "mask": 0xFF}],
            }), encoding="utf-8")
            mask = work / "mask.json"
            mask.write_text(json.dumps({
                "schema": "stunrun-known-regions/v1",
                "device": "d", "space": "s", "regions": [],
            }), encoding="utf-8")

            stage1 = work / "st1"
            run = run_tool("classify-temporal", "--residue", res,
                           "--snapshots", *shots, "--output", stage1)
            self.assertEqual(run.returncode, 0, run.stderr)
            stage2 = work / "st2"
            run = run_tool("classify-writers", "--residue",
                           stage1 / "residue.json", "--trace", trace,
                           "--output", stage2)
            self.assertEqual(run.returncode, 0, run.stderr)
            stage3 = work / "st3"
            run = run_tool("classify-adjacency", "--residue",
                           stage2 / "residue.json", "--mask", mask,
                           "--output", stage3)
            self.assertEqual(run.returncode, 0, run.stderr)
            stage4 = work / "st4"
            run = run_tool("classify-shape", "--residue",
                           stage3 / "residue.json", "--snapshot", shots[2],
                           "--output", stage4)
            self.assertEqual(run.returncode, 0, run.stderr)

            final = json.loads((stage4 / "residue.json").read_text())
            tags = final["residue_ranges"][0]["tags"]
            self.assertEqual(tags["temporal"]["behavior"], "monotonic-up")
            self.assertEqual(tags["writers"]["ownership"], "single-writer")
            self.assertEqual(tags["writers"]["writer_pcs"], [0x3000])
            self.assertEqual(tags["adjacency"]["nearest"], None)
            self.assertEqual(tags["shape"]["shape"], "tiny")
            self.assertEqual(final["classifiers"],
                             ["temporal", "writers", "adjacency", "shape"])


if __name__ == "__main__":
    unittest.main()
