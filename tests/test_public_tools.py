import json
import pathlib
import struct
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

    def test_analyze_gsp_vram_write_trace_groups_writers(self):
        with tempfile.TemporaryDirectory() as temp:
            temp = pathlib.Path(temp)
            trace = temp / "writes.json"
            output = temp / "summary.json"
            trace.write_text(json.dumps({
                "schema": "stunrun-ram-write-trace-result/v1",
                "device": ":mainpcb:gsp", "space": "program",
                "base": 0x2000000, "end": 0x20000FF,
                "capture_start_frame": 10, "capture_end_frame": 12,
                "events": [
                    {"frame": 10, "pc": 0xFFF46590, "address": 0x2000000,
                     "data": 1, "mask": 0xFFFF},
                    {"frame": 10, "pc": 0xFFF46590, "address": 0x2000010,
                     "data": 2, "mask": 0xFFFF},
                    {"frame": 12, "pc": 0xFFF43030, "address": 0x2000040,
                     "data": 3, "mask": 0xFFFF},
                ],
            }), encoding="utf-8")
            self.run_tool("analyze-gsp-vram-write-trace", trace,
                          "--label", "0xFFF46590=PIXBLT", "-o", output)
            report = json.loads(output.read_text())
            self.assertEqual(report["schema"],
                             "stunrun-gsp-vram-write-summary/v1")
            self.assertEqual(report["writers"]["0xFFF46590"]["label"], "PIXBLT")
            self.assertEqual(report["writers"]["0xFFF46590"]["event_count"], 2)
            self.assertEqual(report["writers"]["0xFFF46590"]["address_ranges"], [
                {"start": 0x2000000, "end": 0x2000000, "count": 1},
                {"start": 0x2000010, "end": 0x2000010, "count": 1},
            ])

    def test_track_table_analyzer_snapshot_comparison_without_roms(self):
        with tempfile.TemporaryDirectory() as temp:
            temp = pathlib.Path(temp)
            rompath = temp / "roms"
            romdir = rompath / "stunrun"
            romdir.mkdir(parents=True)
            manifest = temp / "manifest.json"
            output = temp / "analysis.json"
            snapshot = temp / "snapshot.json"
            bases = [0x20, 0x320, 0x620, 0x920]
            region = bytearray(0x47406 + 23 * 4)
            for slot, base in enumerate(bases):
                for word in range(384):
                    value = 0x1000 + word
                    if 48 <= word <= 143:
                        value += slot << 8
                    if slot == 0 and word == 24:
                        value ^= 0x00FF
                    region[base + word * 2:base + word * 2 + 2] = struct.pack(
                        ">H", value)
            selector = [0x44630, 0x44930, 0x45230] + [0x44630] * 20
            for index, value in enumerate(selector):
                region[0x47406 + index * 4:0x4740A + index * 4] = struct.pack(
                    ">I", value)
            (romdir / "even.bin").write_bytes(bytes(region[0::2]))
            (romdir / "odd.bin").write_bytes(bytes(region[1::2]))
            manifest.write_text(json.dumps({
                "roms": [
                    {"filename": "even.bin", "region": "mainpcb:maincpu",
                     "size": len(region) // 2, "offset": 0},
                    {"filename": "odd.bin", "region": "mainpcb:maincpu",
                     "size": len(region) // 2, "offset": 1},
                ]
            }), encoding="utf-8")
            selected = bytes(region[0x620:0x920])
            selected = bytearray(selected)
            selected[-48:] = bytes((value ^ 0x55 for value in selected[-48:]))
            snapshot.write_text(json.dumps({
                "schema": "stunrun-memory-snapshot/v1",
                "base": 0xFF9584,
                "width": 8,
                "values": list(selected) * 2,
            }), encoding="utf-8")
            base_args = [argument for base in bases
                         for argument in ("--base", hex(base))]
            result = self.run_tool(
                "analyze-track-tables", rompath, "--manifest", manifest,
                *base_args, "--snapshot", snapshot, "--output", output,
            )
            self.assertEqual(result.returncode, 0)
            report = json.loads(output.read_text())
            comparison = report["snapshot_comparisons"][0]
            self.assertEqual(report["common_tail_sha256"],
                             "d54e3b376eb4d0fc96fd4af38c020e99c1e4b54d38d65edc4e97e2e72eeec18d")
            self.assertEqual(report["state_table_selector"]["entry_count"], 23)
            self.assertEqual(report["state_table_selector"]["state_to_table"][10],
                             {"state": 10, "rom_base": "0x44630"})
            self.assertEqual(comparison["best"]["slot"], "0x00620")
            self.assertEqual(comparison["best"]["different_word_ranges"], ["360-383"])
            self.assertEqual(comparison["settled_tail_sha256"],
                             "00c560922d5d536424b96f2d92e6cdda6055330024c3bd693e41cb1734a801a8")

    def test_native_visible_bridge_runner(self):
        with tempfile.TemporaryDirectory() as temp:
            temp = pathlib.Path(temp)
            expected = temp / "expected.ppm"
            actual = temp / "actual.ppm"
            vram = temp / "vram.bin"
            palette = temp / "palette.rgb"
            native = temp / "native"
            payload = bytes([0x12, 0x34, 0x56])
            expected.write_bytes(b"P6\n1 1\n255\n" + payload)
            vram.write_bytes(b"\x00\x00")
            palette.write_bytes(bytes(768))
            native.write_text(
                "#!/bin/sh\ncp '" + str(expected) + "' \"$STUNRUN_RENDER_PPM\"\n",
                encoding="utf-8")
            native.chmod(0o755)
            result = self.run_tool("run-native-visible-bridge", native, vram,
                                   palette, expected, "--actual", actual)
            report = json.loads(result.stdout)
            self.assertEqual(report["schema"],
                             "stunrun-native-visible-bridge-result/v1")
            self.assertEqual(report["comparison"]["changed_pixels"], 0)

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
        self.assertIn("fine_scroll", lua)

    def test_export_gsp_native_state_writes_little_endian_fixture(self):
        with tempfile.TemporaryDirectory() as temp:
            temp = pathlib.Path(temp)
            common = {
                "schema": "stunrun-memory-snapshot/v1",
                "device": ":mainpcb:gsp", "space": "program",
                "base": 0, "count": 4, "width": 16,
            }
            vram = temp / "vram.json"
            lo = temp / "lo.json"
            hi = temp / "hi.json"
            vram.write_text(json.dumps({**common, "values": [1, 0x2345, 3, 4]}), encoding="utf-8")
            lo.write_text(json.dumps({**common, "values": [0x1000] * 256}), encoding="utf-8")
            hi.write_text(json.dumps({**common, "values": [0x0020] * 256}), encoding="utf-8")
            vram_out = temp / "vram.bin"
            palette_out = temp / "palette.rgb"
            self.run_tool("export-gsp-native-state", vram, lo, hi,
                          "--vram-out", vram_out, "--palette-out", palette_out)
            self.assertEqual(vram_out.read_bytes(), b"".join(
                struct.pack("<H", value) for value in (1, 0x2345, 3, 4)))
            self.assertEqual(palette_out.read_bytes(), bytes([0x10, 0, 0x20]) * 256)

    def test_export_geometry_native_state_validates_twin_and_writes_words(self):
        with tempfile.TemporaryDirectory() as temp:
            temp = pathlib.Path(temp)
            values = list(range(256)) * 6
            snapshot = temp / "road.json"
            snapshot.write_text(json.dumps({
                "schema": "stunrun-memory-snapshot/v1",
                "device": ":mainpcb:maincpu", "space": "program",
                "base": 0xFF9584, "count": 0x600, "width": 8,
                "frame": 30, "values": values,
            }), encoding="utf-8")
            output = temp / "geometry.bin"
            result = self.run_tool("export-geometry-native-state", snapshot,
                                   "--output", output)
            metadata = json.loads(result.stdout)
            self.assertEqual(metadata["copies"], "match")
            self.assertEqual(metadata["bytes"], 0x300)
            self.assertEqual(output.stat().st_size, 0x300)
            self.assertEqual(output.read_bytes()[:4], b"\x00\x01\x02\x03")

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

    def test_ram_write_trace_exposes_early_tap_frame(self):
        wrapper = (ROOT / "tools" / "mame-ram-write-trace").read_text(
            encoding="utf-8")
        lua = (ROOT / "mame" / "lua" / "ram-write-trace.lua").read_text(
            encoding="utf-8")
        self.assertIn('"--tap-frame"', wrapper)
        self.assertIn("STUNRUN_RAM_TRACE_TAP_FRAME", wrapper)
        self.assertIn("STUNRUN_RAM_TRACE_TAP_FRAME", lua)
        self.assertIn("frame == tap_frame", lua)


if __name__ == "__main__":
    unittest.main()
