# Feature Specification: Robustness & Limits (0.9.0)

**Version**: 0.9.0  
**Branch**: `022-robustness-limits-0-9-0`

## Goal

Harden the library for real-world use: replace or augment silent truncation with opt-in error reporting or debug assertions, add CUI_DEBUG checks for misuse, harden UTF-8 handling, add fuzz and stress tests, and memory-leak CI.

## User Stories

1. **US1** As a developer, I want parent stack overflow, focusable/a11y overflow, and widget ID truncation to be detectable (opt-in error callback or return code, or assert in debug) so I can fix misuse instead of silent truncation.
2. **US2** As a developer, I want CUI_DEBUG assertions for common misuse (unbalanced push/pop, etc.) so bugs are caught in development.
3. **US3** As a maintainer, I want `utf8_next` audited and hardened for malformed input (overlong sequences, surrogates) so untrusted or malformed UTF-8 does not cause undefined behavior or overread.
4. **US4** As a maintainer, I want fuzz tests for the UTF-8 decoder, vault hash table, and frame allocator so regressions are caught.
5. **US5** As a maintainer, I want stress tests (1000+ widgets, deep nesting, rapid state churn) and memory-leak CI (Valgrind / LeakSanitizer) so the library is validated under load.

## Acceptance Criteria

- Parent stack overflow: return error or assert in debug (opt-in behavior); document in API.
- Focusable/a11y overflow: warn or grow dynamically; document behavior.
- Widget ID truncation: warn in debug builds when ID is truncated.
- CUI_DEBUG: when defined, assert on unbalanced push/pop and other common misuse.
- utf8_next: reject or safely handle overlong sequences, surrogates; document contract; no overread on truncated input where feasible.
- Fuzz: at least one fuzz target for UTF-8 decoder (and optionally vault, frame allocator); run in CI or as a separate job.
- Stress test: test with 1000+ widgets per frame, deep nesting, rapid state churn; no crash, acceptable performance.
- Memory leak: CI step with Valgrind or LeakSanitizer; no leaks in test suite.

## Out of Scope (0.9.0)

- Changing default release behavior to hard-fail on overflow (keep backward compatibility; opt-in or debug-only).
- Full formal verification.

## Requirements

- **R1** Opt-in or debug-only for new failure modes so existing apps do not break.
- **R2** C11 only; no new mandatory dependencies; fuzz/Valgrind are test-time only.
- **R3** API surface cap (~120 functions) respected; new hooks (e.g. error callback) are minimal and optional.
