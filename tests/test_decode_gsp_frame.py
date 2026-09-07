import json
import pathlib
import subprocess
import tempfile
import unittest


ROOT = pathlib.Path(__file__).resolve().parents[1]


class DecodeGspFrameTests(unittest.TestCase):
    def test_decodes_literal_multisync_formula(self):
        with tempfile.TemporaryDirectory() as temp:
            temp = pathlib.Path(temp)
            common = {
                "schema": "stunrun-memory-snapshot/v1",
                "device": ":mainpcb:gsp", "space": "program",
                "base": 0, "count": 4, "width": 16,
            }
            (temp / "vram.json").write_text(json.dumps({
                **common, "values": [0x1200, 0x0034, 0x5600, 0x0078],
            }), encoding="utf-8")
            (temp / "lo.json").write_text(json.dumps({
                **common, "values": [0x0000] * 256,
            }), encoding="utf-8")
            (temp / "hi.json").write_text(json.dumps({
                **common, "values": [0x0000] * 256,
            }), encoding="utf-8")
            state = {
                "schema": "stunrun-gsp-state-snapshot-result/v1",
                "snapshots": [{"frame": 1, "display": {
                    "dpyadr": 0, "dpytap": 0, "dpystart": 0,
                    "heblnk": 0, "hsblnk": 2, "dpyctl": 0x8400,
                }}],
            }
            (temp / "state.json").write_text(json.dumps(state), encoding="utf-8")
            output = temp / "frame.ppm"
            subprocess.run([
                str(ROOT / "tools/decode-gsp-frame"),
                str(temp / "vram.json"), str(temp / "lo.json"),
                str(temp / "hi.json"), str(temp / "state.json"),
                str(output), "--frame", "1", "--width", "2", "--lines", "1",
                "--start-dpyadr", "0", "--dpy-step", "0",
            ], cwd=ROOT, check=True, capture_output=True, text=True)
            data = output.read_bytes()
            self.assertEqual(data[:11], b"P6\n2 1\n255\n")
            self.assertEqual(data[11:], bytes([0, 0, 0, 0, 0, 0]))

    def test_visible_layout_reads_four_512_byte_lines_per_row(self):
        with tempfile.TemporaryDirectory() as temp:
            temp = pathlib.Path(temp)
            common = {
                "schema": "stunrun-memory-snapshot/v1",
                "device": ":mainpcb:gsp", "space": "program",
                "base": 0, "count": 4, "width": 16,
            }
            (temp / "vram.json").write_text(json.dumps({
                **common, "values": [0x0201, 0x0000, 0x0000, 0x0000],
            }), encoding="utf-8")
            lo = [0] * 256
            lo[1] = 0x1234
            lo[2] = 0x5678
            (temp / "lo.json").write_text(json.dumps({**common, "values": lo}), encoding="utf-8")
            (temp / "hi.json").write_text(json.dumps({**common, "values": [0] * 256}), encoding="utf-8")
            state = {
                "schema": "stunrun-gsp-state-snapshot-result/v1",
                "snapshots": [{"frame": 1, "display": {
                    "dpyadr": 0, "dpytap": 0, "dpystart": 0,
                    "heblnk": 0, "hsblnk": 2, "dpyctl": 0,
                }}],
            }
            (temp / "state.json").write_text(json.dumps(state), encoding="utf-8")
            output = temp / "frame.ppm"
            subprocess.run([
                str(ROOT / "tools/decode-gsp-frame"),
                str(temp / "vram.json"), str(temp / "lo.json"),
                str(temp / "hi.json"), str(temp / "state.json"),
                str(output), "--frame", "1", "--width", "2", "--lines", "1",
                "--visible-layout",
            ], cwd=ROOT, check=True, capture_output=True, text=True)
            data = output.read_bytes()
            self.assertEqual(data[11:], bytes([0x12, 0x34, 0, 0x56, 0x78, 0]))


if __name__ == "__main__":
    unittest.main()
