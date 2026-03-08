# Contract: 1.0.0 Release

## Scope

ClearUI 1.0.0 marks the first stable release. The public API (as frozen in 0.10.0) is now the official 1.0 contract.

## Guarantees

- **Stability**: No breaking public API changes until 2.0. Additive changes only (new functions, new optional fields in opts structs).
- **Semantic versioning**: MAJOR bumped for breaking changes, MINOR for additive features, PATCH for bug fixes.
- **Platform support**: Tier 1: Linux (X11/Wayland via SDL3), macOS, Windows (MSYS2/MinGW). Tier 2 (iOS, Android, Web) not guaranteed for 1.0.
- **Render driver**: Software RDI ships; GPU drivers (Vulkan, Metal, WebGPU) are future additions.

## Release process

1. All CI jobs pass (build, tests, sanitizers, leak-check).
2. Version set to 1.0.0 in `include/clearui.h`.
3. CHANGELOG.md updated with 1.0.0 entry.
4. Merge to main.
5. Tag `v1.0.0`.
6. Create GitHub release with changelog body.
