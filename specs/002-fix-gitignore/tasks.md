# Tasks: Fix .gitignore and Remove Tracked Build Artifacts

**Input**: Design documents from `/specs/002-fix-gitignore/`
**Prerequisites**: plan.md, spec.md, research.md, quickstart.md

**Tests**: Not requested. Build verification via `make` is the acceptance criterion.

**Organization**: Single atomic fix with 4 requirements (R1–R4). All tasks are sequential since they operate on the same files/index.

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no dependencies)
- **[Story]**: Which requirement this task fulfills
- Include exact file paths in descriptions

---

## Phase 1: Update .gitignore (R1)

**Purpose**: Add missing ignore rules for test and example binaries

- [x] T001 [R1] Add test binary ignore rule `/test_*` to `.gitignore` under the "Build artifacts (C)" section
- [x] T002 [P] [R1] Add example binary ignore rules `/hello` and `/counter` to `.gitignore` under the "Build artifacts (C)" section

**Checkpoint**: `.gitignore` now covers all Makefile outputs (test binaries + example binaries + existing `*.o` rules)

---

## Phase 2: Remove Tracked Binaries from Git Index (R2, R3)

**Purpose**: Untrack the 6 test binaries without deleting them from disk

**⚠️ CRITICAL**: Must use `git rm --cached` (not `git rm`) to preserve local files

- [x] T003 [R2] Run `git rm --cached test_arena test_counter test_hello test_layout test_rdi_platform test_vault` to remove tracked binaries from index
- [x] T004 [R3] Run `git ls-files | grep -vE '^\.(git|specify|cursor|github)' | xargs file 2>/dev/null | grep -i 'executable\|ELF\|Mach-O'` to audit for any other tracked binaries

**Checkpoint**: `git ls-files 'test_*'` returns empty. No other binary artifacts are tracked.

---

## Phase 3: Verification (R4)

**Purpose**: Confirm build and ignore rules work correctly after the change

- [x] T005 [R4] Run `make clean && make all && make unit-tests && make integration-tests` to verify build still passes
- [x] T006 [R4] Run `git status` and confirm test binaries do NOT appear as untracked files
- [x] T007 [R4] Run `git ls-files 'test_*'` and confirm empty output

**Checkpoint**: All acceptance criteria from spec.md verified

---

## Dependencies & Execution Order

### Phase Dependencies

- **Phase 1 (Update .gitignore)**: No dependencies — start immediately
- **Phase 2 (Remove tracked binaries)**: Depends on Phase 1 (ignore rules must be in place before removing from index, otherwise `git status` would show them as untracked)
- **Phase 3 (Verification)**: Depends on Phase 2 completion

### Within Phase 1

- T001 and T002 are marked [P] — they edit the same file but different sections, so they can be done as a single edit

### Parallel Opportunities

- T001 + T002 can be done in a single edit pass (same file, different lines)
- T005, T006, T007 are sequential verification steps but can be run in a single shell session

---

## Parallel Example

```bash
# Phase 1: Single edit to .gitignore (T001 + T002 together)
# Add under "Build artifacts (C)":
#   /test_*
#   /hello
#   /counter

# Phase 2: Single command (T003)
git rm --cached test_arena test_counter test_hello test_layout test_rdi_platform test_vault

# Phase 3: Verify (T005 + T006 + T007 in sequence)
make clean && make all && make unit-tests && make integration-tests
git ls-files 'test_*'  # expect empty
git status              # expect clean (no untracked test binaries)
```

---

## Implementation Strategy

### Single-Pass Execution

This is an atomic fix — all 3 phases should be completed in a single session:

1. Edit `.gitignore` (T001 + T002)
2. Remove tracked binaries from index (T003)
3. Audit for other artifacts (T004)
4. Verify build and ignore rules (T005 + T006 + T007)
5. Commit all changes together

### Commit Guidance

A single commit covering T001–T004 is appropriate. Suggested message:

```
fix: remove tracked test binaries and update .gitignore

Add /test_*, /hello, /counter ignore rules for Makefile build outputs.
Remove 6 tracked ELF binaries from git index (git rm --cached).
```

---

## Notes

- T001 + T002 target the same file (`.gitignore`) but are logically separate rules
- T003 uses `--cached` flag — this is critical to avoid deleting local binaries
- T004 is an audit step; if it finds issues, additional T003-style removals may be needed
- No source code changes in this entire task list
- Total: 7 tasks across 3 phases
