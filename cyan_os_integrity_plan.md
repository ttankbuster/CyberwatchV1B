# Cyan OS Integrity Initiative — Stepped Plan

*Formalized plan for hardening Cyan OS: fault tolerance, service criticality,
a native settings system, power-aware rendering, and a remote shell — bounded
by a full audit on each end.*

---

## Cross-cutting rule, applies to every phase below

**Each phase ends with a full boot-and-smoke-test on real hardware before
the next phase begins, followed by a git tag at that known-good point.**
This project has already hit the same regression twice (`tabCount`
initialization silently disappearing during unrelated edits, reintroducing
a crash that was already fixed once). An 8-plus-phase restructuring effort
touching core architecture is exactly the environment that happens in
again, quietly, several phases deep, with no easy way to know which phase
introduced it. This rule is non-negotiable, not a nice-to-have.

---

## Current status (updated as work progresses)

- **Phase 0, audit pass complete, both confirmed bugs now fixed in the
  working tree.** Originally decided to keep Phase 0 audit-only and defer
  both bugs (the `app_handler.c` buffer overflow and the
  `display_st7789.cpp` out-of-bounds read) to Phase 1 alongside the
  rename — superseded: fixed directly instead, each as its own isolated
  change (see Audit Findings Log for the diffs). **Committed** as
  `de4f58a` ("memory safety fix"), which also included the
  `main_esp32.cpp` dead-code cleanup and this plan doc itself.
  **PC half of the boot-and-smoke-test complete**: native build compiles
  clean and the launched binary stays up and responsive (window "Watch",
  no crash) — confirms the `app_handler.c` fix. Tagged at that point:
  `phase0-pc-verified` → `de4f58a`. **Still outstanding**: the ESP32
  hardware boot test for the `display_st7789.cpp` icon-dimension fix,
  which is ESP32-only and unverifiable on the PC build. Phase 0 isn't
  fully closed until that runs too.
  - **Follow-up commit landed outside this documented flow**: `8e53165`
    ("memory safety fix #2") added three more `app_handler.c`/`.h` changes
    on top of `de4f58a` — see Audit Findings Log for detail on each.
    Reconciled here since it wasn't logged when made, matching the
    cross-cutting rule's concern about silent drift between phases.
    PC boot-and-smoke-test re-run against this commit specifically
    (native build clean, binary stayed alive and responsive, indexed all
    5 apps correctly, no crash) — tagged `phase0-pc-verified-2` →
    `8e53165`. ESP32 hardware boot test is still the one outstanding item
    blocking Phase 0 closure — unverifiable from this environment,
    needs to be run on real hardware.
  - **Explicit decision, 2026-08-16**: no ESP32 hardware access right
    now. Rather than stall, proceeding into Phase 1 with Phase 0 held
    open specifically on that one item — the PC-verifiable half (the
    `app_handler.c` overflow, and everything else audited off the
    source directly) is done and tagged. The ESP32 boot test for the
    `display_st7789.cpp` icon fix will be run once hardware is
    available again; this is a deliberate, logged exception to the
    cross-cutting rule's "boot-test before the next phase" ordering,
    not a silent skip.
  - `src/` reconciliation: **resolved** — confirmed directly against the
    repo tree that no `src/` directory exists; nothing to migrate.
  - Entry points audited (`main_pc.c`, `main_esp32.cpp`) — see Audit
    Findings Log below.
  - `cyan_os.c`/`cyan_os.h` audit: **unblocked and complete** — read
    directly off disk (`cyan/cyan_os.c`, `cyan/cyan_os.h`) once working
    from Claude Code, which has direct filesystem access; the prior
    attachment-delivery issue doesn't apply in this environment. Audit
    expanded outward from there to everything `cyan_os.c` pulls in
    directly: `app_handler.c/h`, `surface.c/h`, `services.h`,
    `services_base.c`, and both platform backends
    (`data_pc.c`/`display_sdl.c`, `data_esp32.cpp`/`display_st7789.cpp`) —
    see Audit Findings Log below. Two concrete, reproducible bugs found
    (buffer overflow, out-of-bounds read); both deferred to Phase 1, see
    decision above.
