# Platform notes

- Independent Android identity: `com.vrlightgunhub.bel`; native libraries `bel_xr` / `bel_audio`; Windows executable target `bel.exe`.
- Quest support target is ARM64 Quest 3 / Quest 3S. Windows support target is x86-64 OpenXR. Compilation and device acceptance are distinct.
- Android owned media folder: `/sdcard/Android/data/com.vrlightgunhub.bel/files/roms/`; `bel.zip` contains the 31 manifest payloads. Downloads access remains optional. No separate billboard firmware or CHD is required.
- Local development signing key is private at `D:/VR4ALL-BEL-Build/private/bel.keystore`; never include it in software packages. No prior released signing identity exists for this new app.
- Native Test is IN0 `0x08`, Service `0x04`, main triggers IN1 `0x01/0x02`, Missiles IN1 `0x10/0x20`. ADC is eight-bit with channel order P1 X, P2 X, P1 Y, P2 Y. Driver and actual native Input Test agree. There is no invented reload.
- Source, engine dependencies, native state, original media and compiler caches are separate. Existing game ports and original archives are read-only references.
- Optional inherited raster skip, UART coalescing and adaptive audio playback are disabled in the BEL Android launcher until BEL-specific equivalence is measured.
- Native readiness, credit acknowledgement, independent player activation and saved calibration are fingerprinted and tested. The measured focus400 world inverse is resolved at the native delivery tick; virgin/invalid calibration, unsupported views, stale player/session intent and unreachable targets suppress firing. Entirely erased calibration now runs the original native Gun Calibration and Save/Exit sequence automatically, without seeded NVRAM or game-memory writes. Existing non-erased calibration is preserved. Original gun extraction and accepted-shot effects remain unverified; feedback stays closed. Actual death/continue play remains untested.
- Wider view and the provisional common rendering lens require BEL neutral replay and stereo checks before acceptance. Physical headset geometry, aim, audio, haptics and pacing remain untested.
- Media-free packages must pass structural/ABI/signature checks. No publication, device install or headset session is part of this work.
