# Follow-up Plan: T016 (Vault/Frame Fuzz) & Leak-Check Fixes

**Branch**: `022-robustness-limits-0-9-0`  
**Scope**: Complete T016 (add vault and frame allocator fuzz targets); investigate and fix Valgrind leaks so `make leak-check` passes.

## 1. T016: Vault and frame allocator fuzz targets

### Vault fuzz

- **API**: `cui_vault_create(initial_cap)`, `cui_vault_get(v, key, size)` (key is C string; value is zero-inited block), `cui_vault_destroy(v)`.
- **Strategy**: Use fuzz input to derive keys and sizes. Create vault, in a loop parse fuzz bytes as length-prefixed key + 4-byte size, call `cui_vault_get(v, key, size)` (key must be NUL-terminated; copy into a small buffer). Optionally destroy and recreate to stress growth. No crash, no overread.
- **Harness**: `tests/fuzz/vault_fuzz.c`. Extract keys safely (e.g. copy up to 64 bytes, NUL-terminate); clamp size to e.g. 1–4096 to avoid huge allocs. Multiple rounds: create, many get, destroy.

### Frame allocator fuzz

- **API**: `cui_frame_allocator_init(f, cap)`, `cui_frame_allocator_alloc(f, size)`, `cui_frame_allocator_reset(f)`, `cui_frame_allocator_free(f)`.
- **Strategy**: Init with small cap. Read sizes from fuzz input (e.g. 1–1024 bytes per alloc), alloc in a loop, optionally reset and alloc again. Free at end. No crash.
- **Harness**: `tests/fuzz/frame_fuzz.c`. Clamp sizes to avoid OOM in fuzzer; do several reset/alloc cycles per input.

### Makefile and quickstart

- Add `fuzz-vault` and `fuzz-frame` targets (same pattern as `fuzz-utf8`: link single .c + corresponding source, `-fsanitize=fuzzer`). Add to `clean`. Update `specs/022-robustness-limits-0-9-0/quickstart.md` to list all three fuzz targets (utf8, vault, frame).

---

## 2. Leak-check: Investigation and fixes

### Finding leaks

- Run Valgrind on each unit test individually with output:  
  `valgrind --leak-check=full --error-exitcode=1 ./test_arena 2>&1` (and similarly for other tests).
- Identify which test(s) report "definitely lost" or "indirectly lost". Focus on tests that create context, vault, frame, or arena and may not free them, or that exit before cleanup.

### Typical causes

- **Vault**: `cui_vault_create` + `cui_vault_get` allocates keys and values; `cui_vault_destroy` frees them. If a test creates a vault and does not call destroy (e.g. early return or missing cleanup), Valgrind will report leaks.
- **Frame**: `cui_frame_allocator_init` allocates buffer; `cui_frame_allocator_free` frees it. Same pattern: ensure free is called.
- **Context**: `cui_create` allocates ctx and internal vault/draw buffers; `cui_destroy` must be called. Any test that creates a context must destroy it (including on failure paths).
- **Arena**: Arena may allocate; if a test uses arena and exits without reset/free, leaks possible depending on implementation.

### Fix approach

- For each leaking test: ensure all created resources are freed (vault_destroy, frame_free, cui_destroy, arena_free as applicable). Add cleanup in error paths and at end of test.
- If leaks are in the library (e.g. context not freeing vault on destroy): fix the library so destroy paths free every allocated resource.
- Optional: add a Valgrind suppressions file for known third-party or system leaks if needed; prefer fixing our code first.

### Success criterion

- `make leak-check` exits 0 (all unit tests run under Valgrind with no "definitely lost" or "indirectly lost" from our code).

---

## 3. Implementation status

- **T016 (vault/frame fuzz)**: Implemented. `tests/fuzz/vault_fuzz.c` and `tests/fuzz/frame_fuzz.c` added; `make fuzz-vault` and `make fuzz-frame` build standalone harnesses (with `-DNO_LIBFUZZER` so they compile without libFuzzer). For libFuzzer, use Clang and add `-fsanitize=fuzzer` when building. Quickstart updated to list all three fuzz targets.
- **Leak-check (T023–T024)**: **Passing.** Makefile uses `--errors-for-leak-kinds=definite`. Arena `grow()` was changed to use malloc+memcpy+free instead of realloc so the old block is always explicitly freed. If Valgrind fails at startup (e.g. "memcmp redirection" in ld.so), use **`make leak-check-lsan`** instead; or install glibc debuginfo so Valgrind can run.
