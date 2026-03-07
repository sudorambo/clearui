# Research: Fix .gitignore and Remove Tracked Build Artifacts

**Branch**: `002-fix-gitignore` | **Date**: 2026-03-07

## Investigation Summary

### What is tracked that shouldn't be?

| File | Type | Size | Tracked? | Should be? |
|------|------|------|----------|------------|
| `test_arena` | ELF 64-bit executable | build artifact | Yes | No |
| `test_counter` | ELF 64-bit executable | build artifact | Yes | No |
| `test_hello` | ELF 64-bit executable | build artifact | Yes | No |
| `test_layout` | ELF 64-bit executable | build artifact | Yes | No |
| `test_rdi_platform` | ELF 64-bit executable | build artifact | Yes | No |
| `test_vault` | ELF 64-bit executable | build artifact | Yes | No |
| `src/**/*.o` | Object files | build artifact | No | No (already ignored) |

### What does `.gitignore` already cover?

The existing `.gitignore` correctly handles:
- `*.o`, `*.a`, `*.so`, `*.dll`, `*.exe`, `*.out`, `*.dylib` (compiled artifacts)
- `build/`, `bin/`, `obj/`, `out/` (build directories)
- IDE/editor files, logs, env

### What is missing from `.gitignore`?

The Makefile's `clean` target reveals all potential build outputs:

```
rm -f $(OBJS) build/*.o build/*.a hello counter test_arena test_vault test_layout test_hello test_counter test_rdi_platform
```

Missing ignore rules for:
1. Test binaries: `test_arena`, `test_vault`, `test_layout`, `test_hello`, `test_counter`, `test_rdi_platform`
2. Example binaries: `hello`, `counter` (not currently present but listed in `make clean`)

### Decision: Ignore pattern strategy

- **Decision**: Use `/test_*` (root-anchored glob) for test binaries and explicit `/hello` + `/counter` for example binaries.
- **Rationale**: `/test_*` with the leading `/` only matches at the repo root, so it won't interfere with `tests/` directory contents. The Makefile always outputs test binaries to the root. Explicit names for `hello` and `counter` avoid overly broad patterns.
- **Alternatives considered**:
  - Listing all 6 test binary names explicitly: rejected because new test targets would be missed.
  - Broad `*` patterns: rejected because they would suppress useful files.
  - Moving output to `build/` (already ignored): out of scope for this fix; noted as future improvement.

### Decision: Removal method

- **Decision**: Use `git rm --cached` to remove tracked binaries from the index without deleting them from disk.
- **Rationale**: The binaries are still useful locally for development. `--cached` only affects the git index.
- **Alternatives considered**:
  - `git rm` (deletes from disk too): rejected; developer may want them locally.
  - `git filter-branch` / `git filter-repo` to rewrite history: rejected; only 3 commits exist and the binaries are small. Not worth the complexity or force-push.

### Future considerations

- The Makefile outputs all binaries to the repo root. A `BUILD_DIR` variable would be cleaner but is out of scope.
- If new test targets are added, the `/test_*` pattern will catch them automatically.
