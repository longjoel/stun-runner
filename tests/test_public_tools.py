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


if __name__ == "__main__":
    unittest.main()
