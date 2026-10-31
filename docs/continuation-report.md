# BEL calibration continuation — 2026-10-29

This is a local software test candidate, not an accepted or published VR release. No headset installation/launch or publication was performed.

## Changes

- Read original saved gun calibration through BEL ROM/RAM fingerprints; reject virgin FF, invalid/stale spans, unsupported camera states and unreachable targets.
- Implement the measured native angle/point conversion and screen inverse for the observed focus400 live world. Resolve physical aim at the native delivery tick, with session/player ownership checks.
- Require independent physical trigger/Missile releases after calibration recovery. Isolate deliberate service-gun intent from queued gameplay input and correct native service calibration's vertical direction.
- Preserve original saved NVRAM and the existing development signing identity. Runtime ABI0x42454c04 pairs current Windows/ARM64 engines and hosts.
- Repair BEL's private offline Gradle dependency cache. Include the optional built audio DLL and its dependency/export validation in portable Windows packages when present.

## Evidence

| Check | Result |
| --- | --- |
| Original native samples |7,890 TGP points; independent mathematical comparison below0.000003 native units |
| Production forward/inverse capture check |7,760 live points,7,744 visible screen inversions; maximum inverse error0.000261757 pixels |
| Actual pre-conversion-yaw capture |5,302 live points including native yaw resets; forward error2.00958e-6 units, inverse error0.000218965 pixels |
| Real engine + host, virgin calibration |60 native seconds, both players acknowledged, all5,212 sampled world trigger bytes released |
| Real engine + host, saved calibration |60 native seconds, both players acknowledged and native main-gun input delivered;4,308 target points including the horizontal step within0.838841 pixels of the old/new target |
| Runtime transitions |Both hand layouts, P1/P2, changing yaw, invalid→valid recovery, held actions, service transitions and stale session/player intent pass |
| Windows/ARM64 |Both engines and OpenXR/audio hosts rebuilt successfully |
| CTest |18/18 pass; Release assertions remain enabled |
| Python / Android/platform |16 +13 cases pass; standalone native source checks pass |
| Quest APK |Built and verified; correct package, ARM64 contents, preserved signer and media exclusion |

The independent review's Important findings were fixed with regressions that failed first and then passed. Its coordinate-normalization finding was also reproduced through `NativeLens`, then corrected. No additional review round was dispatched. Existing aggregate-initializer/indentation compiler warnings in inherited renderer/gun-pose code remain.

Private logs/captures stay under `D:/VR4ALL-BEL-Build`; exact execution evidence and decisions are in [continuation-progress.md](continuation-progress.md). Original handoff evidence remains in [handoff.md](handoff.md); it describes the earlier baseline. Current source/artifact manifests are generated separately for this candidate.

## First physical-test scope and limits

On first use, enter **Open Service Menu → Gun Calibration**, perform both native stop sequences and Save. Existing user NVRAM is not reset. Then test solo hands, both player layouts, native Start/join, main trigger/Missile, menu/focus transitions, aim, original audio, stereo/HUD and sustained pacing.

Physical Quest3/3S and PCVR rendering, aim, audio, haptics and performance are untested. Actual death/continue/routes remain untested. Native accepted-shot feedback/haptics stays closed. No original weapon extraction is claimed; Arcade/Zapper remain explicit fallbacks. Camera states outside the measured live view remain unsupported. Residual native-vs-replay raster differences and Wider-view/HUD acceptance remain open. Native compilation, desktop simulation and real headset play are separate results.

No release repository is selected or publication approved. Canonical local candidates are produced by `tools/package.py`; publication/download/installer checks are a later separately approved task.
