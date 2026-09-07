"""Static checks for the saved-state instruction-trace wrapper."""

import pathlib
import subprocess
import unittest


ROOT = pathlib.Path(__file__).resolve().parents[1]
TOOL = ROOT / "tools" / "mame-trace"


class MameTraceWrapperTests(unittest.TestCase):
    def test_shell_syntax(self):
        result = subprocess.run(["bash", "-n", str(TOOL)],
                                capture_output=True, text=True)
        self.assertEqual(result.returncode, 0, result.stderr)

    def test_documents_saved_state_option(self):
        text = TOOL.read_text(encoding="utf-8")
        self.assertIn("--load-state STATE", text)
        self.assertIn("state_args=(-state_directory", text)
        self.assertIn("seconds_to_run=86400", text)

    def test_supports_saved_state_visible_forks(self):
        text = TOOL.read_text(encoding="utf-8")
        self.assertIn('"fork_hold_left"', text)
        self.assertIn('"fork_center"', text)


if __name__ == "__main__":
    unittest.main()
