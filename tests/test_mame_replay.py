"""ROM-free contract tests for replay construction and state forks."""

import json
import pathlib
import runpy
import subprocess
import sys
import tempfile
import unittest


ROOT = pathlib.Path(__file__).resolve().parents[1]
REPLAY = runpy.run_path(str(ROOT / "tools/mame-replay"),
                        run_name="mame_replay_test")


def experiment(initial_events=None):
    value = {
        "schema": "arcade-experiment/v1",
        "id": "fork",
        "start": "saved_state",
        "events": [{
            "frame": 2, "port": ":mainpcb:a80000",
            "field": "P1 Button 1", "action": "press",
        }],
        "expect": {"kind": "frame", "frame": 30},
    }
    if initial_events is not None:
        value["initial_events"] = initial_events
    return value


class ReplayTests(unittest.TestCase):
    def test_state_fork_script_keeps_relative_events(self):
        state = pathlib.Path("/tmp/example-fork.sta")
        script = REPLAY["lua_script"](experiment())
        self.assertIn("local events =", script)
        self.assertIn("frame >= 30", script)
        self.assertIn("P1 Button 1", script)
        self.assertNotIn("machine:load(state_path)", script)

    def test_state_fork_rejects_pre_reset_events(self):
        with tempfile.TemporaryDirectory() as directory:
            directory = pathlib.Path(directory)
            experiment_path = directory / "experiment.json"
            state = pathlib.Path(directory) / "fork.sta"
            state.write_bytes(b"state")
            experiment_path.write_text(
                json.dumps(experiment([{"frame": 1, "action": "press"}])),
                encoding="utf-8")
            run = subprocess.run(
                [sys.executable, str(ROOT / "tools/mame-replay"),
                 str(experiment_path), "--rompath", str(directory),
                 "--output", str(directory / "out"), "--load-state", str(state)],
                capture_output=True, text=True, check=False)
            self.assertNotEqual(run.returncode, 0)
            self.assertIn("initial_events", run.stderr)

    def test_validate_preserves_non_decreasing_event_contract(self):
        REPLAY["validate"](experiment())
        invalid = experiment()
        invalid["events"] = [dict(invalid["events"][0], frame=3),
                              dict(invalid["events"][0], frame=2)]
        with self.assertRaises(ValueError):
            REPLAY["validate"](invalid)


if __name__ == "__main__":
    unittest.main()
