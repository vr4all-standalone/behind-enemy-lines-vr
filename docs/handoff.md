# Stable handoff — 2026-10-29

The parent is transferring this port to the visible **Behind Enemy Lines Model 2 VR** chat (`01a1897e-cc2a-78f1-a994-c810d7828b32`). Do not edit until the parent sets `readyToEdit` in the overseer session file. This agent has finished edits. No native build, headset session or publication is running. No final production Start SIM has been performed.

## Current result

BEL is a real, independently compiled Model 2C development port. Owned media, native boot/gameplay, independent native P1/P2 activation and credit consumption, service/calibration protocol, production player/hand guards, common UI/settings, native geometry replay, Windows host/engine and ARM64 host/engine have evidence. It is **not a playable VR release**: tracked firing remains closed at the unmeasured native aiming inverse. Original weapon extraction, accepted-shot feedback, physical headset checks and software-only delivery completion remain pending.

Package `com.vrlightgunhub.bel`; native runtime ABI `0x42454c02`, audio ABI `0x42454101`. Local new development key exists at `D:/VR4ALL-BEL-Build/private/bel.keystore`; preserve it. No prior released signing identity exists.

Exact final source/header hashes are in `source-manifest.json`; current engine/host artifact hashes and comparison of every copied engine header are in `final-artifacts.json`. The source manifest excludes mutable outputs/private media. Engine HEAD is exactly `f34f02505e32c1993c6a782b6814232cbfc74e36`, with the owned BEL patch stack; no sibling working-tree edits or compiler trees were copied.

## Verification status

- Production C++ regressions were developed RED→GREEN for BEL Test/Service, foreign-state rejection, Missile guards, angle migration/pose, measured native reader, real credit acknowledgement, native intro-to-world fire guards, P2-only continuation and transferred solo Start ownership.
- Parent's final handoff Windows pass: **17/17 CTest**, all assert-based targets compiled with `-UNDEBUG`, exit0 against the final readiness/helper source. Windows and ARM64 host refreshes also exited0. Logs are `bel-handoff-host-build.log`, `bel-handoff-ctest.log` and `bel-handoff-android-host-build.log` in the parent batch folder.
- Python media/patch suite **16/16**, Android/storage/delivery suite **13/13**. The actual production Android Java validator also accepted the complete owned 31-payload canonical archive.
- Final sequential Windows + ARM64 engine refresh **exit 0**, including the latest ready guard and reviewed headers. Logs: `build/engine-windows-final.log`, `build/engine-android-final.log`. All copied engine headers match current source.
- Full Quest build compiled/linked the ARM64 engine, copied its native libraries, created the new private BEL signing key and downloaded the pinned Gradle 9.4.1 distribution. APK assembly then **failed** because the new private offline Gradle cache lacks `com.android.application:9.2.1`. Full log `build/quest-build.log`; command session 65878 ended with exit 1. There is no current-source APK.
- Final engine/host production SIM, actual calibration inverse, death/continue play, headset render/play/audio and publication remain **not performed**. Do not relabel desktop traces or compilation as physical acceptance.

## Native evidence to preserve

`native-state-evidence.md` contains the exact original code and dynamic correlation. Important verified fields:

| Meaning | Native location / evidence |
| --- | --- |
| Native mode | Byte `0x5fa12c`; dispatcher at `0x56690`, table `0x29767f0` |
| Execution / Service | Byte `0x5a8831`: 1 normal, ff native Test; entry at `0x6957c`, normal write `0x12a4` |
| Title readiness | Mode 4 **phase 0**; initial mode 4/phase 4 logo ignores native Start |
| Demo / world | Demo mode 10/phase 1; actual world mode 21 |
| Native activation | Signed halves `0x5b4996`, `0x5b4a1a`, stride 0x84; native store `0x5d154` sets only the requested player to 2700 |
| Shared credit | Byte `0x5a87f0`; coin acknowledgement appears about .25s after press/release/debounce, then native Start consumes it |
| Settings / prices | Pointer `0x5a85e8 → 0x5a85f0`; start/continue bytes +15/+16; free play `0x5a8810` |
| Service page / phase | `0x5a883d`, `0x5b4b64`; Gun Calibration page 3, P1 phases 3/4, P2 phases 5/6 |

