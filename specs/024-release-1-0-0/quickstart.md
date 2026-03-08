# Quickstart: 1.0.0 Release

## For the release manager

1. **Run local verification**:
   ```bash
   make clean && make all && make unit-tests && make integration-tests
   make demo && ./demo
   ```

2. **Run sanitizers** (Ubuntu/Linux):
   ```bash
   make asan
   make ubsan
   make leak-check-lsan
   ```

3. **Verify CHANGELOG**: Confirm entries exist for 0.2.0, 0.3.0, 0.4.0, 0.5.0, 0.6.0, 0.7.0, 0.8.0, 0.9.0, 0.10.0, and 1.0.0.

4. **Verify README**: Confirm API surface count, docs links, freeze statement, and project structure are current.

5. **Verify API docs**: Confirm `docs/API.md` and `docs/MIGRATION.md` are up to date.

6. **Version bump**: Set `CUI_VERSION_MAJOR=1, MINOR=0, PATCH=0` in `include/clearui.h`. Verify `cui_version_string()` returns `"1.0.0"`.

7. **CI**: Push to branch, verify all CI jobs pass.

8. **Merge and tag**:
   ```bash
   git checkout main && git merge 024-release-1-0-0
   git tag v1.0.0
   git push origin main --tags
   ```

9. **GitHub release**:
   ```bash
   gh release create v1.0.0 --title "ClearUI 1.0.0" --notes "See CHANGELOG.md for details."
   ```

## For users

- Download the release or clone at tag `v1.0.0`.
- Follow the README Hello World or Counter example.
- Build with `make all` or include `clearui.h` + amalgamated source.
- See `docs/API.md` for the full API reference and `docs/MIGRATION.md` for upgrade notes.