- **Decision made, deferred**: `CyberwatchData` → `CyanData` rename,
  agreed but intentionally not executed until Phase 1 (naming pass), so
  it happens as one deliberate sweep rather than piecemeal. Scope
  confirmed during this pass: touches `cyan_os.c`, `cyan_os.h`, `data.h`,
  `app_handler.h`, `services.h`, and every platform data file — matches
  what Phase 1 already expected, no surprises. **Done** — landed in
  `6d60554`.
- **Gap reconciled, 2026-08-29**: this status section hadn't been updated
  since `6d60554`, but 8 more commits had landed since (mostly unrelated
  hardware/case-design/documentation work — `485a9d7`, `b2f2483`,
  `c793bb0`'s electronics portion, and the schematic/analogue commits
  through `9153969`). One of those, **`c793bb0`, silently renamed every
  `app_handler_*` function to `AppHandler_*`** (PascalCase) without being
  logged anywhere — exactly the undocumented-drift scenario the
  cross-cutting rule exists to catch, since every other module
  (`services_*`, `surface_*`, `display_*`, `cyan_*`) stayed snake_case.
  Latest commit (`0ef6e52`, "added code style enforcement") also added
  `.clang-format` plus two **empty** stub files, `cyan/cyan_shell.c` and
  `cyan/cyan_shell.h` — read as early Phase 3 scaffolding, but Phase 2
  (criticality model) hasn't been started yet (confirmed: `services.h`
  has no `CyanCriticality` enum or `staticCriticality`/`dynamicCriticality`
  fields). **Explicit decision**: revert the `AppHandler_*` drift back to
  `app_handler_*`, finish Phase 1's two still-outstanding dead-code items
  before moving on, and leave the `cyan_shell.c/h` stubs untouched for
  now — Phase 2 comes next, per the plan's own sequencing.
  - Reverted in the working tree: `app_handler_init`, `app_handler_launch`,
    `app_handler_unload`, `app_handler_run_frame` (was renamed
    `AppHandler_update`), `app_handler_dispatch_events`,
    `app_handler_shutdown`, `app_handler_index`, `app_handler_print`, and
    the four Lua draw closures (`app_handler_draw_rect/width/height/text`,
    previously `lua_draw_*` before `c793bb0`, then `AppHandler_draw_*`) —
    all back to snake_case, across `app_handler.c`, `app_handler.h`, and
    every call site in `cyan_os.c`. See Audit Findings Log for detail.
  - Both remaining Phase 1 dead-code items (see Audit Findings Log,
    original entries) are now resolved: `cyan/data/timer_handling.c`
    deleted (empty, vestigial), `display_load_image()` in
    `display_st7789.cpp` deleted (dead, zero call sites, confirmed again
    before removal).
  - PC boot-and-smoke-test re-run against this working tree: native build
    clean, binary launched, stayed alive and responsive for 5+ seconds
    with no crash or error output, terminated cleanly. Tagged
    `phase1-pc-verified` once committed.
  - **New finding, not fixed this pass**: `cyan/tabs/screen_app.c` and
    `screen_catalogue.c` have their own pre-existing mixed-case names —
    `clay_AppHandler_app`, `clay_AppHandler_catalogue`,
    `render_AppHandler_catalogue`, `AppHandler_catalogue_move` — embedding
    `AppHandler` PascalCase inside otherwise snake_case names. These
    predate `c793bb0` (unrelated to that drift) and predate this plan's
    Phase 0 audit entirely. Left as-is — out of scope for this specific
    drift-revert — but logged as a Phase 1 naming-sweep candidate.

---

## Audit Findings Log

*Running log, appended to as each file is reviewed — not just Phase 0's
findings, every phase that touches existing code adds here.*

### `platform/pc/main_pc.c`, `platform/esp32/main_esp32.cpp`

| Finding | Severity | Disposition |
|---|---|---|
| ESP32 `loop()`'s `bool running` is declared fresh every call, value never meaningfully consulted afterward | Informational | **Accepted, known pattern** — ESP32's `loop()` has no real "exit" concept regardless. Recorded so a future reader doesn't mistake this for an oversight |
| Commented-out dead code (`// while(1)delay(1000);`) in `setup()` | Trivial | Cleanup item — sweep in Phase 1 |
| `main_pc.c` uses `clock()` for `dt` — measures CPU time, not strictly wall-clock; can diverge under some schedulers | Informational | Recorded as a known characteristic, matches original project convention — not changed unless later deliberately decided otherwise |
| Neither entry point has bounded-time protection around `esp32_hardware_init()`/`cyan_init()` failure — both halt forever or return an error | Expected, pre-Phase-4 | **Not a new finding** — this is exactly what Phase 4's bounded-time-init work replaces |

