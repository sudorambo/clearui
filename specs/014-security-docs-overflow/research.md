# Research: Security and Robustness Hardening

**Branch**: `014-security-docs-overflow` | **Date**: 2026-03-07

## Research Tasks

### R1: How to address frame_printf format-string and unbounded allocation

**Decision**: (1) Document in API comments (and optionally public docs) that `fmt` MUST be application-controlled, not user input. (2) Optionally enforce a maximum formatted length (e.g. 64K): if `vsnprintf(NULL, 0, fmt, ap)` returns a value greater than the cap (or negative), return "" and do not allocate. Implementation choice: document-only for minimal scope, or document + cap for defense-in-depth.

**Rationale**: Format-string attacks and DoS via huge allocation are mitigated by contract documentation; a cap adds a hard guarantee without changing the API signature.

**Alternatives considered**:
- No documentation: Rejected — leaves misuse likely.
- Remove `cui_frame_printf`: Rejected — useful API; documenting and optionally capping is sufficient.
- Require a separate “safe” API with length limit: Possible future work; not in scope for this feature.

### R2: How to address utf8_next buffer overread

**Decision**: Document in source and (if present) API docs that the UTF-8 pointer passed to `cui_font_measure` (and thus to internal `utf8_next`) MUST be a valid C string (NUL-terminated). For untrusted input, a length-based API or internal length parameter can be added in a future change; this feature does not add new API.

**Rationale**: Callers currently assume C strings; making the contract explicit prevents misuse. Length-based API is a natural follow-up if untrusted text is required.

**Alternatives considered**:
- Add `cui_font_measure_n(..., const char *utf8, size_t len)` now: Deferred — documentation-first keeps scope small.
- Bounds-check inside `utf8_next` with no length: Impossible without a length or sentinel; documenting the sentinel (NUL) is the minimal fix.

### R3: How to prevent integer overflow in arena and frame allocator grow

**Decision**: Before each `new_cap *= 2` (or equivalent), check `new_cap <= SIZE_MAX / 2`. If the check fails, do not multiply; treat as allocation failure (return NULL from alloc path) or cap at a maximum allowed capacity. Prefer “fail” to avoid silently capping at an arbitrary value.

**Rationale**: Prevents undefined behavior and potential buffer overflow from wraparound. Single comparison per grow step; negligible cost.

**Alternatives considered**:
- Ignore overflow on the basis that “cap will never be that large”: Rejected — defense-in-depth and sanitizer-clean.
- Use a fixed maximum capacity instead of doubling: Rejected — would change growth semantics; overflow check preserves current behavior for all realistic sizes.

### R4: How to document vault key contract

**Decision**: Add comments in the header (or vault API documentation) stating that the `key` parameter to `cui_vault_get`, `cui_state`, and any other vault API taking a key MUST be a valid C string (NUL-terminated). No new API in this feature; a length-bounded variant can be considered later for untrusted keys.

**Rationale**: `dup_str(key)` uses `strlen(key)`; documenting the requirement is the minimal and necessary step.

**Alternatives considered**:
- Add `cui_vault_get_n(ctx, key, key_len, size)` now: Deferred — documentation-first.
- No documentation: Rejected — UB for non–NUL-terminated keys should be explicit in the contract.

### R5: Bounded string copy in test_vault.c

**Decision**: Replace the `strcpy` usage with `snprintf(str, sizeof(str), "%s", "hello")` (or equivalent bounded copy). Keeps the test safe if buffer size or source string is changed later.

**Rationale**: Low risk, high maintainability; aligns with “no unbounded strcpy” best practice in tests.

**Alternatives considered**:
- Leave as strcpy: Rejected — brittle; one-line change improves hygiene.
- Use strncpy: Acceptable but snprintf is consistent with other bounded-format usage and avoids padding/termination nuances of strncpy.
