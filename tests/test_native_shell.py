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
import os
import pathlib
import shutil
import struct
import subprocess
import tempfile
import unittest


ROOT = pathlib.Path(__file__).resolve().parents[1]
ADSP = ROOT / "reproduction" / "adsp"
MAINCPU = ROOT / "reproduction" / "maincpu"
SOUND = ROOT / "reproduction" / "sound"

SOURCES = [
    str(ROOT / "native" / "shell.c"),
    str(ROOT / "native" / "render.c"),
    str(ROOT / "native" / "gsp_video.c"),
    str(ROOT / "native" / "checkpoint.c"),
    str(ROOT / "native" / "experiment.c"),
    str(ADSP / "adsp_init_image.c"),
    str(ADSP / "adsp_control_seq.c"),
    str(ADSP / "adsp_upload_stream.c"),
    str(ROOT / "reproduction" / "maincpu" / "fifo_block.c"),
    str(MAINCPU / "geom_upload.c"),
    str(MAINCPU / "road_fifo.c"),
    str(ROOT / "reproduction" / "gsp" / "text_cursor.c"),
    str(ROOT / "reproduction" / "gsp" / "text_record.c"),
    str(SOUND / "jsa_latch.c"),
]


class NativeShellTests(unittest.TestCase):
    def compile_shell(self, directory):
        cc = shutil.which("cc") or shutil.which("gcc") or shutil.which("clang")
        self.assertIsNotNone(cc, "no C compiler available")
        binary = pathlib.Path(directory) / "native-shell"
        compile_proc = subprocess.run(
            [cc, "-std=c99", "-Wall", "-Wextra",
             "-I", str(ADSP), "-I", str(MAINCPU),
             "-I", str(ROOT / "reproduction" / "gsp"), "-I", str(SOUND),
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
            self.assertIn("time-us=10000000", first.stdout)
            self.assertIn("input-hash=0x", first.stdout)
            self.assertIn("render frame=600 width=512 height=240 "
                          "hash=0x", first.stdout)
            self.assertIn('checkpoint-json={', first.stdout)
            self.assertIn('"description": "native-shell-transport-model"',
                          first.stdout)
            checkpoint_line = next(
                line for line in first.stdout.splitlines()
                if line.startswith("shell: checkpoint-json="))
            checkpoint = json.loads(
                checkpoint_line.split("=", 1)[1])
            self.assertEqual(checkpoint["schema"], "stunrun-checkpoint/v1")
            self.assertEqual(checkpoint["frame"], 600)
            self.assertTrue(checkpoint["selectors"]["adsp_program_loaded"])

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
            self.assertIn("input-state frame=9 active=1 hash=0x", run.stdout)
            self.assertIn("full-contract=skipped terminal!=600", run.stdout)
            self.assertIn("upload=empty install_ready=0", run.stdout)
            self.assertIn("RESULT PASS", run.stdout)

    def test_shell_writes_optional_ppm_frame(self):
        with tempfile.TemporaryDirectory() as directory:
            binary = self.compile_shell(directory)
            ppm = pathlib.Path(directory) / "frame.ppm"
            env = dict(os.environ)
            env["STUNRUN_RENDER_PPM"] = str(ppm)
            run = subprocess.run([str(binary)], env=env,
                                 capture_output=True, text=True,
                                 check=False)
            self.assertEqual(run.returncode, 0,
                             f"shell failed:\n{run.stdout}")
            self.assertIn("render-output", run.stdout)
            payload = ppm.read_bytes()
            header = b"P6\n512 240\n255\n"
            self.assertTrue(payload.startswith(header))
            self.assertEqual(len(payload), len(header) + 512 * 240 * 3)

    def test_shell_consumes_optional_gsp_video_state(self):
        with tempfile.TemporaryDirectory() as directory:
            directory = pathlib.Path(directory)
            binary = self.compile_shell(directory)
            vram = directory / "vram.bin"
            palette = directory / "palette.rgb"
            frame = directory / "gsp-frame.ppm"
            vram.write_bytes(b"".join(struct.pack("<H", value)
                                         for value in (0x0101, 0, 0, 0)))
            palette.write_bytes(bytes(256 * 3))
            env = dict(os.environ)
            env["STUNRUN_GSP_VRAM_BIN"] = str(vram)
            env["STUNRUN_GSP_PALETTE_BIN"] = str(palette)
            env["STUNRUN_RENDER_PPM"] = str(frame)
            run = subprocess.run([str(binary)], env=env,
                                 capture_output=True, text=True, check=False)
            self.assertEqual(run.returncode, 0,
                             f"shell failed:\n{run.stdout}")
            self.assertIn("mode=gsp-visible-state", run.stdout)
            self.assertTrue(frame.is_file())

    def test_shell_consumes_raw_gsp_palette_planes(self):
        with tempfile.TemporaryDirectory() as directory:
            directory = pathlib.Path(directory)
            binary = self.compile_shell(directory)
            vram = directory / "vram.bin"
            low = directory / "palette-low.bin"
            high = directory / "palette-high.bin"
            frame = directory / "gsp-raw-frame.ppm"
            vram.write_bytes(b"".join(struct.pack("<H", value)
                                         for value in (0x0707, 0, 0, 0)))
            low.write_bytes(b"".join(struct.pack("<H", 0x1234)
                                        for _ in range(256)))
            high.write_bytes(b"".join(struct.pack("<H", 0xABCD)
                                         for _ in range(256)))
            env = dict(os.environ)
            env["STUNRUN_GSP_VRAM_BIN"] = str(vram)
            env["STUNRUN_GSP_PALETTE_LOW_BIN"] = str(low)
            env["STUNRUN_GSP_PALETTE_HIGH_BIN"] = str(high)
            env["STUNRUN_RENDER_PPM"] = str(frame)
            run = subprocess.run([str(binary)], env=env,
                                 capture_output=True, text=True, check=False)
            self.assertEqual(run.returncode, 0,
                             f"shell failed:\n{run.stdout}\n{run.stderr}")
            self.assertIn("mode=gsp-visible-state", run.stdout)
            payload = frame.read_bytes()
            header = b"P6\n512 240\n255\n"
            self.assertEqual(payload[len(header):len(header) + 3],
                             bytes((0x12, 0x34, 0xCD)))

    def test_shell_consumes_gsp_text_cursor_fixture(self):
        with tempfile.TemporaryDirectory() as directory:
            directory = pathlib.Path(directory)
            binary = self.compile_shell(directory)
            table = [0] * (128 * 4)
            table[0x43 * 4:0x43 * 4 + 4] = [0x633E, 0x0303,
                                             0x6303, 0x003E]
            table[0x72 * 4:0x72 * 4 + 4] = [0x0000, 0x6E3E,
                                             0x0606, 0x0006]
            table_path = directory / "text-table.bin"
            words_path = directory / "text-words.bin"
            frame = directory / "text-frame.ppm"
            table_path.write_bytes(b"".join(struct.pack("<H", value)
                                               for value in table))
            words_path.write_bytes(struct.pack("<HH", 0x7243, 0x0000))
            env = dict(os.environ)
            env.update({
                "STUNRUN_GSP_TEXT_TABLE_BIN": str(table_path),
                "STUNRUN_GSP_TEXT_WORDS_BIN": str(words_path),
                "STUNRUN_GSP_TEXT_A0": "0xFFFEA810",
                "STUNRUN_GSP_TEXT_A1": "0x010800D4",
                "STUNRUN_GSP_TEXT_Y_BIAS": "0x28",
                "STUNRUN_RENDER_PPM": str(frame),
            })
            run = subprocess.run([str(binary)], env=env,
                                 capture_output=True, text=True, check=False)
            self.assertEqual(run.returncode, 0,
                             f"shell failed:\n{run.stdout}\n{run.stderr}")
            self.assertIn("mode=gsp-text-cursor-fixture", run.stdout)
            payload = frame.read_bytes()
            header = b"P6\n512 240\n255\n"
            self.assertEqual(payload[len(header) +
                                    ((224 * 512 + 213) * 3):
                                    len(header) + ((224 * 512 + 213) * 3) + 3],
                             bytes((0xFF, 0xFE, 0x00)))

    def test_shell_consumes_gsp_text_record_fixture(self):
        with tempfile.TemporaryDirectory() as directory:
            directory = pathlib.Path(directory)
            binary = self.compile_shell(directory)
            table = [0] * (128 * 4)
            table[0x43 * 4:0x43 * 4 + 4] = [0x633E, 0x0303,
                                             0x6303, 0x003E]
            table[0x72 * 4:0x72 * 4 + 4] = [0x0000, 0x6E3E,
                                             0x0606, 0x0006]
            table_path = directory / "text-table.bin"
            records_path = directory / "text-records.bin"
            frame = directory / "record-frame.ppm"
            table_path.write_bytes(b"".join(struct.pack("<H", value)
                                               for value in table))
            records_path.write_bytes(b"".join(struct.pack("<H", value)
                for value in (0x4013, 0, 0x00D4, 0x0108,
                              0x7243, 0x0000, 0, 0)))
            env = dict(os.environ)
            env.update({
                "STUNRUN_GSP_TEXT_TABLE_BIN": str(table_path),
                "STUNRUN_GSP_TEXT_RECORD_BIN": str(records_path),
                "STUNRUN_GSP_TEXT_RECORD_BASES": "0xFFFEA7D0",
                "STUNRUN_GSP_TEXT_Y_BIAS": "0x28",
                "STUNRUN_RENDER_PPM": str(frame),
            })
            run = subprocess.run([str(binary)], env=env,
                                 capture_output=True, text=True, check=False)
            self.assertEqual(run.returncode, 0,
                             f"shell failed:\n{run.stdout}\n{run.stderr}")
            self.assertIn("mode=gsp-text-record-fixture", run.stdout)
            payload = frame.read_bytes()
            header = b"P6\n512 240\n255\n"
            self.assertEqual(payload[len(header) +
                                    ((224 * 512 + 213) * 3):
                                    len(header) + ((224 * 512 + 213) * 3) + 3],
                             bytes((0xFF, 0xFE, 0x00)))

    def test_shell_consumes_optional_geometry_fixture(self):
        with tempfile.TemporaryDirectory() as directory:
            directory = pathlib.Path(directory)
            binary = self.compile_shell(directory)
            table = directory / "geometry.bin"
            table.write_bytes(b"".join(
                struct.pack(">H", 0xA000 + value) for value in range(384)))
            env = dict(os.environ)
            env["STUNRUN_GEOM_TABLE_BIN"] = str(table)
            run = subprocess.run([str(binary)], env=env,
                                 capture_output=True, text=True,
                                 check=False)
            self.assertEqual(run.returncode, 0,
                             f"shell failed:\n{run.stdout}\n{run.stderr}")
            self.assertIn("geometry-upload fixture=loaded passes=2 bytes=768",
                          run.stdout)
            self.assertIn("copies=match", run.stdout)
            self.assertIn("fifo-drain=192/384 dest=0x00C0000C", run.stdout)

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

    def test_shell_reports_populating_upload_state(self):
        with tempfile.TemporaryDirectory() as directory:
            binary = self.compile_shell(directory)
            path = pathlib.Path(directory) / "mid.json"
            path.write_text(json.dumps({
                "schema": "arcade-experiment/v1",
                "id": "mid-upload",
                "start": "power_on",
                "events": [],
                "expect": {"kind": "frame", "frame": 409},
            }), encoding="utf-8")
            run = subprocess.run([str(binary), str(path)],
                                 capture_output=True, text=True, check=False)
            self.assertEqual(run.returncode, 0,
                             f"shell failed:\n{run.stdout}\n{run.stderr}")
            self.assertIn("upload=populating install_ready=0", run.stdout)
            self.assertIn("RESULT PASS", run.stdout)


if __name__ == "__main__":
    unittest.main()
