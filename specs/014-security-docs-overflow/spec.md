# Feature Specification: Security and Robustness Hardening (Remaining Recommendations)

**Feature Branch**: `014-security-docs-overflow`  
**Created**: 2026-03-07  
**Status**: Draft  
**Input**: Remaining recommendations from deep memory/security review — documentation and optional overflow/length caps

## User Scenarios & Testing *(mandatory)*

### User Story 1 - Document frame_printf Format and Allocation Contract (Priority: P1)

A developer or integrator must know that `cui_frame_printf(ctx, fmt, ...)` requires `fmt` to be application-controlled (not user input) to avoid format-string risk and unbounded allocation (DoS). Optionally, the implementation may cap the formatted length (e.g. max 64K) and fail or truncate when exceeded.

**Why this priority**: Medium-severity finding; undocumented contract can lead to misuse when user-controlled strings are passed as `fmt`.

**Independent Test**: Documentation is present in API comments and/or public docs; optional: unit test that passing a format yielding >64K length returns empty or truncated result per design.

**Acceptance Scenarios**:

1. **Given** public API docs or header comments, **When** a developer looks up `cui_frame_printf`, **Then** they see that `fmt` must be application-controlled and (if implemented) any length cap.
2. **Given** optional length cap is implemented, **When** `vsnprintf(NULL, 0, fmt, ap)` returns a value greater than the cap, **Then** the function returns "" or a truncated buffer per design (and does not allocate unbounded memory).

---

### User Story 2 - Document UTF-8 String Contract for Font/Measure APIs (Priority: P1)

A developer calling `cui_font_measure(..., utf8, ...)` or any API that ultimately uses `utf8_next` must know that `utf8` must be NUL-terminated. For untrusted input, a length-bounded API (or internal bounds checks) would be safer; this feature focuses on documentation first.

**Why this priority**: Prevents buffer overread if non–NUL-terminated or truncated buffers are passed.

**Independent Test**: Documentation states the NUL-termination requirement; code comments at `utf8_next` and call sites reference it.

**Acceptance Scenarios**:

1. **Given** font/atlas API docs or header/source comments, **When** a developer looks up `cui_font_measure` or UTF-8 handling, **Then** they see that the UTF-8 pointer must be a valid C string (NUL-terminated).
2. **Given** future work for untrusted input, **When** a length parameter is added, **Then** `utf8_next` (or a variant) can bounds-check against that length.

---

### User Story 3 - Harden Arena and Frame Allocator Growth Against Overflow (Priority: P2)

When arena or frame allocator doubles capacity in a loop (`new_cap *= 2`), the implementation must avoid integer overflow (e.g. check `new_cap <= SIZE_MAX / 2` before doubling) and fail or cap allocation.

**Why this priority**: Low-severity but correct hardening; prevents undefined behavior on extreme sizes.

**Independent Test**: Unit test or code review confirms overflow check before multiplication; growth fails gracefully when cap would overflow.

**Acceptance Scenarios**:

1. **Given** `cui_arena` or frame allocator grow path, **When** doubling would exceed `SIZE_MAX`, **Then** the code does not perform the multiply (or caps) and returns NULL or fails the allocation.
2. **Given** existing tests, **When** run after changes, **Then** no regression in normal growth behavior.

---

### User Story 4 - Document Vault Key String Contract (Priority: P2)

A developer using `cui_vault_get(ctx, key, size)` or any vault API that takes a key must know that `key` must be a valid C string (NUL-terminated). Optionally, a length-bounded variant could be considered for untrusted keys.

**Why this priority**: Low severity; `dup_str(key)` uses `strlen(key)` — undefined behavior if not NUL-terminated.

**Independent Test**: Documentation in header or public docs states that `key` must be NUL-terminated.

**Acceptance Scenarios**:

1. **Given** vault API documentation, **When** a developer looks up `cui_vault_get` or `cui_state`, **Then** they see that `key` must be a valid C string.
2. **Given** future work, **When** untrusted keys are a requirement, **Then** a length-taking API can be added without changing the existing contract.

---

### User Story 5 - Use Bounded String Copy in test_vault (Priority: P3)

The unit test in `tests/unit/test_vault.c` that uses `strcpy` should use a bounded alternative (e.g. `snprintf(str, sizeof(str), "%s", "hello")`) so the test remains safe if the buffer size or source changes.

**Why this priority**: Low; current use is safe (fixed literal) but brittle for maintainability.

**Independent Test**: Grep or test run confirms no `strcpy` in test_vault (or only in a commented example); build and unit tests pass.

**Acceptance Scenarios**:

1. **Given** `test_vault.c`, **When** building and running unit tests, **Then** no `strcpy` is used for the vault key/buffer test (or equivalent bounded API is used).
2. **Given** the change, **When** the test runs, **Then** behavior is unchanged (same assertions pass).

---

### Edge Cases

- What happens when `cui_frame_printf` is called with a format that produces a negative length from `vsnprintf`? (Already handled: `n < 0` returns "".)
- What happens when arena or frame allocator is at or near `SIZE_MAX/2`? Growth must not overflow; allocation fails or is capped.
- What happens when vault is used with a key that is not NUL-terminated? Documented as undefined behavior; caller responsibility.

## Requirements *(mandatory)*

### Functional Requirements

- **FR-001**: The contract for `cui_frame_printf(ctx, fmt, ...)` MUST be documented so that `fmt` is application-controlled (not user input). Optionally, a maximum formatted length (e.g. 64K) MAY be enforced and result in empty or truncated return.
- **FR-002**: The contract for UTF-8 parameters to `cui_font_measure` and internal `utf8_next` MUST be documented: the UTF-8 buffer MUST be NUL-terminated.
- **FR-003**: Arena and frame allocator grow logic MUST check for integer overflow before doubling capacity (e.g. `new_cap <= SIZE_MAX / 2`) and MUST NOT perform multiplication that would overflow; allocation MUST fail or cap instead.
- **FR-004**: The contract for vault key parameters (e.g. `cui_vault_get`, `cui_state`) MUST be documented: `key` MUST be a valid C string (NUL-terminated).
- **FR-005**: The unit test in `tests/unit/test_vault.c` MUST use a bounded string copy (e.g. `snprintf`) instead of `strcpy` for the buffer/key test, where applicable.

### Key Entities

- **Documentation**: API comments (header/source), and optionally a short “Security and robustness” section in the project docs, describing format-string, UTF-8, vault key, and allocation contracts.
- **Arena / Frame allocator**: Internal grow logic; no new public types.
- **Vault / frame_printf**: Existing APIs with clarified contracts; no new public types unless a future length-bounded API is added (out of scope for minimal implementation).

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: All five areas (frame_printf, utf8, arena, frame_alloc, vault key) have explicit documentation (comments or docs) stating the contract.
- **SC-002**: Arena and frame allocator do not overflow on grow; either tests or code review confirm overflow checks.
- **SC-003**: test_vault.c uses no unbounded `strcpy` for the relevant test buffer; unit tests pass.
- **SC-004**: No new security or memory issues introduced; existing tests and (if run) sanitizers pass.
