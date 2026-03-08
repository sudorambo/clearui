# Research: Platform Backend (0.7.0)

Decisions and rationale for shipping a real platform adapter and documentation.

## 1. SDL3 vs native adapter first

**Decision**: Prefer implementing the **SDL3 adapter** first.

**Rationale**: SDL3 is cross-platform (Windows, macOS, Linux), has a stable C API, and handles windowing, events, clipboard, cursor, and scale factor in one dependency. One code path serves all Tier 1 platforms. Constitution allows “built-in adapters (e.g. SDL3, GLFW, native)”; SDL3 is the most efficient first step to “at least one real platform adapter.”

**Alternatives considered**:
- **Native (X11/Wayland/Win32/Cocoa)**: Zero extra deps per OS but four code paths and more maintenance; better as a follow-up or community contribution.
- **GLFW**: Lighter than SDL but less uniform for clipboard/cursor/scale; can be added later.

## 2. Hi-DPI scale factor

**Decision**: Expose scale factor via the **platform context** or **window_get_size** companion. If `clearui_platform.h` does not yet have a scale_factor getter, add an optional `float (*scale_factor_get)(cui_platform_ctx *ctx)` to `cui_platform` (NULL = not supported; app sets `cui_config.scale_factor` manually). SDL3 adapter implements it using SDL’s display/window scale APIs.

**Rationale**: ClearUI already has `cui_config.scale_factor`; the adapter should provide it when the platform supports it so the app does not need platform-specific code.

**Alternatives considered**: Document-only (app queries SDL/OS and sets config) — acceptable but less convenient; adding the optional callback keeps the contract in one place.

## 3. Integration test and CI without display

**Decision**: Add an integration test that **opens a real window, runs one frame, closes**. When no display is available (e.g. headless CI), the test **skips** and reports “skipped (no display)” or is excluded from the default `make integration-tests` and run only in a job that has a display or virtual framebuffer.

**Rationale**: Constitution requires CI on Tier 1 platforms; many CI runners are headless. Making the test optional/skippable keeps CI green while still validating the adapter where a display exists.

**Alternatives considered**: Always run and fail in headless CI — would force all contributors to have a display or complex Xvfb setup; skip is simpler. Virtual framebuffer (Xvfb) can be a separate CI step if desired later.

## 4. Optional dependency and build

**Decision**: SDL3 is an **optional** dependency. Build system (Makefile) builds `cui_platform_sdl3.c` only when SDL3 is available (e.g. `sdl3-config` or pkg-config); otherwise only the stub is built. Default `make all` can remain zero-deps (stub only); `make all WITH_SDL3=1` or similar enables the SDL3 adapter. Document in README.

**Rationale**: Zero-to-window with SDL3 installed; zero-deps headless/tests without it. Aligns with “no mandatory CMake/pkg-config” and “optional external dependency” from constitution.

**Alternatives considered**: Always require SDL3 — would violate zero-deps default. Submodule/vendor SDL3 — possible but heavy; system or user-installed SDL3 is simpler for 0.7.0.

## 5. “Bring your own platform” documentation

**Decision**: Add a **dedicated section** (README or `docs/platform-adapter.md`) that lists the `cui_platform` callbacks, which are required vs optional, how to allocate/free `cui_platform_ctx`, and how to call `cui_set_platform(ctx, &my_platform, my_ctx)`. Reference the existing contract in `specs/001-clearui-core/contracts/platform-interface.md`.

**Rationale**: FR-004 and User Story 3 require that developers can implement a custom adapter without forking; a single place for “how to implement” avoids scattering and matches constitution’s “window-library agnostic” and “app owns the loop.”
