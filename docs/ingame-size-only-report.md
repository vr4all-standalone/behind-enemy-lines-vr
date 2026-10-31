# BEL 30% increase restricted to In-game — 2026-10-30

The user accepted the larger native sight and corrected player colors, then clarified that the 30% increase should apply only to In-game, not Custom. This supersedes the previous report's statement that both modes were enlarged.

Only In-game retains the 1.30 artwork multiplier. Custom returns to its prior `28 * reference / 400` transform, matching the accepted Gunblade-style geometry and sizing. The shared minimum/shrink function, saved options and target depth are unchanged. Native P1 remains green and P2 red; Custom remains red P1/blue P2. Wider view off keeps the original complete native presentation.

Existing production GL tests cover the enlarged In-game footprint, both native palettes, logical/physical-hand transitions, Custom rendering with a hollow center, minimum/shrink behavior and None. This is a small reversible artwork correction; no new duplicate implementation test is added. Exact fresh software/build/package/install results, source lineage and hashes are in `overseer/changes/2026-10-30-bel-ingame-size-only/`. Existing engine binaries and headers are unchanged and may be reused after verification. Physical acceptance of the corrected Custom size is separate from software and installation checks. No sibling, shared contract, registry, save or game-rule change; no GitHub publication.
