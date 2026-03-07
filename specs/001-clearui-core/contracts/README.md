# Contracts: ClearUI Core

Contracts define the external interfaces of the library: what the application implements (platform, optional RDI) and what the library guarantees (public API, RDI interface).

- **public-api.md** — Public `cui_*` API surface (categories and invariants).
- **rdi-interface.md** — Render Driver Interface: what ClearUI expects from a render backend.
- **platform-interface.md** — Platform adapter: windowing, input, clipboard, surface.

Implementations (e.g. SDL3 adapter, Vulkan RDI) must satisfy these contracts.
