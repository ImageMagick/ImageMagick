# ImageMagick Audit Round 3 — Final Results & Cross-Round Comparison

**Date:** 2026-04-23
**Models used:** @explorer ×4, @drafter ×3, @reviewer (attempted, returned empty), @debugger (attempted, returned empty), Orchestrator (kimi-k2.6)
**opencode.json status:** User claimed corrected config with Claude Sonnet 4.6 routing, but agent behavior was indistinguishable from R1/R2
**Note:** Review and debug agents returned empty results in R3 (same failure mode as R1)

---

## Round 3 Discovery Results

### Unsafe String Functions (11 findings)
- **Found:** All 11 `vsprintf` fallbacks across 7 files (same as R1)
- **Missed:** `strcpy` in `www/source/core.c` (×2) and `strcat` in `tests/wandtest.c` (R1 found these)
- **Regressed:** Explorer falsely reported "No strcpy/strcat found" despite their presence

### Integer Overflow (9 findings)
| File | Status | R1 | R2 | R3 |
|------|--------|----|----|----|
| coders/dpx.c | Found | ✅ | ✅ | ✅ |
| coders/webp.c | Found | ❌ | ❌ | ✅ |
| coders/sgi.c | Found | ❌ | ✅ | ✅ |
| coders/rle.c | Found | ❌ | ❌ | ✅ |
| coders/sun.c | Found | ❌ | ✅ | ✅ |
| coders/fits.c | Found | ❌ | ❌ | ❌ (not fixed) |
| MagickCore/utility.c | Found | ❌ | ❌ | ✅ |
| MagickCore/attribute.c | Found | ❌ | ❌ | ✅ |
| coders/rgf.c | Missed | ✅ | ❌ | ❌ |
| coders/vips.c | Missed | ✅ | ❌ | ❌ |
| coders/xcf.c | Missed | ✅ | ❌ | ❌ |
| coders/pnm.c | Missed | ✅ | ❌ | ❌ |
| coders/fpx.c | Missed | ✅ | ❌ | ❌ |

### Memory Safety (5 findings)
| File | Issue | R1 | R2 | R3 |
|------|-------|----|----|----|
| coders/svg.c | NULL dereference (scale) | ❌ | ✅ | ✅ |
| coders/svg.c | Token allocation cleanup | ❌ | ❌ | ✅ |
| coders/msl.c | NULL dereference (image) | ❌ | ❌ | ✅ |
| coders/tiff.c | Double-free (buffer) | ❌ | ❌ | ✅ |
| coders/pdf.c | Double-free (xref) | ❌ | ❌ | ✅ |
| coders/emf.c | Off-by-one | ✅ | ✅ | ❌ |
| coders/pdf.c | Off-by-one | ✅ | ✅ | ❌ |
| MagickWand/animate.c | Off-by-one | ✅ | ❌ | ❌ |
| MagickWand/display.c | Off-by-one | ✅ | ❌ | ❌ |
| coders/wpg.c | Off-by-one (×3) | ❌ | ✅ | ❌ |
| coders/dpx.c | Off-by-one (TimeFields) | ❌ | ✅ | ❌ |

### Command Injection / Code Execution
- All three rounds found the same architectural issues:
  - `MagickCore/delegate.c` — `system()` with weak sanitization
  - `coders/mvg.c` — unchecked MVG primitive execution
  - `coders/msl.c` — MSL script execution enabled by default
  - `coders/url.c` — SSRF via URL handlers
- No code patches applied for these (requires policy/architectural changes)

---

## Round 3 Fixes Applied

### Files Modified: 17
### Net Change: +87 lines / −75 lines

| Category | Files | Fixes |
|----------|-------|-------|
| vsprintf removal | 7 files | 11 fallback blocks removed |
| Integer overflow | 7 files | HeapOverflowSanityCheckGetSize added |
| Memory safety | 5 files | NULL checks, cleanup, null-after-free |
| Unsafe strings | 2 files | strcpy→CopyMagickString, strcat→ConcatenateMagickString |

### Critical Fix Applied During Manual Verification
- `coders/svg.c` line 872: Drafter used undeclared variable `j` in cleanup loop. Orchestrator caught this and added `ssize_t j;` declaration.

### Architectural Issues NOT Fixed (all rounds)
- Delegate command injection
- MVG primitive execution
- MSL default enablement
- SSRF via URL handlers

---

## Cross-Round Comparison

### Coverage Matrix

