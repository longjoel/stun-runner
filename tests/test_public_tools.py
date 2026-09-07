import json
import pathlib
import subprocess
import tempfile
import unittest


ROOT = pathlib.Path(__file__).resolve().parents[1]


class PublicToolTests(unittest.TestCase):
    def run_tool(self, name, *args):
        return subprocess.run(
            [str(ROOT / "tools" / name), *map(str, args)],
            cwd=ROOT,
            check=True,
            capture_output=True,
            text=True,
        )

    def test_manifest_parser_without_roms(self):
        with tempfile.TemporaryDirectory() as temp:
            temp = pathlib.Path(temp)
            xml = temp / "sample.xml"
            output = temp / "manifest.json"
            xml.write_text(
                '<mame><machine name="stunrun" sourcefile="atari/harddriv.cpp">'
                '<description>synthetic</description>'
                '<rom name="a.bin" size="2" crc="12345678" sha1="abc" region="maincpu" offset="0"/>'
                '</machine></mame>',
                encoding="utf-8",
            )
            self.run_tool("mame-manifest", xml, "--mame-version", "test", "-o", output)
            manifest = json.loads(output.read_text())
            self.assertEqual(manifest["roms"][0]["filename"], "a.bin")
            self.assertEqual(manifest["roms"][0]["size"], 2)

    def test_machine_map_reconciliation_without_roms(self):
        with tempfile.TemporaryDirectory() as temp:
            inventory = pathlib.Path(temp) / "inventory.json"
            inventory.write_text(
                json.dumps({
                    "schema": "test",
                    "system": {"name": "stunrun", "description": "test"},
                    "mame": {"version": "test"},
                    "devices": [
                        {"tag": ":mainpcb:maincpu", "shortname": "m68010", "name": "main"},
                        {"tag": ":mainpcb:gsp", "shortname": "tms34010", "name": "gsp"},
                        {"tag": ":mainpcb:adsp", "shortname": "adsp2100", "name": "adsp"},
                        {"tag": ":mainpcb:jsa:cpu", "shortname": "m6502", "name": "sound"},
                    ],
                }),
                encoding="utf-8",
            )
            result = self.run_tool(
                "check-machine-map",
                ROOT / "analysis/driver-mining/stunrun.machine-map.yaml",
                inventory,
            )
            self.assertIn('"ok": true', result.stdout)

    def test_trace_to_lcov_without_mame(self):
        with tempfile.TemporaryDirectory() as temp:
            temp = pathlib.Path(temp)
            listing = temp / "sample.lst"
            trace = temp / "sample.trace"
            output = temp / "sample.info"
            listing.write_text("000000: nop\n000002: rts\n", encoding="utf-8")
            trace.write_text("000000: nop\n000000: nop\n", encoding="utf-8")
            self.run_tool(
                "trace-to-lcov", trace, listing,
                "--source", str(listing), "--test-name", "synthetic", "-o", output,
            )
            report = output.read_text()
            self.assertIn("TN:synthetic", report)
            self.assertIn("DA:1,2", report)
            self.assertIn("LH:1", report)

    def test_diff_memory_snapshot_reports_ranges(self):
        with tempfile.TemporaryDirectory() as temp:
            temp = pathlib.Path(temp)
            before = temp / "before.json"
            after = temp / "after.json"
            common = {
                "schema": "stunrun-memory-snapshot/v1",
                "device": ":mainpcb:adsp", "space": "data",
                "base": 0x10, "count": 6, "width": 16,
            }
            before.write_text(json.dumps({**common, "frame": 1,
                                          "values": [0, 1, 2, 3, 4, 5]}))
            after.write_text(json.dumps({**common, "frame": 2,
                                         "values": [0, 1, 9, 8, 4, 5]}))
            result = self.run_tool("diff-memory-snapshot", before, after)
            diff = json.loads(result.stdout)
            self.assertEqual(diff["changed_count"], 2)
            self.assertEqual(diff["changed_ranges"], [{"start": 0x12, "end": 0x13, "count": 2}])
            self.assertEqual(diff["changes"][0]["before"], 2)

    def test_memory_snapshot_exposes_address_stride(self):
        tool = (ROOT / "tools" / "mame-memory-snapshot").read_text()
        lua = (ROOT / "mame" / "lua" / "memory-snapshot.lua").read_text()
        self.assertIn("--address-stride", tool)
        self.assertIn("STUNRUN_MEMORY_STRIDE", tool)
        self.assertIn("offset * address_stride", lua)

    def test_gsp_state_snapshot_can_pair_screen_output(self):
        tool = (ROOT / "tools" / "mame-gsp-state-snapshot").read_text()
        lua = (ROOT / "mame" / "lua" / "gsp-state-snapshot.lua").read_text()
        self.assertIn("--screen", tool)
        self.assertIn("STUNRUN_GSP_STATE_SCREEN", lua)
        self.assertIn("screen:snapshot", lua)

    def test_analyze_memory_candidates_ranks_persistent_diffs(self):
        with tempfile.TemporaryDirectory() as temp:
            temp = pathlib.Path(temp)
            baseline, variant = temp / "baseline", temp / "variant"
            baseline.mkdir()
            variant.mkdir()
            for frame, base_values, variant_values in (
                (10, [0, 0, 1, 2], [0, 0, 1, 2]),
                (20, [0, 0, 1, 2], [4, 0, 1, 3]),
                (30, [0, 0, 1, 2], [4, 0, 1, 3]),
            ):
                for directory, values in ((baseline, base_values), (variant, variant_values)):
                    (directory / f"snapshot-{frame}.json").write_text(json.dumps({
                        "schema": "stunrun-memory-snapshot/v1",
                        "frame": frame, "base": 0x100, "count": len(values),
                        "width": 8, "device": ":mainpcb:maincpu",
                        "space": "program", "values": values,
                    }), encoding="utf-8")
            report = temp / "report.json"
            self.run_tool("analyze-memory-candidates", baseline, variant,
                          "--output", report, "--top", "10")
            result = json.loads(report.read_text())
            self.assertEqual(result["common_frames"], [10, 20, 30])
            self.assertEqual(result["candidates"][0]["address"], 0x100)
            self.assertEqual(result["candidates"][0]["address_hex"], "0x100")
            self.assertEqual(result["candidates"][0]["first_diff_frame"], 20)
            self.assertEqual(result["candidates"][0]["persistence_after_first"], 1.0)

    def test_analyze_memory_series_finds_monotonic_word(self):
        with tempfile.TemporaryDirectory() as temp:
            temp = pathlib.Path(temp)
            for frame, word in ((10, 100), (20, 90), (30, 80), (40, 80)):
                values = [word >> 8, word & 0xFF, 0xAA, 0x55]
                (temp / f"snapshot-{frame}.json").write_text(json.dumps({
                    "schema": "stunrun-memory-snapshot/v1", "frame": frame,
                    "base": 0x200, "count": 4, "width": 8,
                    "device": ":mainpcb:maincpu", "space": "program",
                    "values": values,
                }), encoding="utf-8")
            report = temp / "report.json"
            self.run_tool("analyze-memory-series", temp, "--output", report, "--top", "10")
            result = json.loads(report.read_text())
            candidate = next(item for item in result["candidates"]
                             if item["address"] == 0x200 and item["width_bytes"] == 2)
            self.assertEqual(candidate["direction"], "decreasing")
            self.assertEqual(candidate["monotonicity"], 1.0)

    def test_diff_ram_write_trace_reports_first_mismatch(self):
        with tempfile.TemporaryDirectory() as temp:
            temp = pathlib.Path(temp)
            before = temp / "before.json"
            after = temp / "after.json"
            common = {
                "schema": "stunrun-ram-write-trace-result/v1",
                "device": ":mainpcb:maincpu", "space": "program",
                "base": 0xFF9000, "end": 0xFF9FFF,
                "capture_start_frame": 680, "capture_end_frame": 705,
            }
            before.write_text(json.dumps({**common, "input": "none", "events": [
                {"frame": 680, "pc": 0x100, "address": 0xFF9000, "data": 1, "mask": 0xFF}
            ]}))
            after.write_text(json.dumps({**common, "input": "late_drive", "events": [
                {"frame": 680, "pc": 0x100, "address": 0xFF9000, "data": 2, "mask": 0xFF},
                {"frame": 681, "pc": 0x200, "address": 0xFF9006, "data": 1, "mask": 0xFF}
            ]}))
            result = self.run_tool("diff-ram-write-trace", before, after)
            diff = json.loads(result.stdout)
            self.assertEqual(diff["first_event_mismatch"]["index"], 0)
            self.assertEqual(diff["after_only_writer_pcs"], [0x200])


if __name__ == "__main__":
    unittest.main()
