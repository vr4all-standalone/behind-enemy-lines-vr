# Quest3S follow-up — 2026-10-29

The headset subsequently awakened and the newly installed GameActivity became resumed. The updated native engine log distinguishes this run from the earlier appended history:

- Native4.0695s: erased1, saved config0, setup0.
- Native9.07609s: setup1, service1, main selection3.
- Native14.0827s: setup1, erased0, saved config1; original service save path in progress.
- Native19.0893s: original reboot in progress, setup still1.
- Native24.0959s: setup0, saved config1, service0.
- Native34.109s onward: saved config1 and live aim1/1, preserved through native48.3813s.

OpenXR log advances through XR3600/native2797 frames with Model2 geometry and Wider view on. The stereo capture shows the native arcade scene, HUD, controller gun and laser. This verifies the real headset's original calibration/save/reboot path and available aiming inverse; it does not independently verify controller shot acceptance or whether the new apparent scale meets the user's preference. That requested user feedback remains pending.

A fresh extraction of the final PCVR ZIP also passed bel.exe -verifyroms bel with a Windows-only PATH (exit0; one valid ROM set). Original ROMs were read from the existing private folder, not copied into the package.

Private device logs/screenshot and their hashes are recorded in scale-shooting-artifacts.json. No saved settings/NVRAM were externally replaced, and no publication occurred.