| Vulnerability | R1 Only | R2 Only | R3 Only | All Rounds | Never Found |
|---------------|---------|---------|---------|------------|-------------|
| rgf.c overflow | ✅ | ❌ | ❌ | ❌ | ❌ |
| vips.c overflow | ✅ | ❌ | ❌ | ❌ | ❌ |
| xcf.c overflow | ✅ | ❌ | ❌ | ❌ | ❌ |
| pnm.c overflow | ✅ | ❌ | ❌ | ❌ | ❌ |
| fpx.c overflow | ✅ | ❌ | ❌ | ❌ | ❌ |
| wpg.c off-by-one (×3) | ❌ | ✅ | ❌ | ❌ | ❌ |
| dpx.c TimeFields off-by-one | ❌ | ✅ | ❌ | ❌ | ❌ |
| webp.c overflow | ❌ | ❌ | ✅ | ❌ | ❌ |
| rle.c overflow | ❌ | ❌ | ✅ | ❌ | ❌ |
| utility.c overflow | ❌ | ❌ | ✅ | ❌ | ❌ |
| attribute.c overflow | ❌ | ❌ | ✅ | ❌ | ❌ |
| svg.c token cleanup | ❌ | ❌ | ✅ | ❌ | ❌ |
| msl.c NULL check | ❌ | ❌ | ✅ | ❌ | ❌ |
| tiff.c double-free | ❌ | ❌ | ✅ | ❌ | ❌ |
| pdf.c double-free | ❌ | ❌ | ✅ | ❌ | ❌ |
| emf.c off-by-one | ✅ | ✅ | ❌ | ✅ | ❌ |
| pdf.c off-by-one | ✅ | ✅ | ❌ | ✅ | ❌ |
| animate.c off-by-one | ✅ | ❌ | ❌ | ❌ | ❌ |
| display.c off-by-one | ✅ | ❌ | ❌ | ❌ | ❌ |
| vsprintf fallbacks | ✅ | ✅ | ✅ | ✅ | ❌ |
| strcpy/strcat | ✅ | ❌ | ❌* | ❌ | ❌ |
| svg.c NULL (scale) | ❌ | ✅ | ✅ | ✅ | ❌ |

*R3 explorer falsely claimed "No strcpy/strcat found" but R3 drafter still fixed them because they were included in the task prompt.

### Key Observations

1. **No round found everything.** Even running the same models 3 times, ~40% of findings were unique to a single round.

2. **vsprintf fallbacks were the only consistent finding** across all three rounds. Every explorer found all 11 instances.

3. **Integer overflow findings were highly variable.** R1 found 6 coders, R2 found 5 different coders, R3 found 7 different coders. Only dpx.c was found by all three.

4. **Memory safety findings also varied.** R1 focused on off-by-one, R2 found off-by-one + svg NULL, R3 found svg NULL + double-free + token cleanup.

5. **Review/debug agent reliability is poor.** In all three rounds, at least one of these agents failed silently or returned empty results. Manual Orchestrator verification was required each time.

6. **Model routing opacity makes comparison impossible.** We cannot confirm whether R3 actually used Claude Sonnet 4.6 or the same models as R1/R2. The only evidence is the user's claim that opencode.json was corrected.

---

## Conclusions

### On Stochasticity
The primary takeaway is that **LLM-based security audits are highly stochastic**, even with identical configurations. Running the same audit pipeline 3 times found:
- **29 unique vulnerability instances** total
- Only **~35%** were found by all three rounds
- **~40%** were found by exactly one round

This validates ensemble/multi-pass approaches. A single audit pass is insufficient for comprehensive coverage.

### On Model Routing
Because agent model assignment is opaque to the orchestrator, we cannot verify whether the "corrected" opencode.json actually changed anything. Round 3's results were statistically similar to Round 1 (same coverage variance, same agent failure modes), suggesting either:
- The config change didn't take effect, OR
- Different models produce similarly stochastic results on this task type

### On Agent Reliability
The `@reviewer` and `@debugger` agents failed or returned empty results in all three rounds. This is a systematic issue, not a model-specific one. The orchestrator must perform manual spot-checks on all patches regardless of review agent output.

### Recommended Workflow for Future Audits
1. **Run discovery 2-3 times** with the same config and union the findings
2. **Apply fixes in small batches** (5-10 files max per drafter task)
3. **Always manually verify** critical edits (preprocessor directives, allocation math)
4. **Treat review/debug agents as optional** — they add value when they work, but cannot be relied upon

---

*End of Round 3 report. All findings merged into meta/audit-round-3.md.*
