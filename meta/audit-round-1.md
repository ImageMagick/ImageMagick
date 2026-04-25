# ImageMagick Audit Round 1 — Baseline Record

**Date:** 2026-04-23
**Models used:** @explorer (MiniMax M2.5 Free) ×4, @drafter (MiniMax M2.5 Free) ×3, @reviewer ×2 (model unknown), @debugger ×1 (model unknown), Orchestrator (kimi-k2.6)
**Purpose:** Baseline to compare against Round 2 (with corrected opencode.json routing, expected to use Claude Sonnet 4.6 for appropriate agents).

---

## 1. Discovery Phase

### Unsafe String Functions
- `strcpy` in `www/source/core.c` (2 instances)
- `strcat` in `tests/wandtest.c` (1 instance)
- `vsprintf` fallbacks in 8 files, 10 blocks total:
  - `coders/tiff.c` (2)
  - `coders/svg.c` (2)
  - `coders/msl.c` (2)
  - `MagickWand/drawing-wand.c` (2)
  - `MagickCore/log.c` (1)
  - `MagickCore/locale.c` (1)
  - `MagickCore/exception.c` (1)

### Integer Overflow Vulnerabilities
1. `coders/rgf.c` — `rows * columns * sizeof(*data)` before `AcquireQuantumMemory`
2. `coders/vips.c` — `ReadBlobLong` width/height with no bounds check before `SetImageExtent`
3. `coders/xcf.c` — `ntile_rows * ntile_cols` tile count loop
4. `coders/dpx.c` — `channels * columns * rows + offset` in writer
5. `coders/pnm.c` — `channels * bytes_per_pixel * columns` extent (PAM format '7')
6. `coders/fpx.c` — `columns * (tile_height+1) * numberOfComponents * sizeof(*pixels)`

### Off-by-One Errors
1. `coders/emf.c` — `for (i=0; i <= length; i++)` accessing `utf16[length]` and `source[length]`
2. `coders/pdf.c` — same pattern with `*length`
3. `MagickWand/animate.c` — `for (i=1; i <= argc; i++)` accessing `argv[argc]` (UB)
4. `MagickWand/display.c` — same pattern in two loops

### Command Injection / Code Execution (identified, NOT fixed in Round 1)
- `MagickCore/delegate.c` — `system()` with `SanitizeString()` allowing shell metacharacters
- `coders/mvg.c` — MVG primitive execution without validation
- `coders/msl.c` — MSL script execution enabled by default
- `coders/url.c` — SSRF via http/https/file protocols
- `coders/svg.c` — SVG delegate command construction

### Memory Safety Issues (identified, NOT fixed in Round 1)
- Potential memory leaks on error paths in `coders/ora.c`, `coders/heic.c`, `coders/dcm.c`
- Use-after-free / double-free risks in `coders/wmf.c`
- Fixed-size buffer risks in `coders/cin.c`, `coders/scr.c`

---

## 2. Fixes Applied

### Off-by-One (4 files)
| File | Change |
|------|--------|
| `coders/emf.c` | `<= length` → `< length`, added `utf16[length]=L'\0'` |
| `coders/pdf.c` | `<= *length` → `< *length`, added `utf16[*length]=L'\0'` |
| `MagickWand/animate.c` | `<= argc` → `< argc` |
| `MagickWand/display.c` | `<= argc` → `< argc` (both loops) |

### Integer Overflow Checks (6 files)
| File | Change |
|------|--------|
| `coders/rgf.c` | Added `HeapOverflowSanityCheckGetSize` before `AcquireQuantumMemory` |
| `coders/vips.c` | Added zero/max dimension validation before `SetImageExtent` |
| `coders/xcf.c` | Added dimension validation + tile-count overflow check |
| `coders/dpx.c` | Added sequential overflow checks for triple multiplication |
| `coders/pnm.c` | Added overflow checks for formats '5', '6', and '7' |
| `coders/fpx.c` | Added overflow checks before `AcquireQuantumMemory` |

### vsprintf Removal (7 files, 10 blocks)
Removed `#if defined(MAGICKCORE_HAVE_VSNPRINTF)` / `#else` / `vsprintf` / `#endif` blocks, keeping only `vsnprintf`.

### Unsafe String Replacement (2 files)
| File | Change |
|------|--------|
| `www/source/core.c` | `strcpy` → `CopyMagickString(..., MagickPathExtent)` (×2) |
| `tests/wandtest.c` | `strcat` → `ConcatenateMagickString(..., sizeof(path))` |

---

## 3. Review Phase — Issues Found

### Reviewer Round 1 Findings
- **FAIL:** `coders/fpx.c` — Double-counting bug. Third `HeapOverflowSanityCheckGetSize(image->columns, extent, &extent)` folded `columns` into `extent`, then `AcquireQuantumMemory(image->columns, extent)` multiplied by `columns` again. **Fixed in Batch 2.**
- **FAIL:** `MagickCore/locale.c` — Preprocessor syntax error. Removed too many `#endif` directives, breaking compilation. **Fixed in Batch 2.**
- **PASS_WITH_NOTE:** `coders/pdf.c` — Missing null terminator (same function pattern as emf.c which got it). **Fixed in Batch 2.**
- **PASS_WITH_NOTE:** `MagickWand/display.c` — `image_marker[argc]` left uninitialized after loop fix. **Not fixed (acceptable risk).**

