# Current BEL performance follow-up â€” 2026-10-29

The preceding Quest logs contain active native-emulation windows around93â€“94% speed, with corresponding audio underruns; menu/sleep intervals are excluded. The original raster-work gate still used an inherited mode2/window1 check, while BEL's measured gameplay uses mode21/submode0 and world window2.

The corrected conservative gate can skip duplicate CPU3D drawing only when the current owned geometry, resources, frame coverage and protected native state are valid. Boot/title/service, direct framebuffer, explicit reference/capture, rejected/incomplete frames and missing resources retain the native renderer. No arcade clocks, original input, audio synthesis or game program are changed.

Paired60-native-second desktop runs used identical initial native NVRAM and deterministic cabinet inputs. CPU reference took20.9643 wall seconds; guarded raster took17.1166 (18.35% less time). All12 one-MiB native RAM snapshots and all104 firing/projectile events were byte-for-byte identical. The guarded run skipped2643 duplicate world frames and retained original rendering for806 non-play frames plus3 no-world frames. Production GL tests confirm healthy3D presentation is independent of the duplicate CPU composite image.

These are unthrottled Windows results with sound output disabled, not Quest FPS. The new signed Quest build is installed for active-play measurement. App submission cadence, native active speed, phase/player state, raster decisions and audio intervals are recorded separately; compositor GPU FPS, thermals and physical comfort are not inferred from build success. See feedback-sky-performance-report.md for the current candidate and remaining acceptance.

---

# Historical initial status

The existing reference engine's isolated BEL attract run completed 60 native seconds in 16.8 wall seconds (`Average speed: 511.63%` for the native 59-second measured interval). This is an unthrottled Windows native diagnostic with video/audio output disabled; it is not Quest performance or a real-time stereo benchmark.

No headset pacing, sustained play, audio equivalence, thermal or input-latency measurements have been made for BEL. Optional common raster/UART/audio optimizations remain disabled by default. Do not infer playability from library compilation.
