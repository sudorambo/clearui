<!--
Sync Impact Report
- Version change: (none) → 1.0.0
- Initial ratification; all principles and sections added from ClearUI RFC-0001.
- Modified principles: N/A (initial)
- Added sections: Core Principles (6), Technology & Constraints, Development Workflow, Governance
- Removed sections: N/A
- Templates: plan-template.md (Constitution Check gates to be filled from this file) ✅
- spec-template.md: no changes required ✅
- tasks-template.md: no changes required ✅
- commands: no outdated references ✅
- Follow-up TODOs: None
-->

# ClearUI Constitution

## Core Principles

### I. Clarity Over Cleverness

The API MUST be readable as pseudocode. No macro sorcery, no hidden allocations, no implicit global state. A developer reading ClearUI code for the first time MUST be able to understand what it does without consulting internals.

**Rationale**: Boilerplate fatigue and opaque APIs are primary reasons developers leave C for GUI work. ClearUI code should describe the UI it produces.

### II. Explicit Ownership

Memory management MUST follow the three-tier model: arena for the UI tree, per-frame bump allocator for transient data, and the Vault for persistent application state. The developer MUST NOT call `free()` on any UI node; the arena is reset each frame. Ownership is explicit but not painful—arenas and frame allocators make the common case trivial.

**Rationale**: Memory anxiety in C GUIs is eliminated when lifetimes are unambiguous and the common path requires no manual deallocation.

### III. Zero-to-Window

A new developer MUST be able to download the library, write a `main()` function, compile with a single command, and see a window with a button in under five minutes. No mandatory CMake, pkg-config, or vcpkg. Primary distribution is two-file drop-in (header + amalgamated implementation).

**Rationale**: Build system and dependency friction is a major barrier to adoption; the first run must be trivial.

### IV. C99/C11 Only

The library MUST use only C99/C11. No C++ features, no compiler-specific extensions, no GNU-isms. It MUST compile cleanly under gcc, clang, and MSVC with `-std=c11 -Wall -Wextra -Wpedantic`. The single permitted optional external dependency is HarfBuzz for complex-script shaping; default builds MAY have zero dependencies.

**Rationale**: Portability and FFI compatibility (Rust, Zig, Odin, etc.) require strict standard C and minimal deps.

### V. Performance by Design

Performance is a feature, not an afterthought. The library MUST target 60fps on integrated GPUs for typical application UIs. The rendering pipeline MUST be designed for GPU acceleration from day one (draw command buffer, RDI, batching). Layout MUST complete in sub-microsecond time for trees under 1,000 nodes where feasible.

**Rationale**: C is chosen for control and efficiency; the GUI must not become a bottleneck.

### VI. Beautiful by Default & Accessibility First

The default theme, font rendering (e.g. SDF/MSDF), and layout behavior MUST produce output visually competitive with modern frameworks (e.g. SwiftUI, Material). Ugliness MUST require deliberate effort. Accessibility MUST be first-class: an internal retained tree enables an accessibility tree; platform mapping (UI Automation, NSAccessibility, AT-SPI2) and keyboard navigation with WCAG 2.1 AA–compliant focus indicators are required, not retrofits.

**Rationale**: Aesthetic and inclusive design are non-negotiable for user-facing C GUIs to be taken seriously.

## Technology & Constraints

- **Render Driver Interface (RDI)**: ClearUI MUST target a thin RDI (function-pointer struct); no in-tree abstraction over multiple graphics APIs. Official drivers: Vulkan, Metal, WebGPU; software fallback for headless/testing.
- **Platform adapters**: Window-library agnostic via `cui_platform` interface. Built-in adapters (e.g. SDL3, GLFW, native, Emscripten) MUST not force a single event-loop ownership model; the app owns the loop.
- **API surface**: Public API MUST stay within approximately 60–80 functions for a complete application. If the API grows beyond ~120 functions, the simplicity principle has been violated and the design MUST be revisited.
- **Layered architecture**: Four layers (Widget, Layout, Core, Platform) with strict downward dependency; no layer may call into a layer above it. Application code primarily uses Layout and Widget layers; Core for custom widgets; Platform behind an interface only.

## Development Workflow

- **Constitution Check**: Before Phase 0 research and after Phase 1 design, plans MUST verify compliance with every principle above. Any violation MUST be justified in a Complexity Tracking table or resolved by changing the plan/spec.
- **CI**: Tier 1 platforms (Windows 10+, macOS 12+, Linux X11/Wayland) MUST be tested in CI on every commit where applicable. Tier 2 (iOS, Android, Web) on release candidates.
- **Versioning**: Semantic versioning for the project; breaking API or ABI changes require a MAJOR bump and a migration path.

## Governance

This constitution supersedes ad-hoc practices. All PRs and design reviews MUST verify compliance with these principles. Complexity or exceptions MUST be justified in writing (e.g. in plan Complexity Tracking) and not accepted by default.

**Amendments**: Require a version bump (MAJOR/MINOR/PATCH per semantic rules), updated documentation, and explicit approval. Last amended date MUST be updated.

**Runtime guidance**: For day-to-day implementation, use the feature spec (e.g. `ClearUI_Technical_Specification_RFC-0001.md`) and plans under `specs/`; the constitution is the stability layer those artifacts must not contradict.

**Version**: 1.0.0 | **Ratified**: 2026-03-06 | **Last Amended**: 2026-03-06