`research/native-state-join` used the independent BEL engine, read-only write taps, native P1 first/P2 second and 11 completed geometry captures. `research/native-state-p2first` independently proves P2 first/P1 second using the existing read-only reference engine. No RAM was changed. Do not confuse delivered buttons with acknowledgement.

`input_policy.h` now waits for actual credit acknowledgement, retains a quick Start tap and then emits a full Start pulse. Existing credit/free play avoids an extra coin. A request contributes at most one native coin; higher native prices require additional fresh requests. No direct credit/RAM write bypasses the game. Sole-controller transfer retains an already joined/continuing native seat; both controllers use saved assignments. The same helper drives controller Start labels.

## Main remaining gate: measured aiming inverse

Fresh private native NVRAM leaves all eight gun calibration bytes at **ff**. This is virgin state, not established user calibration. Preserve existing user NVRAM; do not reset or silently overwrite it.

The original converter at `0x632f0..0x63448` reads:

- P1/P2 X minima: `0x5a8607/08`; Y minima `0x5a8609/0a`.
- P1/P2 X deltas: `0x5a860b/0c`; Y deltas `0x5a860d/0e`.
- X offset floats: `0x5a17d4/18`; Y offset floats `0x5a2230/34`.
- X half-span floats: `0x5a021c/20`; Y half-span floats `0x5a0224/28`.
- Actual gun angle conversion is `0x63730..0x637dc`, with native X span at signed half `0x5b4a80`, Y span at `0x5b4b1c` and original floating/truncation behavior. The lightweight disassembler prints general register names for some FP operands; verify their mode bits against pinned `i960.cpp` before translating math.

The completed `research/calibrated-positive-span` run used fresh private state, visited native lower-left/upper-right stops for each player, explicitly selected native Save, selected Exit and observed a native reset. After save and reset, bytes persisted as **69 00 11 11 96 96 9d 9d**; P1 offset/half-spans were **105,17 / 75,78.5**. This establishes native storage, not a screen inverse. The later scripted Start in this same diagnostic happened during phase-4 initial logo and was ignored; the final readiness guard fixes that observation.

The native positive-span convention indicates vertical ADC increases toward the upper stop. The copied fallback `NativeRanges` screen-v mapping still needs its BEL-specific vertical convention corrected/tested when finishing the inverse. Do not infer screen calibration from Gunblade's physical port ranges. The first private diagnostic with lower-left Y=ae produced a wrapped delta63; retain it as a failed-direction observation, not valid calibration.

`read_native_calibration`, `native_aim_point` and `native_screen_to_adc` intentionally remain closed in `native/game_state.h`. Next: translate and validate the actual converter against controlled ADC sweeps and native gun/reticle/projectile coordinates, then prove native screen-ray alignment through the production aim adapter, including calibration recovery/fresh input and both players. No native shot counter or weapon ID has been enabled. Candidate statistics/angles in the player block are research only until independently audited.

## Geometry evidence

Actual BEL live world uses dominant focus400/400 in window2; title geometry uses330/330. Private frame2303 at native40.052736 seconds contains4,700 polygons; the production GPU replay emits1,476 world triangles. Neutral comparison differs at1,108/190,464 pixels (RGB mean absolute error0.02696, max135); independent left/right differ at152,574 pixels. Residual neutral differences remain to analyze. `renderer-evidence.json` records the numeric result. This is real geometry replay on desktop, not headset stereo acceptance. Wider view and native HUD placement still need visual acceptance.

## Exact remaining commands

The final host/CTest and both engines are now refreshed. These commands are for reproducibility or after further source changes. Run from this source root. Set `TEMP` and `TMP` to `D:/VR4ALL-BEL-Build/tmp`; C: was low during the run. At most two compiler jobs. Do not install or launch a headset or publish.

```powershell
$env:TEMP=$env:TMP='D:/VR4ALL-BEL-Build/tmp'
$env:PATH='D:/msys64/mingw64/bin;'+$env:PATH
$cmake='C:/Program Files (x86)/Microsoft Visual Studio/2022/BuildTools/Common7/IDE/CommonExtensions/Microsoft/CMake/CMake/bin/cmake.exe'
& $cmake --build D:/VR4ALL-BEL-Build/build/host-win -j2
& 'C:/Program Files (x86)/Microsoft Visual Studio/2022/BuildTools/Common7/IDE/CommonExtensions/Microsoft/CMake/CMake/bin/ctest.exe' --test-dir D:/VR4ALL-BEL-Build/build/host-win --output-on-failure
& $cmake --build D:/VR4ALL-BEL-Build/host-android --target bel_xr bel_audio -j2
& ./tools/test.ps1
```

