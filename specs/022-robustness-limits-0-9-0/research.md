# Research: Robustness & Limits (0.9.0)

## R1: Opt-in error reporting vs return codes

**Decision**: Prefer an optional **error callback** on the context (e.g. set via `cui_config` or `cui_set_error_callback`). When a limit is hit (parent stack, focusable, a11y, ID truncation), call the callback with an error code and context (e.g. limit type). In **CUI_DEBUG** builds, also assert on parent stack overflow so misuse is caught immediately. Return codes on every push/begin would enlarge the API and break existing code; a single callback keeps the API small and opt-in.

**Rationale**: Callback is one place to log or break; apps that do not set it keep current silent behavior. Constitution API cap is respected.

**Alternatives considered**: Return codes on `cui_begin`/`cui_button` etc. (too invasive). Abort in release (breaks existing apps; rejected).

---

## R2: Focusable / a11y overflow: warn vs grow

**Decision**: For 0.9.0, **warn** via the same error callback when focusable or a11y list is full; do not grow dynamically. Dynamic growth would require allocator changes and could hide bugs. Document that increasing `CUI_FOCUSABLE_MAX` / `CUI_A11Y_MAX` is the remedy for large UIs.

**Rationale**: Keeps fixed limits predictable and avoids allocation in hot path. Callback allows apps to log and optionally reduce widget count or increase build-time constants.

**Alternatives considered**: Dynamic growth (rejected for 0.9.0 to avoid allocator and API churn).

---

## R3: CUI_DEBUG assertions

**Decision**: When `CUI_DEBUG` is defined (e.g. `-DCUI_DEBUG`), add assertions for: (1) unbalanced push/pop (e.g. `parent_top == 0` at end of frame or `cui_end` called without matching begin); (2) parent stack overflow (before writing to stack); (3) optionally widget ID length truncation. Do not assert in release so production behavior is unchanged.

**Rationale**: Standard practice for C libraries; zero cost in release; helps developers catch bugs early.

**Alternatives considered**: Always assert (rejected; would break apps that rely on silent truncation until they opt in).

---

## R4: utf8_next robustness (overlong, surrogates, truncated)

**Decision**: **Audit and harden** `utf8_next`: (1) Reject overlong encodings (e.g. 0xC0 0x80 for U+0000). (2) Reject or replace UTF-16 surrogates (U+D800–U+DFFF) if they appear in UTF-8. (3) When a multi-byte sequence is truncated (no NUL, not enough bytes), do not advance past buffer; return 0 and do not read beyond known-safe length. This may require a length-aware variant or documenting that callers must pass NUL-terminated strings and we only validate well-formedness. Add a shared helper (e.g. in a small internal header or duplicated in atlas + rdi) so both font and soft RDI use the same rules. Document the contract (valid UTF-8, NUL-terminated or length-bound).

**Rationale**: Prevents undefined behavior and overread on malformed or truncated input; keeps single-file or minimal deps.

**Alternatives considered**: Leave as-is and only document (rejected; spec asks for audit and hardening). Full ICU (rejected; dependency and API surface).

---

## R5: Fuzz and stress tests, leak CI

**Decision**: **Fuzz**: Add one or more fuzz targets (e.g. `tests/fuzz/utf8_fuzz.c` that feeds bytes to a UTF-8 decoder; optionally vault and frame allocator). Build with libFuzzer or AFL when requested (e.g. `make fuzz` or CI job). **Stress**: Add a test that builds a frame with 1000+ widgets and/or deep nesting (e.g. 17 levels) and runs multiple frames; assert no crash and optionally check timing. **Leak**: Add a CI job (e.g. `make leak-check` or `valgrind --leak-check=full make unit-tests`) or LeakSanitizer build; fix any leaks in the test suite so the job is green.

**Rationale**: Catches regressions and validates robustness; leak CI is standard for C libraries.

**Alternatives considered**: Skip fuzz (spec requires it). Run Valgrind only on one platform (acceptable; document in CI).
