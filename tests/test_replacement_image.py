import json
import pathlib
import subprocess
import tempfile
import unittest


ROOT = pathlib.Path(__file__).resolve().parents[1]


class ReplacementImageTests(unittest.TestCase):
    def test_adsp_nop_image_has_fixed_layout(self):
        with tempfile.TemporaryDirectory() as directory:
            output = pathlib.Path(directory)
            subprocess.run([
                str(ROOT / "tools/build-replacement-image"),
                "--processor", "adsp2100", "--fixture", "nop",
                "--output", str(output),
            ], check=True, capture_output=True, text=True)
            image = json.loads((output / "image.json").read_text())
            self.assertEqual(image["origin"], "0x0")
            self.assertEqual(image["entry"], "0x4")
            self.assertEqual(image["bytes_hex"], "0000000000000000000000000000000000000000")
            self.assertEqual((output / image["binary"]).read_bytes(), b"\0" * 20)

    def test_adsp_reset_loop_has_independent_fixed_encoding(self):
        with tempfile.TemporaryDirectory() as directory:
            output = pathlib.Path(directory)
            subprocess.run([
                str(ROOT / "tools/build-replacement-image"),
                "--processor", "adsp2100", "--fixture", "reset-loop",
                "--output", str(output),
            ], check=True, capture_output=True, text=True)
            image = json.loads((output / "image.json").read_text())
            self.assertEqual(image["entry"], "0x4")
            self.assertEqual(image["bytes_hex"][-8:], "0018004f")
            self.assertEqual(image["length"], 20)

    def test_adsp_init_prefix_encodes_observed_calls(self):
        with tempfile.TemporaryDirectory() as directory:
            output = pathlib.Path(directory)
            subprocess.run([
                str(ROOT / "tools/build-replacement-image"),
                "--processor", "adsp2100", "--fixture", "init-prefix",
                "--output", str(output),
            ], check=True, capture_output=True, text=True)
            image = json.loads((output / "image.json").read_text())
            payload = (output / image["binary"]).read_bytes()
            self.assertEqual(image["entry"], "0x4")
            self.assertEqual(image["length"], (0x834 + 1) * 4)
            self.assertEqual(payload[0x4 * 4:0x7 * 4].hex(), "001c780f001c834f0018006f")
            self.assertEqual(payload[0x780 * 4:0x781 * 4].hex(), "000a000f")
            self.assertEqual(payload[0x834 * 4:0x835 * 4].hex(), "000a000f")

    def test_adsp_init_state_preserves_observed_setup_prefix(self):
        with tempfile.TemporaryDirectory() as directory:
            output = pathlib.Path(directory)
            subprocess.run([
                str(ROOT / "tools/build-replacement-image"),
                "--processor", "adsp2100", "--fixture", "init-state",
                "--output", str(output),
            ], check=True, capture_output=True, text=True)
            image = json.loads((output / "image.json").read_text())
            payload = (output / image["binary"]).read_bytes()
            self.assertEqual(image["length"], (0x846 + 1) * 4)
            self.assertEqual(payload[0x6 * 4:0x10 * 4].hex(),
                             "00340008003400090034000a0034000b00380008003800090038000a0038000b0034001500380017")
            self.assertEqual(payload[0x3f * 4:0x40 * 4].hex(), "0018043f")
            self.assertEqual(payload[0x4f * 4:0x50 * 4].hex(), "001804d1")
            self.assertEqual(payload[0x50 * 4:0x51 * 4].hex(), "0018050f")
            self.assertEqual(payload[0x780 * 4:0x781 * 4].hex(), "00380014")
            self.assertEqual(payload[0x7a1 * 4:0x7a2 * 4].hex(), "000a000f")
            self.assertEqual(payload[0x834 * 4:0x835 * 4].hex(), "00340014")
            self.assertEqual(payload[0x846 * 4:0x847 * 4].hex(), "000a000f")


if __name__ == "__main__":
    unittest.main()
