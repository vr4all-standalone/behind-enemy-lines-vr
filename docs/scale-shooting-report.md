# BEL scale and shooting correction — 2026-10-29

The user physically tested the previous Quest3S build, reported good FPS/audio, requested a larger Wider-view world while retaining the existing 3D-screen mode, and reported no shooting.

## Changes

- Entirely erased native calibration now completes the original Service/Gun Calibration/Save-Exit sequence automatically through cabinet inputs. The adapter neither writes game RAM/ROM nor seeds NVRAM. Existing non-erased calibration is untouched. Setup suppresses normal Coin/Start/trigger/Missile intent, releases inputs on suspension and has a bounded timeout with manual service recovery. Native P2 manual calibration busy-state detection now includes the observed submodes7–10.
- Wider view defaults to metric world scale0.300, three times the old0.100. Screen mode retains0.100. Both modes remember their own World size. Non-default legacy size choices are retained. Rendering, aim tracing, menu changes and grip/stick shortcuts use the same active scale. Settings version5 migrates once and preserves later choices.
- First-run guidance explains automatic preparation. The runtime ABI is0x42454c05, with matched Windows/ARM64 engine and host builds.

## Verification

The two new regressions failed before implementation. After implementation, all16 Python and15 Android/platform tests passed, as did17 standalone native programs and the complete20/20 CTest suite. The gun-setup test additionally checks that focus suspension releases the original controls without consuming its timeout. Existing inherited renderer/gun-pose warnings remain.

A real fresh-state90-native-second desktop integration run exited0 without Lua/capture errors. The production adapter entered native service at6.294648s, completed both original stop sequences, selected Save/Exit at16.864120s and allowed the original reboot. The saved bytes are6900111196969d9d. Both native players were acknowledged and real active-low native trigger samples were observed: P1 945/P2 796. No diagnostic calibration was injected and the observation script sent no Coin/Start or gun input.

A second60-native-second run reused only that game's newly saved NVRAM. It exited0, never re-entered service, retained the calibration, and delivered P1 768/P2 659 trigger samples. The production forward/inverse checker accepted6588 captured native points, with maximum coordinate error2.00958e-6 native units and inverse error0.000218965 native pixels. These samples prove software/native trigger delivery, not accepted-shot counters, headset aim or haptic feedback.

Both Windows and ARM64 engines and OpenXR/audio hosts built successfully. The signed Quest APK passed structure, ABI, package identity and signer continuity checks and was installed with data preserved; base.apk SHA2563597b076231f97d6f96d9c86ad1b42e500e97a5303a76d3bbe190fc2aeb72a78. Existing device bel.zip still matches90f08a0ad78952715dc2dd67293ad3c5452d27efd047d8418d85f90dea82ee03. No headset NVRAM/settings were copied or overwritten externally.

## Remaining acceptance

The headset was asleep after installation. The user has been asked to open BEL, wait for preparation/reboot, test firing and assess the new scale. That feedback and a native headset smoke check are pending; the running APK is installed, but physical firing/scale acceptance is not claimed. The user's positive FPS/audio report applies to the preceding build. No new audio or performance change was made.

No verified original player weapon model was identified in the current native extraction path; its weapon snapshot is still unpopulated. Arcade/Zapper remain honest fallbacks. No invented mesh is labeled original. Native accepted-shot/hit feedback and unmeasured camera/target behavior remain closed. Invalid partial/existing calibration still requires deliberate manual service adjustment; automatic setup is restricted to the fully erased eight-byte case.

Independent review was unavailable after the earlier review agent failed before execution due to its unsupported inherited model. The protocol was inspected inline and checked against original state transitions plus the fresh/saved real-engine runs. No review verdict is claimed. No sibling game, registry entry, Git branch or GitHub publication was changed by this correction.

Exact source diffs/hashes, commands, artifact hashes and device evidence are recorded separately in scale-shooting-source-manifest.json and scale-shooting-artifacts.json. Private captures remain under D:/VR4ALL-BEL-Build/research and private; no original game data is included in deliverables.
