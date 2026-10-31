# BEL scale and first-run shooting correction

The user physically accepted startup, described FPS/audio as good, requested a larger world with Wider view enabled while keeping the existing 3D-screen mode, and reported that neither gun shoots. They requested continued implementation and installation without further design questions.

The verified virgin native RAM capture has all eight gun calibration bytes at FF. Native analog mode is already1, but native converter minima/spans become255/255; the existing fail-closed aiming guard consequently suppresses shots. The original service input capture establishes the genuine Gun Calibration procedure and save/reboot path. The adapter must complete that original procedure through cabinet inputs for an entirely erased calibration only, without injecting diagnostic NVRAM or writing game RAM/ROM. Preserve any existing non-erased calibration. A timeout must release inputs and leave manual service recovery available.

Implement and test the original state-driven first-run setup, including real P1/P2 stop values and the native Save/Exit path; keep firing, Start and credits released during setup. Verify a fresh native run saves valid calibration and that a second run preserves it, then verify real production trigger delivery. Fix manual calibration's P2 busy-state detection at the same observed service boundary.

Use independent saved scales for Wider view and the existing screen mode. Initial proposed Wider-view scale0.300 is three times the old0.100 metric scale; screen mode stays0.100. Preserve explicit non-default legacy size choices and later changes. Keep rendering and aiming on the identical active scale. This is BEL-specific presentation calibration, not a new common setting or a cross-game batch.

Check for an identifiable original player weapon; retain the explicit Arcade/Zapper fallback if no supported original model is found. Do not claim an invented mesh as the original gun.

Run meaningful native/unit and settings checks, fresh Windows and ARM64 builds, package/signature/hash checks and Quest installation. Record software, native simulation, headset observation and user acceptance separately. No publication is authorized.
