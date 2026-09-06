"""Public ROM-free tests for the deterministic native shell.

Compiles native/shell.c with the system C compiler and checks:
- compiled-in mode passes and is byte-deterministic across runs;
- file mode replays the canonical coin_start experiment (press/release
  dispatch at exact frames, anchors, checkpoint);
- file mode handles set actions and short terminals (contracts skip
  cleanly instead of failing on unestablished ground);
- missing files and bounded_observation terminals fail loudly (exit 2),
  never as silent passes.
No ROMs, no MAME, no network required.
"""
import json
import pathlib
import shutil
import subprocess
import tempfile
import unittest


ROOT = pathlib.Path(__file__).resolve().parents[1]
ADSP = ROOT / "reproduction" / "adsp"
SOUND = ROOT / "reproduction" / "sound"

SOURCES = [
    str(ROOT / "native" / "shell.c"),
    str(ROOT / "native" / "experiment.c"),
    str(ADSP / "adsp_init_image.c"),
    str(ADSP / "adsp_control_seq.c"),
    str(ADSP / "adsp_upload_stream.c"),
    str(SOUND / "jsa_latch.c"),
]


class NativeShellTests(unittest.TestCase):
    def compile_shell(self, directory):
        cc = shutil.which("cc") or shutil.which("gcc") or shutil.which("clang")
        self.assertIsNotNone(cc, "no C compiler available")
        binary = pathlib.Path(directory) / "native-shell"
        compile_proc = subprocess.run(
            [cc, "-std=c99", "-Wall", "-Wextra",
             "-I", str(ADSP), "-I", str(SOUND),
             "-o", str(binary), *SOURCES],
            capture_output=True, text=True, check=False,
        )
        self.assertEqual(compile_proc.returncode, 0,
                         f"C compile failed:\n{compile_proc.stderr}")
        self.assertEqual(compile_proc.stderr, "",
                         f"C compile warnings:\n{compile_proc.stderr}")
        return binary

    def test_shell_passes_and_is_deterministic(self):
        with tempfile.TemporaryDirectory() as directory:
            binary = self.compile_shell(directory)
            first = subprocess.run([str(binary)], capture_output=True,
                                   text=True, check=False)
            second = subprocess.run([str(binary)], capture_output=True,
                                    text=True, check=False)
            self.assertEqual(first.returncode, 0,
                             f"shell failed:\n{first.stdout}")
            self.assertEqual(second.returncode, 0,
                             f"shell rerun failed:\n{second.stdout}")
            self.assertEqual(first.stdout, second.stdout,
                             "shell output is not deterministic")
            self.assertIn("RESULT PASS", first.stdout)
            self.assertIn("checkpoint frames=600", first.stdout)

    def test_shell_replays_canonical_coin_start(self):
        with tempfile.TemporaryDirectory() as directory:
            binary = self.compile_shell(directory)
            run = subprocess.run(
                [str(binary),
                 str(ROOT / "experiments" / "stunrun" / "coin_start.json")],
                capture_output=True, text=True, check=False)
            self.assertEqual(run.returncode, 0,
                             f"shell failed:\n{run.stdout}\n{run.stderr}")
            self.assertIn("experiment id=coin_start", run.stdout)
            self.assertIn(
                "input frame=120 port=:mainpcb:IN0 field=Coin 1 "
                "action=press", run.stdout)
            self.assertIn(
                "input frame=302 port=:mainpcb:a80000 "
                "field=1 Player Start action=release", run.stdout)
            self.assertIn("control-tally=match", run.stdout)
            self.assertIn("RESULT PASS", run.stdout)
            rerun = subprocess.run(
                [str(binary),
                 str(ROOT / "experiments" / "stunrun" / "coin_start.json")],
                capture_output=True, text=True, check=False)
            self.assertEqual(run.stdout, rerun.stdout,
                             "file-mode output is not deterministic")

    def test_shell_handles_set_actions_and_short_terminal(self):
        fixture = {
            "schema": "arcade-experiment/v1",
            "id": "shell-smoke",
            "start": "power_on",
            "events": [
                {"frame": 5, "port": ":mainpcb:IN0", "field": "Coin 1",
                 "action": "press", "label": "c"},
                {"frame": 7, "port": ":mainpcb:IN0", "field": "Coin 1",
                 "action": "release"},
                {"frame": 9, "port": ":mainpcb:8BADC.0",
                 "field": "AD Stick X", "action": "set", "value": 220},
            ],
            "expect": {"kind": "frame", "frame": 30},
        }
        with tempfile.TemporaryDirectory() as directory:
            binary = self.compile_shell(directory)
            path = pathlib.Path(directory) / "events.json"
            path.write_text(json.dumps(fixture), encoding="utf-8")
            run = subprocess.run([str(binary), str(path)],
                                 capture_output=True, text=True, check=False)
            self.assertEqual(run.returncode, 0,
                             f"shell failed:\n{run.stdout}\n{run.stderr}")
            self.assertIn("action=set value=220", run.stdout)
            self.assertIn("full-contract=skipped terminal!=600", run.stdout)
            self.assertIn("upload=empty install_ready=0", run.stdout)
            self.assertIn("RESULT PASS", run.stdout)

    def test_shell_refuses_unrunnable_inputs_loudly(self):
        with tempfile.TemporaryDirectory() as directory:
            binary = self.compile_shell(directory)
            missing = subprocess.run(
                [str(binary), str(pathlib.Path(directory) / "absent.json")],
                capture_output=True, text=True, check=False)
            self.assertEqual(missing.returncode, 2)
            self.assertIn("parse failed", missing.stderr)
            obs = pathlib.Path(directory) / "obs.json"
            obs.write_text(json.dumps({
                "schema": "arcade-experiment/v1",
                "id": "watch",
                "start": "power_on",
                "events": [],
                "expect": {"kind": "bounded_observation",
                           "terminal": "title"},
            }), encoding="utf-8")
            refused = subprocess.run([str(binary), str(obs)],
                                     capture_output=True, text=True,
                                     check=False)
            self.assertEqual(refused.returncode, 2)
            self.assertIn("unsupported terminal kind", refused.stderr)
            usage = subprocess.run(
                [str(binary), str(obs), "extra-arg"],
                capture_output=True, text=True, check=False)
            self.assertEqual(usage.returncode, 2)
            self.assertIn("usage:", usage.stderr)

    def test_shell_reports_events_beyond_terminal(self):
        with tempfile.TemporaryDirectory() as directory:
            binary = self.compile_shell(directory)
            path = pathlib.Path(directory) / "late.json"
            path.write_text(json.dumps({
                "schema": "arcade-experiment/v1",
                "id": "late-event",
                "start": "power_on",
                "events": [
                    {"frame": 5, "port": ":mainpcb:IN0",
                     "field": "Coin 1", "action": "press"},
                    {"frame": 50, "port": ":mainpcb:IN0",
                     "field": "Coin 1", "action": "release"},
                ],
                "expect": {"kind": "frame", "frame": 30},
            }), encoding="utf-8")
            run = subprocess.run([str(binary), str(path)],
                                 capture_output=True, text=True, check=False)
            self.assertEqual(run.returncode, 0,
                             f"shell failed:\n{run.stdout}\n{run.stderr}")
            self.assertIn("events=1 pending=1", run.stdout)
            self.assertIn("RESULT PASS", run.stdout)


if __name__ == "__main__":
    unittest.main()