Existing host configurations use MinGW GCC15.2 on Windows and NDK29.0.14206865/ARM64/Android29/c++_shared on Quest. For a fresh ARM64 configure:

```powershell
& $cmake -S . -B D:/VR4ALL-BEL-Build/host-android -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_MAKE_PROGRAM=E:/AndroidSdk/cmake/3.22.1/bin/ninja.exe -DCMAKE_TOOLCHAIN_FILE=E:/AndroidSdk/ndk/29.0.14206865/build/cmake/android.toolchain.cmake -DANDROID_ABI=arm64-v8a -DANDROID_PLATFORM=android-29 -DANDROID_STL=c++_shared
```

Engines are already refreshed to final source. If further headers change, refresh sequentially:

```powershell
& ./tools/build-engine.ps1 -MsysRoot D:/msys64 -PythonPath D:/Python314/python.exe -WorkEngine D:/VR4ALL-BEL-Build/engine -Jobs 2 -Platform Windows
& ./tools/build-engine.ps1 -MsysRoot D:/msys64 -PythonPath D:/Python314/python.exe -WorkEngine D:/VR4ALL-BEL-Build/engine -Jobs 2 -Platform Android -AndroidNdk E:/AndroidSdk/ndk/29.0.14206865 -SdlRoot D:/StorageRelief/ege19/2026-10-25/vr4all-workspace-cache/carnevil-port/sdl-android
```

After repairing the isolated Gradle dependency cache, retry packaging **without recompiling either engine or host**:

```powershell
& ./tools/build-quest.ps1 -Jobs 2 -Offline -SkipEngineBuild -SkipHostBuild
```

The missing AGP9.2.1 artifacts exist in the older port's dependency cache, but this run did not copy it or run Gradle against its writable home. Prefer a correctly seeded independent cache. Gradle documents copying only dependency-cache files/metadata while excluding locks/gc.properties, or a separate immutable read-only dependency cache; do not point a shared read-only cache at a live mutable cache. See [Gradle dependency cache documentation](https://docs.gradle.org/current/userguide/dependency_caching.html). Software dependency downloads are separate from forbidden commercial-media downloads.

With the final host refresh complete, perform the promised desktop production Start SIM in a **new** output directory:

```powershell
$src='<local-user>/OneDrive/Documents/ChatGPT/vr mod/ports/behind-enemy-lines-vr'
$env:BEL_VR='1'; $env:BEL_XR_SIM='1'; $env:BEL_SIM_GUN_TEST='1'
$env:BEL_XR_LIBRARY='D:/VR4ALL-BEL-Build/build/host-win/bel_xr.dll'
$env:BEL_ASSETS="$src/assets"
$env:BEL_XR_LOG='D:/VR4ALL-BEL-Build/research/production-start-sim/host.log'
$env:BEL_SETTINGS='D:/VR4ALL-BEL-Build/research/production-start-sim/settings.ini'
$env:BEL_BASELINE_SCRIPT="$src/tools/baseline.lua"
& D:/Python314/python.exe -B "$src/tools/run_diagnostic.py" --engine D:/VR4ALL-BEL-Build/engine/bel.exe --roms D:/VR4ALL-BEL-Build/private/roms --output D:/VR4ALL-BEL-Build/research/production-start-sim --seconds 60 --mode attract --script "$src/tools/state_observe.lua"
```

Clear any inherited diagnostic calibration/P2-first/service flags before the SIM. `mode attract` means baseline.lua sends no scripted Coin/Start: the actual host policy must create native credit, Start and P1→P2 acknowledgement. Confirm `Diagnostic OpenGL simulation ready; no headset session`, ABI acceptance, native states0/0→1/0→1/1, actual credit consumption, no Lua errors and bounded exit. Do not count its withheld aim/shot output as playable fire. No final SIM was run by this agent.

Publication, release repository verification, canonical media-free APK/PCVR packages, public-download verification and installer discovery remain outside this handoff and require the overseer's explicit batch approval.
