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


if __name__ == "__main__":
    unittest.main()
