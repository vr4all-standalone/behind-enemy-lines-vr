# Quest 3S installation — 2026-10-29

The user explicitly requested installation on Quest 3S after the calibrated candidate was prepared. This authorizes the installation despite the older handoff's device-use hold; no launch/play approval is inferred here.

- Connected device identifies as Quest 3S / panther. Foreground was the Quest Home activity; no other app was stopped or launched.
- BEL was not previously installed (`pm path com.vrlightgunhub.bel` returned no package).
- `adb install -r --no-incremental D:/VR4ALL-BEL-Build/artifacts/calibration/behind-enemy-lines-vr-quest-0.1.0-dev.apk` returned `Success`, exit0.
- Installed package: `com.vrlightgunhub.bel`, versionName `0.1.0-dev`, versionCode1, minSdk29/targetSdk36.
- Installed base APK SHA256 exactly matches the verified candidate: `dfeb5ed675fe0ae2b75fd143482927dcef1ef5371674fb829aa642b95804bf10`.
- No app launch, physical play, native calibration, haptic/audio/performance acceptance or publication was performed.

## ROM transfer blocked

The planned source is the existing verified owned archive `D:/VR4ALL-BEL-Build/private/roms/bel.zip` (27,434,891 bytes), SHA256 `90f08a0ad78952715dc2dd67293ad3c5452d27efd047d8418d85f90dea82ee03`. Intended destination: `/sdcard/Android/data/com.vrlightgunhub.bel/files/roms/bel.zip` on the Quest 3S. The planned command would preserve any differing existing ROM rather than overwrite it.

Automatic approval review rejected this transfer before process creation: it classified the private ROM archive as sensitive data egress and said that the user's APK-install request did not explicitly authorize the specific payload and destination. Neither directory creation nor the ROM transfer ran. No workaround was attempted. Explicit user approval for that exact copy is required before retrying.

## Startup repair follow-up

The user subsequently reported the wrong startup UI and approved the exact ROM copy through the named approval prompt. The copy succeeded and the device archive hash matches the verified source. The common wide VR4ALL launcher was rebuilt with the existing signer, installed as an update, and its installed APK hash was verified. See [startup-fix.md](startup-fix.md) and [startup-artifacts.json](startup-artifacts.json) for current evidence and remaining headset checks. The earlier transfer rejection above is historical; the approved copy has now completed.