### `cyan/cyan_os.c`, `cyan/cyan_os.h`, and everything they directly pull in (`app_handler.c/h`, `surface.c/h`, `services.h`, `services_base.c`, `data_pc.c`/`display_sdl.c`, `data_esp32.cpp`/`display_st7789.cpp`)

| Finding | Severity | Disposition |
|---|---|---|
| `app_handler.c:184-185` (`app_handler_index`) — `snprintf(app.name, MAX_FILE_PATH, ...)` and `snprintf(app.path, MAX_FILE_PATH, ...)` passed `MAX_FILE_PATH` (256) as the size bound, but `app.name`/`app.path` are each `char[MAX_FILE_NAME]` (64 bytes) per the `AppEntry` struct (`app_handler.h:18-19`). For `app.path`, the written content is `"<path>/<foldername>"` — not bounded to ≤63 chars the way `folders.names[i]` alone is — so a folder name of roughly 60+ characters overflowed past `path`'s 64-byte field into the adjacent `icon` field. `app.name`'s copy was currently masked (safe only because `scan_folder` truncates source names to ≤63 chars first), but the size argument itself was still wrong. | **Bug — reachable stack buffer overflow** | **Fixed** — originally logged as deferred to Phase 1, but fixed immediately instead once flagged: both calls now use `sizeof(app.name)` / `sizeof(app.path)`. Still needs the boot-and-smoke-test before this is considered closed |
| `display_st7789.cpp:109` and `:114` — `convertRgb888ToRgb565(ICON_EMPTY_TAB, ICON_BATTERY_WIDTH, ICON_BATTERY_HEIGHT)` and the equivalent for `ICON_FULL_TAB` passed the **battery** icon's dimensions instead of their own. Confirmed against the actual asset headers: `ICON_BATTERY` is 30×18 (540px), `ICON_EMPTY_TAB`/`ICON_FULL_TAB` are each 12×12 (144px) — so this read ~396 pixels (1.5KB) past the end of a 144-pixel static array on every boot, and also left `iconEmptyTab.pixels`/`iconFullTab.pixels` populated with the wrong stride relative to `width`/`height` — garbled tab icons, independent of the OOB read. PC's `display_sdl.c:55-62` never had this bug. | **Bug — confirmed out-of-bounds read + wrong render** | **Fixed** — originally logged as deferred to Phase 1, but fixed immediately instead once flagged: both calls now use each icon's own `ICON_EMPTY_TAB_WIDTH/HEIGHT` / `ICON_FULL_TAB_WIDTH/HEIGHT` constants. ESP32-only change — needs the boot-and-smoke-test on real hardware before this is considered closed (can't be verified on the PC build) |
| `IconHandle`/`imageHandle` pattern — explicit audit target from the Phase 0 scope below | N/A | **Checked, consistent — no bug.** It's a deliberately opaque, per-platform `void*`: PC (`data_pc.c`/`display_sdl.c`) always writes/reads it as `SDL_Texture*`; ESP32 (`data_esp32.cpp`/`display_st7789.cpp`) always writes/reads it as `IconHandle*`. Platform-agnostic code (`screen_catalogue.c:72`) only passes it through to `display_draw_image`, never dereferences it directly. No cross-platform mixing found. |
| `cyan/data/timer_handling.c` is an empty file, despite its name implying it holds `timer_init`/`timer_cycle_element`/`timer_toggle`/`timer_spinbox_input` (declared in `data.h`). Traced and resolved: those are actually implemented in `cyan/tabs/screen_timer.c`, and `stopwatch_toggle`/`stopwatch_reset` in `cyan/tabs/screen_stopwatch.c`. | Trivial | **Not a missing-implementation bug** — vestigial/misleadingly-named empty file. **Deleted**, 2026-08-29 |
| `display_load_image()` in `display_st7789.cpp:166` is defined but has no call sites anywhere in the codebase and isn't declared in `display.h`. | Trivial | Dead code — **deleted**, 2026-08-29, re-confirmed zero call sites first |
| `app_handling/app_handler.c`, all six public functions (`app_handler_init/launch/unload/run_frame/dispatch_events/shutdown`) plus internal `app_handler_index` and the Lua-facing statics, were silently renamed to `AppHandler_*` (PascalCase) in `c793bb0`, undocumented at the time. | **Undocumented drift, matches the cross-cutting rule's exact concern** | **Reverted to snake_case**, 2026-08-29, per explicit decision — see Current status above for full list and reasoning |
| `cyan_os.h`'s include guard is `CYBERWATCH_H` (`cyan_os.h:2-3`) — leftover from the pre-rename name. Extends the Phase 1 naming-drift note (which already covers function naming) to include guards specifically. | Informational | Folds into the already-deferred `CyberwatchData` → `CyanData` sweep in Phase 1, not a new decision |
| Input debounce — confirmed at its exact location: `data_esp32.cpp:43-63` (`pollButtons`) does pure edge detection (`pressed != lastButtonNPressed`), no time-based debounce anywhere in the poll path. | Confirmed, matches Phase 0 scope | Not a new finding — this is the debounce gap the plan already named as an explicit audit target (see below); recorded here with its concrete file/line now that the file has been read directly |
| `app_handler.c:186` (`app_handler_index`, landed in `8e53165`) — even after the `sizeof(app.path)` fix, `snprintf` silently truncates rather than erroring when `"<path>/<foldername>"` still doesn't fit `app.path`'s 64 bytes. A truncated path is a distinct hazard from the original overflow: it's a valid-looking path to the *wrong* location (or a nonexistent one), which fails later, further from the actual cause. | Hardening, not a new memory-safety bug (the overflow itself was already fixed) | **Fixed** — `snprintf`'s return value is now checked; a folder name that would overflow `app.path` is logged and the entry is skipped instead of silently truncated. Covered by this pass's PC boot-and-smoke-test (`phase0-pc-verified-2`) |
| `app_handler.c:266` (`app_handler_run_frame`, landed in `8e53165`) — no null check on `AppHandler->appLua` before dereferencing it via `lua_getglobal`. Traced every call site (`cyan_os.c:117-189`): all four `data.state` writes (`launch_app`, `exit_app`, init, shutdown) update `state` and `appLua` together, synchronously, single-threaded — so a null-`appLua` call during `CYW_APP_RUNNING` isn't reachable through any currently-existing control path. | Defensive hardening, not a demonstrated live bug under current control flow | **Fixed** — added as a guard anyway (cheap, and Phase 6's capability-enforcement/Lua-bounds work will add more control paths here where this stops being trivially true). Worth re-checking reachability once Phase 6 lands |
| `app_handler.h` — `app_handler_count`, `app_handler_name`, `app_handler_icon`, `app_handler_is_running` were declared but never defined or called anywhere in `cyan/`. | Trivial | **Removed** in `8e53165`, ahead of the general Phase 1 dead-code sweep — confirmed zero remaining references before deletion |

