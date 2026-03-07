# Quickstart: Fix .gitignore and Remove Tracked Build Artifacts

**Branch**: `002-fix-gitignore` | **Date**: 2026-03-07

## Implementation Steps

### Step 1: Update `.gitignore`

Add the following rules to `.gitignore` under the existing "Build artifacts (C)" section:

```gitignore
# Test and example binaries (Makefile outputs to repo root)
/test_*
/hello
/counter
```

### Step 2: Remove tracked binaries from git index

```bash
git rm --cached test_arena test_counter test_hello test_layout test_rdi_platform test_vault
```

This removes them from the git index only. The files remain on disk.

### Step 3: Commit

```bash
git add .gitignore
git commit -m "fix: remove tracked test binaries and update .gitignore"
```

## Verification

### Verify binaries are no longer tracked

```bash
git ls-files 'test_*'
# Expected: empty output
```

### Verify build still works

```bash
make clean && make all && make unit-tests && make integration-tests
# Expected: all tests PASS
```

### Verify new builds are ignored

```bash
git status
# Expected: test_arena, test_counter, etc. do NOT appear as untracked
```
