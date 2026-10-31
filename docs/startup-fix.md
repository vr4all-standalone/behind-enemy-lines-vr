# BEL Quest startup repair — 2026-10-29

The user reported the wrong setup UI and inability to get past startup. This repair applies only to Behind Enemy Lines, matching the existing accepted launcher. No sibling games, native gameplay code, registry entries, Git branches or publication state were changed.

## Confirmed causes and changes

- BEL still used a plain purple setup activity instead of the accepted VR4ALL wide readiness/Play panel. Reused the common panel and production ReadyLaunchPolicy from the read-only Virtua Cop reference, adapted only for BEL branding, storage and its original validator. The manifest now requests the same resizable 860×640dp panel.
- The Quest app-owned ROM folder was empty. The APK contains software libraries/assets; the original game archive is separate. After explicit approval naming the exact source and destination, copied bel.zip to the app-owned roms folder. Source and device SHA256 match. No NVRAM, calibration, settings or saved state was seeded or overwritten.
- BEL still needs only bel.zip with all 31 original payloads. No separate firmware archive or CHD is required. Its production Java validator accepted the exact copied archive, checking size, CRC32 and SHA1 for all payloads.
- App-owned media works without All files access. Download remains an optional manually granted fallback. Play rechecks the archive; missing/invalid files cannot enter VR. Automatic entry is consumed once, returned sessions allow manual Play, and paused/stale/duplicate handoffs are guarded.

## Verification

Both new regression checks failed against the old launcher. All 15 Android/platform tests passed after implementation, including real production Java readiness and scoped-storage behavior. The release APK built successfully with the existing signer. All six source assets match APK bytes, and all six ARM64 libraries match the earlier verified calibration APK exactly. Native and PCVR targets were not rebuilt because their code and payloads did not change.

Canonical APK: D:/VR4ALL-BEL-Build/artifacts/startup-fix/behind-enemy-lines-vr-quest-0.1.0-dev.apk

APK SHA256: ad3d2d02d32e06568187400acbb97efb96fdf5bd2daf781cf4701f0be74cadbb

Installed on Quest 3S with adb install -r --no-incremental, exit0/Success. The installed base.apk hash matches the candidate. ROM SHA256: 90f08a0ad78952715dc2dd67293ad3c5452d27efd047d8418d85f90dea82ee03 (27,434,891 bytes).

Exact changed-source hashes, aggregate and all APK library/asset hashes are in startup-source-manifest.json and startup-artifacts.json. The continuation baseline was checked before editing and all existing hashes matched. Only two existing source files changed and three launcher source/test files were added. Source aggregate: ed092f1b02f9bf95aab39a0ad9925c93c4e0d1261e7e9472008f066680da93e9.

## Device startup verified

After the user resumed the headset, BEL GameActivity became the resumed activity. The app's log records OpenXR states1→2→3→4→5 (focused), completed VR4ALL splash, and advancing native Model2 frames with geometry (at one read: XR frame2400/native frame1547/4026 polygons). A stereo screen capture shows the original arcade scene, guns/lasers and the in-VR settings overlay. The missing-media setup blocker is resolved; the application reaches native VR rendering.

The shared 2D readiness panel was compiled and its wide-panel manifest and lifecycle were software-tested. A visible screenshot of that panel was not obtained: the headset initially slept, then showed a system physical-power-button prompt, and the user subsequently entered the game. The running game was left undisturbed rather than exited just to capture that panel.

Private evidence is under D:/VR4ALL-BEL-Build/private/startup-check; startup-artifacts.json records hashes. No game screenshots or original data were added to the source or distributable APK. Audio production is active, but diagnostic intervals include underruns and a long production gap, so clean audible playback/performance is not claimed. Native gun calibration, aim/accepted shots, both-player play, haptics and sustained performance still require separate acceptance. No NVRAM or settings were overwritten.

An independent reviewer was requested under the review skill, but failed before execution because its inherited model was unsupported by the account. No independent review verdict was obtained; the launcher and lifecycle adapter were inspected inline. No GitHub publication was requested or performed.
