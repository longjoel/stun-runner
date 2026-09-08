cmake_minimum_required(VERSION 3.20)
project(stunrun_native_smoke LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(stunrun-native-smoke smoke.cpp)
# Agent 2 M3 slice: the native target consumes the same C init-slice
# sources as the reproduction target (single canonical implementation,
# compiled into both). C99, no extra dependencies.
add_executable(stunrun-adsp-init-slice-test
  ../reproduction/adsp/adsp_init_image.c
  ../reproduction/adsp/adsp_init_image_test.c)
target_include_directories(stunrun-adsp-init-slice-test PRIVATE
  ../reproduction/adsp)
set_target_properties(stunrun-adsp-init-slice-test PROPERTIES C_STANDARD 99)
add_executable(stunrun-adsp-image-dump
  ../reproduction/adsp/adsp_init_image.c
  ../reproduction/adsp/adsp_image_dump.c)
target_include_directories(stunrun-adsp-image-dump PRIVATE
  ../reproduction/adsp)
set_target_properties(stunrun-adsp-image-dump PROPERTIES C_STANDARD 99)
# Agent 2 M3 slice 2: 68010-visible ADSP control/IRQ contract plus the
# upload/install frame boundary. Same single-implementation rule as above.
add_executable(stunrun-adsp-control-slice-test
  ../reproduction/adsp/adsp_control_seq.c
  ../reproduction/adsp/adsp_control_seq_test.c)
target_include_directories(stunrun-adsp-control-slice-test PRIVATE
  ../reproduction/adsp)
set_target_properties(stunrun-adsp-control-slice-test PROPERTIES C_STANDARD 99)
# Agent 2 M3 slice 3: 68010-side ADSP program-upload stream framing (17
# records, destination gap, 0xFF terminator). Same rule as above.
add_executable(stunrun-adsp-upload-slice-test
  ../reproduction/adsp/adsp_upload_stream.c
  ../reproduction/adsp/adsp_upload_stream_test.c)
target_include_directories(stunrun-adsp-upload-slice-test PRIVATE
  ../reproduction/adsp)
set_target_properties(stunrun-adsp-upload-slice-test PROPERTIES C_STANDARD 99)
enable_testing()
add_test(NAME native-smoke COMMAND stunrun-native-smoke)
add_test(NAME adsp-init-slice-c COMMAND stunrun-adsp-init-slice-test)
add_test(NAME adsp-control-slice-c COMMAND stunrun-adsp-control-slice-test)
add_test(NAME adsp-upload-slice-c COMMAND stunrun-adsp-upload-slice-test)
# Agent 2 sound slice: JSA-II command/response latch transport (68010 <->
# 6502). Same single-implementation rule as the ADSP slices above.
add_executable(stunrun-jsa-latch-slice-test
  ../reproduction/sound/jsa_latch.c
  ../reproduction/sound/jsa_latch_test.c)
target_include_directories(stunrun-jsa-latch-slice-test PRIVATE
  ../reproduction/sound)
set_target_properties(stunrun-jsa-latch-slice-test PROPERTIES C_STANDARD 99)
add_test(NAME jsa-latch-slice-c COMMAND stunrun-jsa-latch-slice-test)
# Agent 2 M4 step 1: deterministic native shell. Fixed 600-frame walk over
# the five verified slices, checkpoint on stdout, exit 0 iff all asserts
# hold. No speculative game logic.
add_executable(stunrun-native-shell
  shell.c
  render.c
  gsp_video.c
  checkpoint.c
  experiment.c
  ../reproduction/adsp/adsp_init_image.c
  ../reproduction/adsp/adsp_control_seq.c
  ../reproduction/adsp/adsp_upload_stream.c
  ../reproduction/maincpu/fifo_block.c
  ../reproduction/maincpu/geom_upload.c
  ../reproduction/maincpu/road_fifo.c
  ../reproduction/gsp/text_cursor.c
  ../reproduction/sound/jsa_latch.c)
target_include_directories(stunrun-native-shell PRIVATE
  ../reproduction/adsp ../reproduction/maincpu ../reproduction/gsp
  ../reproduction/sound)
set_target_properties(stunrun-native-shell PROPERTIES C_STANDARD 99)
add_test(NAME native-shell COMMAND stunrun-native-shell)
add_executable(stunrun-render-test render.c render_test.c)
set_target_properties(stunrun-render-test PROPERTIES C_STANDARD 99)
add_test(NAME native-render-boundary-c COMMAND stunrun-render-test)
add_executable(stunrun-gsp-video-test gsp_video.c render.c gsp_video_test.c)
set_target_properties(stunrun-gsp-video-test PROPERTIES C_STANDARD 99)
add_test(NAME native-gsp-video-boundary-c COMMAND stunrun-gsp-video-test)
# Literal GSP indexed-record addressing and raw field positions from the M5
# runtime parser trace. No field semantics are assigned here.
add_executable(stunrun-gsp-record-parser-slice-test
  ../reproduction/gsp/record_parser.c
  ../reproduction/gsp/record_parser_test.c)
target_include_directories(stunrun-gsp-record-parser-slice-test PRIVATE
  ../reproduction/gsp)
set_target_properties(stunrun-gsp-record-parser-slice-test PROPERTIES C_STANDARD 99)
add_test(NAME gsp-record-parser-slice-c COMMAND stunrun-gsp-record-parser-slice-test)
# Literal GSP FIFO-fed work-buffer setup from the M5 consumer trace.  This
# preserves the masked selector, 16-byte stride, and byte-lane operation
# without assigning semantics to either destination buffer.
add_executable(stunrun-gsp-work-buffer-consumer-slice-test
  ../reproduction/gsp/work_buffer_consumer.c
  ../reproduction/gsp/work_buffer_consumer_test.c)
target_include_directories(stunrun-gsp-work-buffer-consumer-slice-test PRIVATE
  ../reproduction/gsp)
set_target_properties(stunrun-gsp-work-buffer-consumer-slice-test PROPERTIES
  C_STANDARD 99)
add_test(NAME gsp-work-buffer-consumer-slice-c
  COMMAND stunrun-gsp-work-buffer-consumer-slice-test)
# Literal GSP PIXBLT descriptor lane/cursor state from the M5 register tap.
add_executable(stunrun-gsp-text-cursor-slice-test
  ../reproduction/gsp/text_cursor.c
  ../reproduction/gsp/text_cursor_test.c)
target_include_directories(stunrun-gsp-text-cursor-slice-test PRIVATE
  ../reproduction/gsp)
set_target_properties(stunrun-gsp-text-cursor-slice-test PROPERTIES
  C_STANDARD 99)
add_test(NAME gsp-text-cursor-slice-c
  COMMAND stunrun-gsp-text-cursor-slice-test)
# arcade-experiment/v1 parser self-check (valid, corrupt, and schema-
# violating fixtures; no ROMs or MAME involved).
add_executable(stunrun-experiment-parser-test
  experiment.c
  experiment_test.c)
set_target_properties(stunrun-experiment-parser-test PROPERTIES C_STANDARD 99)
add_test(NAME experiment-parser-c COMMAND stunrun-experiment-parser-test)
# File-driven replay of the canonical coin_start experiment through the
# shell: parse, frame dispatch, anchors, and checkpoint must all hold.
add_test(NAME native-shell-coin-start
  COMMAND stunrun-native-shell ${CMAKE_CURRENT_SOURCE_DIR}/../experiments/stunrun/coin_start.json)
# stunrun-checkpoint/v1 emitter contract (golden comparison lives in
# tests/test_checkpoint_golden.py against the M1 oracle fixture).
add_executable(stunrun-checkpoint-test
  checkpoint.c
  checkpoint_test.c)
set_target_properties(stunrun-checkpoint-test PROPERTIES C_STANDARD 99)
add_test(NAME checkpoint-emitter-c COMMAND stunrun-checkpoint-test)
# Agent 2 maincpu gameplay coverage: live-score accumulator, award
# selection, and hit counter against the weapon_probe/center golden runs.
add_executable(stunrun-score-slice-test
  ../reproduction/maincpu/score.c
  ../reproduction/maincpu/score_test.c)
target_include_directories(stunrun-score-slice-test PRIVATE
  ../reproduction/maincpu)
set_target_properties(stunrun-score-slice-test PROPERTIES C_STANDARD 99)
add_test(NAME score-slice-c COMMAND stunrun-score-slice-test)
# Persistent NVRAM high-score table decoder against the 10-entry golden
# table (score bytes exact; name prefix only — padding unestablished).
add_executable(stunrun-nvram-scores-slice-test
  ../reproduction/maincpu/nvram_scores.c
  ../reproduction/maincpu/nvram_scores_test.c)
target_include_directories(stunrun-nvram-scores-slice-test PRIVATE
  ../reproduction/maincpu)
set_target_properties(stunrun-nvram-scores-slice-test PROPERTIES C_STANDARD 99)
add_test(NAME nvram-scores-slice-c COMMAND stunrun-nvram-scores-slice-test)
# ADSP-buffer to GSP-FIFO block framing and transfer-shape contract.
add_executable(stunrun-fifo-block-slice-test
  ../reproduction/maincpu/fifo_block.c
  ../reproduction/maincpu/fifo_block_test.c)
target_include_directories(stunrun-fifo-block-slice-test PRIVATE
  ../reproduction/maincpu)
set_target_properties(stunrun-fifo-block-slice-test PROPERTIES C_STANDARD 99)
add_test(NAME fifo-block-slice-c COMMAND stunrun-fifo-block-slice-test)
# Literal trajectory coordinate initialization and clamp mechanism from the
# 0x0387C6-0x038A14 listing slice.
add_executable(stunrun-trajectory-state-slice-test
  ../reproduction/maincpu/trajectory_state.c
  ../reproduction/maincpu/trajectory_state_test.c)
target_include_directories(stunrun-trajectory-state-slice-test PRIVATE
  ../reproduction/maincpu)
set_target_properties(stunrun-trajectory-state-slice-test PROPERTIES C_STANDARD 99)
add_test(NAME trajectory-state-slice-c COMMAND stunrun-trajectory-state-slice-test)
# Course-table march and twin-buffer fan-out against the traced upload
# mechanism (384-word +2 march, count-gated dual copy, 0x300 stride).
add_executable(stunrun-geom-upload-slice-test
  ../reproduction/maincpu/geom_upload.c
  ../reproduction/maincpu/geom_upload_test.c)
target_include_directories(stunrun-geom-upload-slice-test PRIVATE
  ../reproduction/maincpu)
set_target_properties(stunrun-geom-upload-slice-test PROPERTIES C_STANDARD 99)
add_test(NAME geom-upload-slice-c COMMAND stunrun-geom-upload-slice-test)
add_executable(stunrun-road-fifo-slice-test
  ../reproduction/maincpu/road_fifo.c
  ../reproduction/maincpu/road_fifo_test.c)
target_include_directories(stunrun-road-fifo-slice-test PRIVATE
  ../reproduction/maincpu)
set_target_properties(stunrun-road-fifo-slice-test PROPERTIES C_STANDARD 99)
add_test(NAME road-fifo-slice-c COMMAND stunrun-road-fifo-slice-test)
add_executable(stunrun-road-buffer-math-slice-test
  ../reproduction/maincpu/road_buffer_math.c
  ../reproduction/maincpu/road_buffer_math_test.c)
target_include_directories(stunrun-road-buffer-math-slice-test PRIVATE
  ../reproduction/maincpu)
set_target_properties(stunrun-road-buffer-math-slice-test PROPERTIES C_STANDARD 99)
add_test(NAME road-buffer-math-slice-c COMMAND stunrun-road-buffer-math-slice-test)
# Object-hit -> award-select -> score-add chain with instruction-stream
# immediates (0x1F4 at 0x3A2E6, dest words 0xFFDD02/0xFF9532).
add_executable(stunrun-award-path-slice-test
  ../reproduction/maincpu/award_path.c
  ../reproduction/maincpu/award_path_test.c)
target_include_directories(stunrun-award-path-slice-test PRIVATE
  ../reproduction/maincpu)
set_target_properties(stunrun-award-path-slice-test PROPERTIES C_STANDARD 99)
add_test(NAME award-path-slice-c COMMAND stunrun-award-path-slice-test)
