"""ROM-free consistency checks for MAME input-mode schedules.

Parses tools/mame-memory-snapshot --input choices and the
input_mode branches in mame/lua/memory-snapshot.lua as text: no MAME
run, no ROMs. Byte-compiles the lua with luac when available.
"""

import pathlib
import re
import shutil
import subprocess
import unittest


ROOT = pathlib.Path(__file__).resolve().parents[1]
WRAPPER = ROOT / "tools" / "mame-memory-snapshot"
LUA = ROOT / "mame" / "lua" / "memory-snapshot.lua"


def wrapper_modes():
    text = WRAPPER.read_text(encoding="utf-8")
    match = re.search(r"add_argument\(\"--input\", choices=\((.*?)\)", text, re.S)
    assert match, "input choices not found in wrapper"
    return set(re.findall(r'"([^"]+)"', match.group(1)))


def lua_modes():
    text = LUA.read_text(encoding="utf-8")
    return set(re.findall(r"input_mode == '([^']+)'", text))


def lua_events(mode):
    text = LUA.read_text(encoding="utf-8")
    start = text.find(f"input_mode == '{mode}'")
    assert start != -1, f"no lua branch for {mode}"
    chunk = text[start:text.find("} or {", start)]
    events = []
    for match in re.finditer(
        r"\{frame = (\d+), port = '([^']+)', field = '([^']+)',"
        r" action = '(\w+)'(, value = (\d+))?\}",
        chunk,
    ):
        frame, port, field, action = match.group(1, 2, 3, 4)
        events.append((int(frame), port, field, action))
    assert events, f"no parsable events for {mode}"
    return events


class InputModeTests(unittest.TestCase):
    def test_wrapper_modes_have_lua_branches(self):
        modes = wrapper_modes()
        branches = lua_modes() | {"none"}
        missing = modes - branches
        self.assertEqual(missing, set(), f"wrapper modes without lua branch: {missing}")

    def test_course_modes_present(self):
        for mode in ("course_sweep", "course_preface_left",
                     "course_preface_right", "course_coin2"):
            self.assertIn(mode, wrapper_modes())
            self.assertIn(mode, lua_modes())

    def test_press_precedes_release(self):
        for mode in lua_modes():
            held = {}
            for frame, port, field, action in lua_events(mode):
                key = (port, field)
                if action == "press":
                    held[key] = frame
                elif action == "release" and key in held:
                    self.assertGreater(
                        frame, held.pop(key),
                        f"{mode}: {field} released before pressed")

    def test_coin_precedes_start(self):
        for mode in ("course_sweep", "course_preface_left",
                     "course_preface_right", "course_coin2",
                     "late_drive", "late"):
            frames = [(frame, field, action)
                      for frame, _, field, action in lua_events(mode)]
            coins = [f for f, field, _ in frames if field.startswith("Coin")]
            starts = [f for f, field, _ in frames if "Start" in field]
            self.assertTrue(coins and starts, f"{mode}: missing coin/start")
            self.assertLess(min(coins), min(starts),
                            f"{mode}: start before coin")

    def test_lua_compiles(self):
        luac = shutil.which("luac")
        if luac is None:
            self.skipTest("luac not installed")
        proc = subprocess.run([luac, "-p", str(LUA)],
                              capture_output=True, text=True)
        self.assertEqual(proc.returncode, 0, proc.stderr)


if __name__ == "__main__":
    unittest.main()