**Before changing anything.**

- **Reconcile the actual current file tree against what's been assumed in
  prior planning.** Confirmed via direct repo check: `src/` still exists
  as a top-level folder alongside `cyan/`, contradicting the last project
  status summary's description of a full migration. Resolve this first —
  know what you're actually starting from.
- First audit pass, specifically for:
  - Memory safety (null-pointer risk, buffer bounds, the `IconHandle`
    pattern's consistent application across every image-loading path).
  - Logic errors.
  - **Input debounce** — confirmed absent. Current edge detection
    (`pressed != lastPressed`) has no time-based debounce at all; real
    mechanical switch bounce can register multiple spurious events within
    milliseconds. Named explicitly as an audit target, not left implicit.
- Output: a concrete, written list of findings — this becomes the
  baseline the Phase 9 final audit is measured against.

---

## Phase 1 — Naming & casing standardization

Mechanical, done early and deliberately so every subsequent phase is
written correctly the first time.

- Confirm and formalize the existing convention (already consistently
  followed): `PascalCase` types, `camelCase` fields, `snake_case`
  functions, `SCREAMING_SNAKE_CASE` enum values/macros.
- New rule going forward: every new public symbol not already covered by
  an established sub-prefix (`display_*`, `app_handler_*`, `services_*`)
  gets `cyan_*`.
- Sweep existing inconsistencies introduced during the earlier Cyan OS
  reframe (drifted top-level function naming around the `cyan_os_*` /
  bare-name transition).
- **`CyberwatchData` → `CyanData` rename** — agreed during Phase 0,
  executed here as one deliberate sweep rather than piecemeal. Touches
  every file with a `data->` access, so this should be done as its own
  isolated commit, boot-tested on its own before anything else in this
  phase proceeds on top of it.
- ~~Two bugs carried in from the Phase 0 audit, deferred here rather than
  fixed piecemeal~~ — superseded, fixed directly during Phase 0 instead
  once flagged (see Audit Findings Log): the `app_handler.c:184-185`
  buffer-size mismatch and the `display_st7789.cpp:109/114` tab-icon
  dimension mixup. Nothing left for Phase 1 to carry on these two.
- **Dead-code sweep, done 2026-08-29**: `cyan/data/timer_handling.c`
  (empty, vestigial) and `display_st7789.cpp`'s dead `display_load_image()`
  both removed. The `main_esp32.cpp` commented-out dead code was already
  removed earlier (part of `de4f58a`).
- **Undocumented `AppHandler_*` naming drift, found and reverted
  2026-08-29** — see Current status and Audit Findings Log for detail.
  Function naming across `cyan/` is snake_case-consistent again as of
  this pass.
- **Still open, deliberately not touched this pass**: `screen_app.c`'s
  `clay_AppHandler_app` and `screen_catalogue.c`'s
  `clay_AppHandler_catalogue` / `render_AppHandler_catalogue` /
  `AppHandler_catalogue_move` mix `AppHandler` PascalCase into otherwise
  snake_case names. Pre-existing, predates this plan's Phase 0 audit —
  a real Phase 1 naming-sweep candidate, not yet actioned.

---

## Phase 2 — Criticality model

Foundational — Phases 4–7 all consume this.

- New type: `CyanCriticality` enum.
- `Service` struct gains two fields: `staticCriticality` and
  `dynamicCriticality` — one enum, two variables per service, as specified.
- **Static** tiers, assigned and reasoned:

  | Service | Tier | Reasoning |
  |---|---|---|
  | Time | HIGH | Core purpose of the device |
  | Input | HIGH | Without it, nothing is reachable at all |
  | Storage | MED | Apps/settings need it; core timekeeping doesn't |
  | Power | MED | Shutdown/sleep control is safety-relevant; battery-% is convenience — may split later |
  | Apps | MED | Real loss of function, but Time/Timer/Stopwatch are core OS now, not apps |
  | Network | LOW | Core watch function needs zero connectivity |
  | Bluetooth | LOW | Same reasoning, moot until built |
  | Display, Notifications | **Foundational — outside the tier scale** | Both are load-bearing for the failure-reporting mechanism itself |

- **Dynamic** override rule: a service's dynamic criticality is elevated
  above its static baseline only while a *currently running app's*
  manifest declares a dependency on it (see Phase 6), reverting to static
  baseline on app exit. No open-ended dynamic recalculation beyond this
  one explicit rule.

---

## Phase 3 — Serial shell

**Deliberately sequenced here, early** — not because it's simple, but
because it directly de-risks Phases 4 and 5 by giving live, on-demand
visibility into boot ordering and service-monitoring behavior while
they're being built, rather than relying solely on screen rendering or
scrolling serial log output.

- Minimal command shell over the existing USB-CDC serial connection first
  (`help`, `services` — dump the live registry with current criticality
  and availability, `log <level>` — adjust runtime verbosity, `reboot`).
- Architecture: reuse the existing `log.h` listener pattern — the shell is
  just another listener, plus a command-input side reading from `Serial`.
- WiFi transport deferred to a later pass once the serial version is
  proven — same command layer, different transport, not a rewrite.
- Headless-mode implication: once this exists, a unit with no display
  attached (or a `Display` failure) can still be fully operated and
  diagnosed through the shell — this is what allows Display's
  "foundational" status from Phase 2 to be *softened* later: the shell
  becomes a real fallback path for critical notifications when Display is
  down, not just a debug convenience.

---

## Phase 4 — Fault-tolerant boot & watchdog

- **Bounded-time init for every hardware service.** Current model handles
  a service *failing* cleanly; it doesn't yet handle one *hanging*
  (a genuine risk — I2C buses can lock up under fault conditions). Every
  `init()` gets an actual timeout, not just a pass/fail return.
- **Boot ordering — reconciled explicitly against already-proven physical
  constraints, not left to a naive criticality sort.** This project has
  two hard, non-negotiable ordering requirements discovered the hard way:
  MCP23017 must init before the display's `RST` pulse, and SD's SPI setup
  must run *after* the display's own SPI init or the board hangs. Rule:
  **respect known hard physical dependencies first; use static criticality
  only to order whatever's left** after those constraints are satisfied.
- **Watchdog timer** (already on the original project TODO, folded in here
  as the natural last line of defense for this whole phase).
- **Crash-surviving log sink.** New `log.h` listener that persists recent
  log lines to SD (falling back to NVS if SD is absent) before a
  watchdog-triggered reset. Nearly every hardware bug in this project's
  history was diagnosed by having a serial monitor live at the exact
  moment of a crash — this removes that dependency, giving real
  post-mortem debugging. The Phase 3 shell becomes the natural way to
  retrieve this log after reboot (a `crashlog` command).

---

## Phase 5 — Service monitoring loop

- Periodic re-registration / availability re-check for every service.
- Check frequency driven by **dynamic** criticality (per Phase 2) —
  scales down under low-battery conditions specifically for LOW-tier
  services (`Network`, `Bluetooth`), directly tying the criticality model
  to the project's stated power-efficiency goal.
- Criticality-significant availability changes fire a notification via
  `Notifications` (falling back to the Phase 3 shell if `Display`/
  `Notifications` themselves are down).

---

## Phase 6 — App capability declarations, SD hot-swap, Lua VM bounds

- **Manifest addition**: Lua app manifests gain a declared `requires =
  {...}` field (e.g. `{"Storage", "Network"}`).
- `app_handler` cross-checks a running app's declared requirements against
  live service availability (fed by Phase 5's monitoring loop).
- **SD hot-swap behavior**: card removed while an app that requires
  `Storage` (or another now-unavailable required service) is running →
  that app exits gracefully, a notification displays, the OS itself does
  not crash. An app with no such dependency, already loaded into RAM,
  continues running.
- **Lua VM bounds** — new, addressing a real gap in the current sandbox
  design: nothing currently stops a misbehaving script from hanging the
  synchronous per-frame update loop (`while true do end`) or growing
  unbounded memory. Add an instruction-count hook forcing a yield/timeout,
  and a memory ceiling per app VM. This belongs here specifically because
  it's the same `app_handler` boundary already being hardened for
  capability enforcement — one enforcement layer, not two.

---

## Phase 7 — Native settings system

- **Lookup hierarchy**: `SD → NVS → defaults`, structured to allow
  additional sources later without redesign.
- **Format**: `key=value`, versioned from the start (`version=1` field) —
  cheap now, allows detecting/migrating old files once the format ever
  changes, rather than silently misparsing them later.
- **Validation**: every known setting has a declared type and valid
  range/enum at its definition point; the parser rejects or falls back to
  default for anything outside that — never trusts a raw parsed value
  downstream. Directly a memory-safety/security audit target, since this
  parses untrusted external input by definition.
- **Write safety**: settings writes go to a `.tmp` file first, which
  replaces the real file only on successful completion, then is deleted —
  protects against a corrupt half-written file if power is cut mid-write
  (directly relevant given the project's own hardware power-isolation
  switch). Checksum-based validation deferred to a later pass, per your
  own scoping.
- **Storage location**: `.cyanos/` on SD (lowercase, matching the
  project's existing tree-naming convention).
- Deliberately independent of the Lua app-sandbox layer — settings do not
  share a base system with apps, since settings are more foundational.

---

## Phase 8 — Power-aware rendering

- Move off unconditional per-frame redraw toward event/dirty-flag-driven
  rendering.
- New event type for the one genuine exception: the watchface needs to
  visibly progress without external input. `EVENT_CLOCK_SECOND_TICK` (or
  an RTC-interrupt-driven equivalent) as the minimum-refresh escape hatch,
  detailed design deferred per earlier discussion.

---

## Phase 9 — Final audit

- Full pass against everything landed in Phases 1–8, closing the loop
  against the Phase 0 baseline.
- Explicit named targets, not just general review: memory safety across
  every new component (settings parser, shell command parser, Lua bounds
  enforcement), the debounce fix from Phase 0, and full ERC-equivalent
  logic review of the criticality/monitoring interaction.

---

## Footnoted, not blocking any phase

- **Unity-based unit tests** for the settings parser and service registry
  specifically — the two most pure-logic, security-relevant, hardware-free
  pieces in this whole plan. Folded in opportunistically as each is built,
  not scheduled as its own phase.
