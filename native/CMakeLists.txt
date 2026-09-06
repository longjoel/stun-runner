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
# the four verified slices, checkpoint on stdout, exit 0 iff all asserts
# hold. No speculative game logic.
add_executable(stunrun-native-shell
  shell.c
  experiment.c
  ../reproduction/adsp/adsp_init_image.c
  ../reproduction/adsp/adsp_control_seq.c
  ../reproduction/adsp/adsp_upload_stream.c
  ../reproduction/sound/jsa_latch.c)
target_include_directories(stunrun-native-shell PRIVATE
  ../reproduction/adsp ../reproduction/sound)
set_target_properties(stunrun-native-shell PROPERTIES C_STANDARD 99)
add_test(NAME native-shell COMMAND stunrun-native-shell)
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
