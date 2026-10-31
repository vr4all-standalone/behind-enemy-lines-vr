# Behind Enemy Lines implementation plan

> **For agentic workers:** Use superpowers:executing-plans to implement this plan task-by-task. The parent selected this isolated game agent and explicitly authorized continuous implementation.

**Goal:** Deliver a tested native BEL cabinet adapter integrated into an independently buildable Model 2 VR host; record remaining native evidence gates without inventing gameplay state.

**Architecture:** Reuse selected common host/render/audio/UI modules with source hashes and licensing. BEL owns cabinet mapping, conservative native-state boundary and Model2C hooks. Build output and private research are separate on D:.

**Tech stack:** C++20, pinned MAME 0.289, OpenXR/OpenGL ES, Python verification, Android Java/Gradle, Windows MinGW/CMake.

**Spec:** `docs/design.md`.

## Global constraints

- Only edit this new port. Do not modify sibling ports, engines, installer or overseer registry.
- MAME pin `f34f02505e32c1993c6a782b6814232cbfc74e36`; native hardware Model 2C.
- At most two compiler jobs; private mutable data under `D:/VR4ALL-BEL-Build`.
- No media downloads, headset install/launch, GitHub publication or invented RAM addresses.

## Review focus

- A held Missile must not fire after focus/menu/tracking/hand transitions.
- Native Test and Service are swapped relative to Gunblade; service navigation must use BEL bits.
- Player acknowledgement must come from measured native state, never from a Start request.
- Invalid or unverified calibration must suppress firing and feedback.
- Packages must have unique BEL identities and contain no commercial data or keys.

### Task 1: Cabinet and native evidence boundary

Files: `native/cabinet_io.h`, `native/game_state.h`, `tests/cabinet_io_test.cpp`, `tests/game_state_test.cpp`, `game-set.json`, `engine-lock.json`.

Interface: `bel::Cabinet`, active-low `in0/in1`, `adc`, `map_aim`; `read_game_state(Read32)` and native calibration return unverified until measured evidence exists.

- [x] Write and run cabinet tests against the reused baseline; observe failure on Test/Service swap and Missile delivery.
- [x] Implement BEL bits and ADC contract; rerun native tests.
- [x] Write native-state test that rejects Gunblade ROM/RAM signatures, observe failure, replace the game adapter with a conservative BEL evidence boundary, rerun.
- [x] Generate exact ROM manifest from pinned driver and inherited firmware; verify manifest identity and report missing media explicitly.

### Task 2: Missile and player/hand guards

Files: `native/input_policy.h`, `native/controller_controls.h`, `native/controller_labels.h`, `native/presentation.cpp`, `tests/input_policy_test.cpp`.

Interface: existing `HandInput::pump` carries the physical B/Y action; BEL delivers `Cabinet::missile[player]` using logical-player hand ownership. Native joined state remains authoritative.

- [x] Test Missile output, sustained input, solo transfer and suppression after focus/menu/layout/tracking transitions; run RED.
- [x] Add independent fresh-Missile guards and BEL operator-bit mapping; run GREEN with inherited common settings/input tests.
- [x] Update help labels and presentation text for Missile and unverified native state.

### Task 3: Host and platform integration

Files: `CMakeLists.txt`, `native/host_bridge.h`, `native/shared_runtime.h`, `patches/`, `tools/`, `android/`, `docs/provenance.json`, `docs/validation.md`.

Interface: `bel_xr_*` and `bel_audio_*` symbols, `BEL_*` environment names, package `com.vrlightgunhub.bel`; Model2C config installs the adapter on BEL only.

- [x] Reuse only selected common modules, assets and dependencies with provenance, adapt BEL identities and native Model2C hook.
- [x] Run full local C++/Python suite and compile Windows/ARM64 host libraries with two jobs.
- [x] Attempt private engine and supported platform packaging, inspect actual failures and record commands/exit codes.
- [x] Record source/artifact SHA256, missing native evidence and separate untested device/release gates.
