## Scale and shooting follow-up — 2026-10-29

The user physically accepted startup and reported good baseline FPS/audio. The subsequent larger Wider-view scale and erased-calibration correction are implemented, verified through fresh/saved real-game runs and installed on Quest3S.20/20 native CTests pass. Read [scale-shooting-report.md](scale-shooting-report.md) for the current results and remaining physical acceptance. Earlier sections below describe their historical candidates.

# Validation — 2026-10-29

## Latest dedicated-chat continuation

The measured native calibration/aim inverse and delivery-boundary guards are implemented and tested. Runtime ABI0x42454c04 Windows/ARM64 engines and hosts build;18/18 CTests pass. Final paired native/host simulations prove Start/join and calibrated native gun input, while virgin calibration keeps firing closed. The signed Quest APK now assembles and passes package/ABI/content checks. Read [continuation-report.md](continuation-report.md) and [continuation-progress.md](continuation-progress.md) for current results and remaining physical/native-feedback limits. No device use or publication occurred.

## Parent handoff baseline — superseded where stated above

This is an unpublished development foundation, **not a playable VR release**. No headset install/launch or GitHub publication occurred. The dedicated visible BEL chat owns the remaining work described in [handoff.md](handoff.md).

| Check | Result |
| --- | --- |
| Upstream engine | Own checkout at `f34f02505e32c1993c6a782b6814232cbfc74e36`, owned BEL patch stack |
| Owned media | All31 original payloads pass size/CRC32/SHA1; no separate firmware/CHD required |
| Original native boot/gameplay | Fresh bounded desktop diagnostics exit0; original title, jungle, P1-first and P2-first runs |
| Native readiness/credit/players | Twelve-word BEL fingerprint, native write taps and credit consumption corroborate the production reader |
| Native Service/Input Test/calibration/Exit | Observed; positive-span calibration explicitly saved and persisted across native reset in fresh private NVRAM |
| Python suite | 16/16 pass, exit0 |
| Android/storage/delivery suite | 13/13 pass, exit0; full real archive also passed production Java validation |
| Final Windows host and CTest | Parent final handoff build exit0; all17 CTests pass, assertions enabled |
| Final ARM64 hosts | Parent final handoff refresh exit0; BEL exports and ELF64 ARM64 verified |
| Final Windows engine | Full independent build and current-source refresh exit0 |
| Final ARM64 engine | Full independent build and current-source refresh exit0 |
| Quest APK | Failed at offline AGP9.2.1 plugin resolution, exit1, after native engine/library preparation and local key creation |
| Actual geometry replay |4,700 native polygons /1,476 world triangles; neutral mismatch1,108/190,464 pixels, RGB MAE0.02696; left/right differ152,574 pixels |
| Final production Start SIM | Not run; dedicated chat owns the exact desktop-only command in handoff |
| Native aiming inverse, original weapon extraction, accepted-shot effects | Pending; tracked firing and feedback fail closed |
| Actual death/continue play and physical Quest3/3S/PCVR acceptance | Not tested |
| Canonical packages, publication, downloads and installer discovery | Not ready / not performed |

Current source hash snapshot: [source-manifest.json](source-manifest.json), aggregate SHA256 `5e2ea6fb48eb89a92fc5131479be4eb193a69927fa9f0e81ff628e04f6f8d0a7`. It excludes private/mutable outputs and generated evidence documents. Exact current artifact paths, sizes and SHA256 hashes are in [final-artifacts.json](final-artifacts.json); every copied native engine header matches final source. Runtime ABI is `0x42454c02`; audio ABI `0x42454101`; Android identity `com.vrlightgunhub.bel`.

Parent's final logs: `overseer/changes/2026-10-29-model2-last-three/bel-handoff-host-build.log`, `bel-handoff-ctest.log`, `bel-handoff-android-host-build.log`. Engine logs: `build/engine-windows-final.log`, `build/engine-android-final.log`. Failed full Quest command: `tools/build-quest.ps1 -Jobs 2 -SkipHostBuild -Offline`; log `build/quest-build.log`. Repair isolated dependency resolution, then retry with `-SkipEngineBuild -SkipHostBuild` as documented in handoff.

Production tests were developed RED→GREEN for BEL cabinet/service bits, foreign-state rejection, Missile/focus/menu/hand guards, displayed-angle migration/pose, measured native state, real credit acknowledgement, intro-to-world firing guards, P2-only and transferred solo Start ownership, and initial-logo readiness. Controller callouts use the same native-player helper as actual Start routing. No native credit or acknowledgement is fabricated from a button request.

Common UI/settings source implements purple VR4ALL panels, logical P1 red/P2 blue Arcade plastic while preserving metal/lens details, displayed angle0 as former physical-10 degrees with a one-time schema marker, reversible assignments and Light/Medium/Strong haptics labels. Physical appearance and feedback are untested. Arcade/Zapper are explicit fallbacks; no extracted original weapon is claimed. Native main-gun hold and Missile replace an invented reload. Native aim inverse, service-gun vertical convention, Wider-view/HUD acceptance and residual neutral raster differences remain explicit limitations.

Earlier failures remain recorded: an initial full clone was rejected by automatic review, then replaced with a parent-authorized new repository fetching only the exact committed upstream pin at depth1. No whole game fork, sibling working changes or compiler outputs were copied. An initial host configure lacked MinGW runtime DLLs on PATH and passed after the toolchain environment was fixed. The inherited18-payload assertion was corrected to independently audited31 BEL payloads; disabled Release assertions were enabled and all tests rerun. The final APK failure is a real dependency-cache failure, not a successful package.