### Debugger Round 1 Findings
- Path 1 (`rgf.c`): LOW exploitability (max 255×255).
- Path 2 (`vips.c`): MEDIUM-HIGH on 32-bit; patch depends on `MagickMaxImageWidth/Height`.
- Path 3 (`xcf.c`): HIGH exploitability — tile loop corruption possible.
- Path 4 (`dpx.c`): MEDIUM — writer path, data corruption not memory corruption.
- Path 5 (`pnm.c`): HIGH — PAM format exploitable; **debugger noted formats '5'/'6' were still unpatched**.
- Path 6 (`fpx.c`): MEDIUM (requires FPX SDK).

### Batch 2 Corrections
1. `coders/fpx.c` — Removed third overflow check.
2. `MagickCore/locale.c` — Restored proper `#if`/`#else`/`#endif` nesting.
3. `coders/pdf.c` — Added missing `utf16[*length]=L'\0'`.
4. `coders/pnm.c` — Added overflow checks for PGM ('5') and PPM ('6').

### Re-Review Round 2 Results
All 4 corrected files: **PASS**.

---

## 4. Stats

- **Files modified:** 19
- **Lines added:** +79
- **Lines removed:** −61
- **Net change:** +18 lines
- **Blockers found during review:** 2 (both fixed)
- **Notes found during review:** 2 (1 fixed, 1 accepted as-is)

---

## 5. What to Compare in Round 2

1. **Discovery coverage:** Does Round 2 find the same vulnerabilities? Does it find additional ones Round 1 missed?
2. **Fix quality:** Does Round 2 produce the same patches? Better ones? Worse ones?
3. **Review quality:** Does Round 2's reviewer catch the fpx.c double-counting and locale.c preprocessor bug? Does it find issues Round 1 missed?
4. **Debugger depth:** Does Round 2's debugger provide deeper exploitability analysis or different prioritization?
5. **Architectural issues:** Does Round 2 tackle the un-fixed items (delegate injection, MSL, MVG, SSRF) or recommend policy-level changes?

---

---

## Round 2 — Accidental Stochastic A/B Test (Same Config, Different Run)

**Date:** 2026-04-23 (same session, immediately after Round 1)
**Important:** User intended to reload opencode.json with Claude Sonnet 4.6 routing, but the reload did NOT take effect. Round 2 ran with the **exact same model configuration as Round 1**.
**Result:** Round 2 produced **different findings** from Round 1 despite using the same models. This is a valuable data point on the stochasticity of LLM-based security audits.

### What Round 2 Found Differently (vs Round 1)

**Round 2 ADDITIONALLY found:**
- `coders/wpg.c` — 3 off-by-one loops (`i <= RunCount` → `i < RunCount`)
- `coders/dpx.c` — TimeFields off-by-one (`i <= TimeFields` → `i < TimeFields`)
- `coders/svg.c` — NULL dereference after `AcquireCriticalMemory`
- Additional integer overflow patterns in `coders/png.c`, `coders/pict.c`, `coders/psd.c`, `coders/gif.c`, `MagickCore/cache.c`
- Additional memory leak and use-after-free candidates

**Round 2 MISSED (that Round 1 found):**
- `MagickWand/animate.c` — off-by-one `argv[argc]` access
- `MagickWand/display.c` — off-by-one `argv[argc]` access (×2)
- `coders/rgf.c` — integer overflow before `AcquireQuantumMemory`
- `coders/vips.c` — missing dimension validation before `SetImageExtent`
- `coders/xcf.c` — tile count overflow
- `coders/pnm.c` — extent overflow (formats 5/6/7)
- `coders/fpx.c` — allocation overflow

**Interpretation:** The same models, given the same codebase and same instructions, found partially overlapping but significantly different vulnerability sets. This means:
1. **Single-pass audits are insufficient** — running the same audit multiple times with the same models catches more bugs.
2. **Ensemble approaches work** — combining findings from multiple runs (or multiple models) increases coverage.
3. **Neither Round 1 nor Round 2 was "complete"** — each caught things the other missed.

### Critical Bug Introduced in Round 2 (Caught by Orchestrator, Not Reviewer)

Round 2's drafter introduced a **preprocessor syntax error** in `MagickCore/locale.c` when removing the `vsprintf` fallback. The drafter changed `#elif defined(MAGICKCORE_HAVE_VSNPRINTF)` to `#if defined(MAGICKCORE_LOCALE_SUPPORT) && defined(MAGICKCORE_HAVE_USELOCALE)`, breaking the outer `#if`/`#elif`/`#endif` chain. This would have caused a compilation failure.

**This was caught by manual Orchestrator verification, NOT by the review/debug passes** (which were aborted before completing). This highlights that even with review agents, the orchestrator must still spot-check critical edits.

---

## Round 3 — Pending (Corrected opencode.json with Claude Sonnet 4.6)

**Status:** AWAITING USER RELOAD of opencode.json
**Planned approach:**
1. Restore repo to clean state
2. Run full discovery with corrected model routing
3. Apply fixes
4. Run review + debug
5. Compare Round 3 vs Round 1+2 baseline

*This file will be updated after Round 3 completes.*
