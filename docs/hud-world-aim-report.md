# BEL HUD, world laser and scale follow-up — 2026-10-29

The user confirmed the preceding update shoots and the scale improved, then requested still larger Wider view, a minimal HUD, removal of the unpleasant reticle and a laser that reaches the 3D world. Screen-mode scale remains explicitly liked.

Original captured native layers identify the bezel/status text in the foreground raster and the large green aiming grids in window4/focus400 geometry. Those overlay polygons could be selected before the world by the old painter-order aiming trace. Native life/fuel indicators use window4/focus330.

During actual native gameplay, the host masks the foreground to the original health, fuel and ammo regions, removes the native green aiming-grid geometry, and clips status indicators to those same regions at the status-panel distance. Boot, title and service presentation remain unchanged. No health/ammo values are invented or read from guessed counters.

Gameplay laser targeting excludes all native HUD/panel geometry, tests real world materials/alpha and chooses the nearest geometric hit. A miss extends50m along the tracked ray rather than intersecting the old1.8m screen plane. Rendered geometry and aiming retain the same active world scale and lens; original native gun constraints and accepted-shot evidence gates remain intact.

Wider view defaults to World size0.600, twice the preceding0.300 candidate. Settings version6 upgrades the previous recommended0.300 value once; other manual sizes persist. Screen-mode size stays unchanged and remains separately saved.

The HUD/aim regression failed before implementation. All21 CTests pass, including the production OpenGL renderer, native gun setup/aiming, nearest-surface/HUD rejection, misses, status-region extraction and size migration. A production GL replay of original native gameplay was visually inspected: no large bezel or green aiming grid remains, with native status retained. Private previews stay outside distributable source/assets.

Windows and ARM64 build/package/install results and exact source/artifact hashes are recorded in hud-world-aim-source-manifest.json and hud-world-aim-artifacts.json. Physical acceptance of this new HUD, scale and laser is pending. The user's confirmed shooting and improved scale apply to the preceding candidate.

This is a BEL-specific correction. No other mod, shared registry, native game program or source media is modified. No publication is authorized. An original player gun extraction remains unverified; explicit Arcade/Zapper fallbacks are unchanged. Independent review tooling was unavailable after the unsupported inherited-model error; no independent verdict is claimed.
