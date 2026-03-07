# Research: ClearUI Core (Phase 0)

**Feature**: 001-clearui-core  
**Date**: 2026-03-06

Consolidation of technical decisions and alternatives considered for the ClearUI implementation. All NEEDS CLARIFICATION items from the plan are resolved below.

---

## 1. Declarative Immediate (DI) Model

**Decision**: Developer writes a function that declares the UI top-to-bottom each frame (immediate-mode style). Internally, ClearUI keeps a lightweight retained tree and diffs it against the declared output. The retained tree is never exposed.

**Rationale**: Preserves the simplicity and readability of immediate-mode code while enabling efficient layout, animation, accessibility, and hit-testing. Aligns with React virtual DOM and SwiftUI declarative model.

**Alternatives considered**:
- Pure immediate mode (e.g. Nuklear): Rejected—no efficient a11y tree, no animation, wasteful full rebuild every frame.
- Pure retained (GTK/Qt-style): Rejected—callback/signal boilerplate and state synchronization burden.

---

## 2. Memory: Arena + Frame Bump + Vault

**Decision**: (1) Arena for UI tree—reset each `cui_begin_frame()`, no user `free()`. (2) Per-frame bump allocator for transient data (e.g. `cui_frame_printf`). (3) Vault: hash map keyed by string IDs for persistent state (`cui_state()`); FNV-1a hash; stale entry pruning after N frames.

**Rationale**: Removes memory anxiety; common path has no manual allocation. Arena size default 4 MB, grow by doubling with diagnostic log.

**Alternatives considered**:
- Per-node malloc/free: Rejected—allocation churn and ownership ambiguity.
- GC or reference counting: Rejected—C99/C11 only, no runtime dependency.

---

## 3. Render Driver Interface (RDI)

**Decision**: Thin struct of function pointers (~15–20 ops): init, texture create/upload, draw commands (quads, rounded rects, lines, text glyphs), scissor, swap/present. Core produces a draw command buffer (flat array of tagged unions); driver batches and submits. No in-tree abstraction over Vulkan/Metal/WebGL—each backend is a separate RDI implementation.

**Rationale**: Keeps core backend-agnostic; allows community drivers (e.g. OpenGL, DX11) without touching core. Software fallback (clearui_rdi_soft) for headless/CI.

**Alternatives considered**:
- Single backend (e.g. Vulkan only): Rejected—macOS requires Metal; Web requires WebGPU.
- Heavy abstraction layer: Rejected—violates clarity and performance goals.

---

## 4. Platform Adapter (Windowing / Input)

**Decision**: `cui_platform` interface (function pointers): window create, event poll, clipboard, cursor, GPU surface creation. ClearUI does not own the event loop; app calls `cui_begin_frame` / `cui_end_frame`. Ship with adapters: SDL3 (recommended), GLFW, native (Win32/Cocoa/X11/Wayland), Emscripten.

**Rationale**: App can use any loop (game loop, server tick, etc.). SDL3 chosen as default for cross-platform and maturity; native adapter for zero-dependency option (built incrementally).

**Alternatives considered**:
- Bundled event loop only: Rejected—reduces flexibility.
- No native adapter: Deferred—SDL3/GLFW first; native later for minimal-deps use case.

---

## 5. Layout: Flexbox-Inspired Single-Pass

**Decision**: CSS Flexbox–style primitives: `cui_row`, `cui_column`, `cui_stack`, `cui_center`, `cui_wrap`. Single layout pass over the tree plus a second pass for flex-grow distribution. Target sub-microsecond for &lt;1k nodes (“where feasible” per constitution: best-effort on Tier 1 platforms in v1; no formal performance gate). `cui_layout` struct: gap, padding, max/min width/height, flex, align, align_y.

**Rationale**: Flexbox is widely understood; one-dimensional layout covers most UI needs. Single pass keeps layout cost predictable.

**Alternatives considered**:
- Grid-only or constraint solver: Rejected—higher complexity; flexbox sufficient for MVP.
- Manual positioning only: Rejected—violates “beautiful by default” and developer ergonomics.

---

## 6. Font Rendering and Text

**Decision**: Default bundled font: Noto Sans (SIL OFL). Rasterization: SDF at build time; optional MSDF for subpixel. Atlas for glyphs; logical pixels in API; platform reports DPI and core scales to physical pixels. Optional HarfBuzz link for complex scripts; Latin/CJK with built-in shaper.

**Rationale**: SDF gives resolution-independent sharp text; single optional dependency keeps default build dependency-free.

**Alternatives considered**:
- Runtime TTF rasterization: Rejected—allocation and complexity; SDF at build time is simpler.
- Mandatory HarfBuzz: Rejected—would break zero-dependency default.

---

## 7. Accessibility

**Decision**: Internal retained tree drives accessibility tree. Every widget has role, label (from text or `cui_aria_label`), state (checked, disabled, etc.), focus order (declaration order; overridable with `cui_tab_index`). Platform layer maps to UI Automation (Windows), NSAccessibility (macOS), AT-SPI2 (Linux). Keyboard: Tab/Shift-Tab, Enter/Space, arrows in composites. Focus indicators meet WCAG 2.1 AA.

**Rationale**: Required for serious adoption; DI model makes a11y tree available without forcing retained-mode API on the user.

**Alternatives considered**:
- A11y as optional add-on: Rejected—constitution requires accessibility first-class.

---

## 8. API Surface and Naming

**Decision**: Prefix `cui_`; verb_noun for actions (`cui_begin_frame`, `cui_push_style`), noun for declarations (`cui_button`, `cui_label`, `cui_row`). Config structs: C99 designated initializers; macro defaults (e.g. `CUI_BUTTON_OPTS_DEFAULT`). Target 60–80 public functions; hard cap 120.

**Rationale**: Readable as pseudocode; designated inits avoid long parameter lists. Cap prevents API creep.

---

## 9. Distribution and Build

**Decision**: Primary: two-file drop-in (clearui.h + clearui.c). Optional: precompiled static libs per platform. One RDI selected at compile time (e.g. `CUI_BACKEND_VULKAN`). No mandatory CMake/pkg-config; `cc main.c clearui.c -lm -o app` for Hello World.

**Rationale**: Zero-to-window in under five minutes; no build system required for first run.

---

## 10. Testing Strategy

**Decision**: Unit tests for arena, vault, layout math, tree diff. Integration tests: RDI (software backend) + platform (SDL3 or stub) + full frame pipeline. Contract tests optional for public API. CI: Tier 1 platforms (Windows, macOS, Linux) on every commit where feasible; Tier 2 on release candidates.

**Rationale**: Software RDI and stub platform allow CI without GPU or display. Unit tests protect core algorithms.

---

## Summary

All technical context fields are resolved. No remaining NEEDS CLARIFICATION. Phase 1 can proceed to data-model, contracts, and quickstart.
