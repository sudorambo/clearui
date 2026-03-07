TECHNICAL SPECIFICATION

**ClearUI**

*A Modern GUI Library for C99/C11*

Making developers fall in love with low-level C again.

RFC-0001  •  Version 1.0  •  Draft

Status: Proposal  •  March 2026

Category: Systems / GUI / Architecture

Etienne Schoeman

github: sudorambo

email: es3178@gmail.com

**TABLE OF CONTENTS**

**1  Abstract & Motivation**

**2  Problem Space & Philosophy**

**2.1  Pain Points in C GUI Development**

**2.2  Why Developers Flee C**

**2.3  Core Philosophy: The ClearUI Manifesto**

**2.4  Immediate Mode vs. Retained Mode: A Hybrid Resolution**

**3  Core Architecture & Rendering**

**3.1  Layered Architecture Overview**

**3.2  Rendering Backend Strategy**

**3.3  Font Rendering, Text Shaping & High-DPI**

**4  Memory & State Management**

**4.1  The Arena-Based Ownership Model**

**4.2  Frame Allocator & Transient State**

**4.3  Persistent State: The Vault**

**4.4  Memory Lifecycle Summary**

**5  API Design & Code Examples**

**5.1  Naming Conventions & Ergonomics**

**5.2  Hello World**

**5.3  Counter Application with State**

**5.4  Complex Layout & Event Handling**

**5.5  Custom Drawing & Canvas**

**6  Layout System**

**6.1  Flexbox-Inspired Layout Engine**

**6.2  Constraint Shorthands**

**7  Ecosystem, Tooling & Integration**

**7.1  Distribution Model**

**7.2  Windowing & Event Loop Strategy**

**7.3  Build Integration**

**7.4  Hot Reload & Developer Experience**

**8  Accessibility**

**9  Platform Matrix & Requirements**

**10  Risks, Trade-offs & Open Questions**

**11  Conclusion**

# **1. Abstract & Motivation**

ClearUI is a proposed graphical user interface library written in pure C99/C11, designed to provide a joyful, ergonomic development experience comparable to modern frameworks like SwiftUI, Flutter, or React—while preserving the performance characteristics, transparency, and simplicity that make C the enduring foundation of systems programming.

This specification defines the architecture, memory model, rendering strategy, API surface, and ecosystem integration for ClearUI. It is intended as a living document to guide implementation and solicit community feedback before any code is written.

The core thesis is simple: the reason developers abandon C for GUI work is not a limitation of the language itself, but a failure of existing libraries to respect the developer’s time, mental model, and aesthetic sensibility. ClearUI aims to prove that a well-designed C API can be as pleasant to use as any high-level framework, without sacrificing the mechanical sympathy that drew us to C in the first place.

# **2. Problem Space & Philosophy**

## **2.1 Pain Points in C GUI Development**

The landscape of C-compatible GUI libraries is rich in history but poor in developer ergonomics. A candid survey of the dominant options reveals systemic issues that push developers toward higher-level alternatives.

**GTK (GIMP Toolkit)**

GTK is the most fully-featured C GUI toolkit available, powering the GNOME desktop environment. However, it carries significant burdens for application developers: GObject, GTK’s simulated object system, requires developers to write extensive boilerplate involving manual type registration, casting macros (G\_TYPE\_CHECK\_INSTANCE\_CAST), and a property system that feels like writing a runtime in C. Cross-platform support on Windows and macOS is a perpetual second-class experience, with rendering inconsistencies, theming problems, and complex build dependencies (pkg-config chains, MSYS2 on Windows). GTK 4 introduced a scene-graph renderer and removed raw drawing access, making some previously trivial tasks architecturally complex.

**Dear ImGui**

Dear ImGui is a masterpiece of immediate-mode design, beloved in game development and tooling. Its limitations are well-understood but significant: it is written in C++, meaning C consumers must use wrapper bindings (cimgui) which lag behind the main branch and lose API ergonomics. It is designed for developer tools, not end-user applications—it lacks proper text selection, accessibility, system clipboard integration (beyond basics), and native-looking widgets. Layout is manual and imperative, with no flexbox-like system. Accessibility is essentially nonexistent.

**Nuklear**

Nuklear is a single-header ANSI C library that demonstrates that immediate-mode GUI in pure C is achievable. However, its API requires the developer to pass a context pointer through every single call, leading to deeply nested, verbose code. Its rendering is extremely basic, with no subpixel font rendering or text shaping support. It has no layout engine beyond simple rows and columns, and the project has limited maintenance momentum.

**Raylib**

Raylib excels as a multimedia and game programming library with a clean C API. Its GUI module (raygui) is functional but minimal—it is best understood as a game-UI toolkit, not an application-UI framework. It lacks layout management, complex widget composition, accessibility, and advanced text handling.

## **2.2 Why Developers Flee C**

Analyzing the migration patterns of developers away from C for GUI work reveals several root causes that this specification must address.

- Boilerplate fatigue: Existing C GUI code tends to be extremely verbose, with signal/callback registration, manual casting, and repetitive initialization sequences that obscure the actual intent of the UI.

- Memory anxiety: Without RAII or garbage collection, developers fear memory leaks in complex widget hierarchies. Existing libraries do not provide a clear, comforting answer to the question: “Who owns this widget’s memory, and when is it freed?”

- Build system hell: Linking against GTK or Qt from a fresh project often takes longer than writing the first hundred lines of application code. Dependency chains, pkg-config, and platform-specific build steps are a major barrier to entry.

- Aesthetic gap: C GUIs often look dated or non-native. The visual quality gap between a Flutter app and a Nuklear app is vast, and developers building user-facing products cannot accept this.

- Ecosystem inertia: Web technologies (Electron, Tauri) offer NPM’s vast ecosystem, hot-reload, CSS-based styling, and familiar paradigms. The switching cost to learn a C GUI library with minimal documentation feels unjustifiable.

## **2.3 Core Philosophy: The ClearUI Manifesto**

ClearUI is built on a set of non-negotiable principles that inform every architectural decision.

1. Clarity over cleverness. The API should be readable as pseudocode. No macro sorcery, no hidden allocations, no implicit global state. A developer reading ClearUI code for the first time should understand what it does.

2. Ownership is explicit, but not painful. Memory management should feel like a conversation with the library, not a negotiation. Arenas and frame allocators make the common case trivial; explicit lifetime management is available for the uncommon case.

3. Zero-to-window in under five minutes. A new developer should be able to download the library, write a main() function, compile with a single command, and see a window with a button in under five minutes. No CMakeLists.txt, no pkg-config, no vcpkg.

4. Beautiful by default. The default theme, font rendering, and layout behavior should produce output that is visually competitive with SwiftUI or Material Design. Ugliness should require effort.

5. C99/C11 only. No C++ features, no compiler extensions, no GNU-isms. The library must compile cleanly under gcc, clang, and MSVC with -std=c11 -Wall -Wextra -Wpedantic.

6. Performance is a feature, not an afterthought. The library targets 60fps on integrated GPUs for typical application UIs, with a rendering pipeline designed for GPU acceleration from day one.

## **2.4 Immediate Mode vs. Retained Mode: A Hybrid Resolution**

The debate between immediate-mode (IMGUI) and retained-mode (RMGUI) paradigms is the central architectural question in GUI library design. ClearUI resolves this with a hybrid approach we call “Declarative Immediate” (DI).

**The Pure Immediate Problem**

In pure immediate mode (Dear ImGui, Nuklear), the UI is described and processed every frame. This is beautiful for simplicity: there is no widget tree to manage, no state synchronization, and the code reads top-to-bottom. However, it makes animation, accessibility, hit-testing, and incremental layout extremely difficult. Every frame rebuilds the entire UI, which is wasteful when 95% of the screen is static.

**The Pure Retained Problem**

In pure retained mode (GTK, Qt), widgets are long-lived objects arranged in a persistent tree. This enables efficient diffing, animation, and accessibility, but forces the developer to synchronize application state with widget state—leading to callback hell, signal chains, and the perennial question of who owns the truth.

**ClearUI’s Declarative Immediate Approach**

ClearUI borrows from both paradigms. The developer writes code that looks immediate-mode: a function that declares the UI top-to-bottom, called each frame. Internally, however, ClearUI maintains a lightweight retained tree that it diffs against the declared output. This tree is an internal implementation detail, never exposed to the user. The developer writes immediate-mode code but gets retained-mode performance, animation support, and accessibility metadata.

This is directly inspired by React’s virtual DOM and SwiftUI’s declarative model, adapted for C’s idioms. The key insight is that in C, the “declaration” is just a series of function calls—no DSL, no macros, no special syntax—and the “diffing” is a simple comparison of lightweight node descriptors, not a complex tree traversal.

# **3. Core Architecture & Rendering**

## **3.1 Layered Architecture Overview**

ClearUI is organized into four distinct layers, each with a clear responsibility boundary. Dependency flows strictly downward; no layer may call into a layer above it.

| **Layer**  | **Name**       | **Responsibility**                                                                             |
| ---------- | -------------- | ---------------------------------------------------------------------------------------------- |
| 4 (Top)    | Widget Layer   | Predefined components (buttons, sliders, text inputs, lists). Built entirely using layers 1–3. |
| 3          | Layout Layer   | Flexbox-inspired layout engine. Computes position/size for all nodes in the declared tree.     |
| 2          | Core Layer     | Node declaration, tree diffing, state management, event dispatch, animation interpolation.     |
| 1 (Bottom) | Platform Layer | Windowing, input capture, GPU rendering, font rasterization, clipboard, OS integration.        |

Application developers primarily interact with Layers 3 and 4. Power users may use Layer 2 directly to build custom widgets. Layer 1 is abstracted behind a clear interface, allowing backend swaps without touching application code.

## **3.2 Rendering Backend Strategy**

ClearUI does not implement its own graphics API abstraction. Instead, it targets a thin Render Driver Interface (RDI) that maps to platform-native APIs.

**Render Driver Interface (RDI)**

The RDI is a struct of function pointers representing the minimal set of GPU operations ClearUI needs. This is approximately 15–20 functions covering: initialization, texture creation and upload, draw command submission (textured quads, rounded rectangles, lines, text glyphs), scissor/clip rectangles, and swap/present.

ClearUI ships with three official render drivers:

| **Driver**           | **Backend**        | **Platforms**             | **Notes**                        |
| -------------------- | ------------------ | ------------------------- | -------------------------------- |
| clearui\_rdi\_vulkan | Vulkan 1.1+        | Windows, Linux, Android   | Primary high-performance driver. |
| clearui\_rdi\_metal  | Metal              | macOS, iOS                | Required for Apple platforms.    |
| clearui\_rdi\_webgpu | WebGPU (Dawn/wgpu) | Web (Emscripten), Desktop | Enables browser deployment.      |

A software rasterizer fallback (clearui\_rdi\_soft) is also provided for headless testing and environments without GPU access. Community drivers for OpenGL 3.3+ and DirectX 11 are anticipated but not part of the core specification.

**Draw Command Buffer**

Each frame, the Core Layer produces an ordered list of draw commands (a command buffer) rather than issuing draw calls directly. This allows the render driver to batch, sort, and optimize GPU submissions. The command buffer uses a flat array of tagged unions—no dynamic allocation per command.

## **3.3 Font Rendering, Text Shaping & High-DPI**

**Font Rendering**

ClearUI bundles a default sans-serif font (Noto Sans, chosen for its exceptional Unicode coverage and SIL Open Font License). Font rasterization uses a signed distance field (SDF) approach at build time, producing a texture atlas that allows resolution-independent rendering with sharp edges at any scale. For subpixel-quality rendering, multi-channel SDF (MSDF) is used, following the msdfgen algorithm.

**Text Shaping**

For Latin and CJK text, ClearUI uses a built-in shaping engine that handles kerning, ligatures, and basic bidirectional reordering. For full complex-script support (Arabic, Devanagari, Thai, etc.), ClearUI optionally links against HarfBuzz at compile time. This is the single permitted external dependency, and it is optional—applications that need only Latin text can omit it entirely, keeping the dependency count at zero.

**High-DPI Scaling**

ClearUI operates in logical pixels throughout the API. The Platform Layer queries the display’s scale factor (DPI) at startup and on display-change events, and all coordinates are automatically scaled to physical pixels at the rendering stage. The developer never handles pixel ratios manually. The SDF font atlas ensures text remains sharp at any scale factor.

# **4. Memory & State Management**

Memory management is the single greatest source of anxiety for C developers building GUIs. ClearUI’s strategy eliminates this anxiety through a three-tier model: arena allocation for the UI tree, a per-frame bump allocator for transient data, and an explicit state vault for persistent application state.

## **4.1 The Arena-Based Ownership Model**

The UI tree is allocated from an arena owned by the ClearUI context. When the developer calls cui\_begin\_frame(), the arena is reset (pointer rewound to the start), and all nodes from the previous frame are implicitly freed. This means the developer never calls free() on any UI node, ever.

The arena is pre-allocated at context creation time with a configurable initial size (default: 4 MB). It grows by doubling if the UI tree exceeds the current capacity, which is logged as a diagnostic event. In practice, 4 MB accommodates thousands of widgets without growth.

// The arena lifecycle — fully automatic

cui\_ctx \*ctx = cui\_create(NULL);  // Allocates 4MB arena internally

while (running) \{

    cui\_begin\_frame(ctx);   // Resets arena → previous frame's tree is gone
    
    // ... declare UI ... (all nodes allocated from arena)
    
    cui\_end\_frame(ctx);     // Diff, layout, render, present

\}

cui\_destroy(ctx);            // Frees the arena and all resources

## **4.2 Frame Allocator & Transient State**

Many UI operations need temporary memory: formatted strings for labels, intermediate layout calculations, temporary arrays for child lists. ClearUI provides a per-frame bump allocator accessible via cui\_frame\_alloc(ctx, size). This memory is valid until the next cui\_begin\_frame() call and requires no explicit free.

// Temporary string formatting — no malloc, no free, no leaks

const char \*label = cui\_frame\_printf(ctx, "Items: %d / %d", count, max);

cui\_label(ctx, label);  // label is valid for the entire frame

## **4.3 Persistent State: The Vault**

Unlike transient UI state, application state (a counter value, a text input buffer, a scroll position) must survive across frames. ClearUI provides the “Vault”: a hash-table-based state store keyed by developer-chosen identifiers.

The Vault is inspired by React’s useState hook but adapted for C. The developer calls cui\_state() with a string key and a size, and receives a pointer to persistent memory. On the first call, the memory is zero-initialized. On subsequent frames, the same pointer is returned.

// Persistent counter state — survives across frames

int \*counter = cui\_state(ctx, "main.counter", sizeof(int));

if (cui\_button(ctx, "Increment")) \{

    (\*counter)++;

\}

cui\_label(ctx, cui\_frame\_printf(ctx, "Count: %d", \*counter));

The Vault uses a single contiguous allocation and is compacted on context creation. Keys are hashed (FNV-1a) for O(1) lookup. Stale entries (keys not accessed for N frames, configurable) are automatically pruned to prevent memory growth from dynamic UIs.

## **4.4 Memory Lifecycle Summary**

| **Category**                         | **Allocator**        | **Lifetime**                                | **Developer Responsibility**      |
| ------------------------------------ | -------------------- | ------------------------------------------- | --------------------------------- |
| UI nodes, layout data                | Frame Arena          | One frame                                   | None. Automatic.                  |
| Temporary strings, arrays            | Frame Bump Allocator | One frame                                   | None. Automatic.                  |
| Persistent state (counters, buffers) | Vault (hash map)     | Until pruned or context destroyed           | Call cui\_state() with key.       |
| Textures, font atlases               | GPU Resource Pool    | Until context destroyed or explicit release | Optional cui\_release\_texture(). |
| ClearUI context itself               | malloc/free          | Application lifetime                        | cui\_create() / cui\_destroy().   |

# **5. API Design & Code Examples**

## **5.1 Naming Conventions & Ergonomics**

All public functions and types are prefixed with cui\_ to avoid namespace collisions. Names follow a verb\_noun pattern for actions (cui\_begin\_frame, cui\_push\_style) and a noun pattern for declarations (cui\_button, cui\_label, cui\_row). Configuration structs use C99 designated initializers extensively, with sensible defaults provided by macro constructors (CUI\_BUTTON\_OPTS\_DEFAULT).

The API surface targets approximately 60–80 public functions for a complete application. This is a deliberate constraint: if the API grows beyond 120 functions, it has failed the simplicity test.

## **5.2 Hello World**

The following is the complete, compilable source for a ClearUI Hello World application. It should require no prior knowledge of ClearUI to understand.

\#include "clearui.h"

int main(void) \{

    cui\_ctx \*ctx = cui\_create(&(cui\_config)\{
    
        .title  = "Hello ClearUI",
    
        .width  = 400,
    
        .height = 300,
    
    \});
    
    
    while (cui\_running(ctx)) \{
    
        cui\_begin\_frame(ctx);
    
    
        cui\_center(ctx);
    
            cui\_label(ctx, "Hello, World!");
    
        cui\_end(ctx);
    
    
        cui\_end\_frame(ctx);
    
    \}
    
    
    cui\_destroy(ctx);
    
    return 0;

\}

Compilation (single command, no build system required):

cc main.c -lclearui -o hello        \# Linux/macOS

cl main.c clearui.lib /Fe:hello.exe \# MSVC

## **5.3 Counter Application with State**

This example demonstrates ClearUI’s state management (the Vault), event handling (button return values), and layout (centering, horizontal row).

\#include "clearui.h"

void ui\_counter(cui\_ctx \*ctx) \{

    int \*count = cui\_state(ctx, "counter", sizeof(int));
    
    
    cui\_column(ctx, &(cui\_layout)\{ .align = CUI\_ALIGN\_CENTER, .gap = 12 \});
    
    
        cui\_label\_styled(ctx, cui\_frame\_printf(ctx, "%d", \*count),
    
            &(cui\_text\_style)\{ .size = 48, .weight = CUI\_WEIGHT\_BOLD \});
    
    
        cui\_row(ctx, &(cui\_layout)\{ .gap = 8 \});
    
    
            if (cui\_button(ctx, "- Decrement")) (\*count)--;
    
            if (cui\_button(ctx, "+ Increment")) (\*count)++;
    
    
        cui\_end(ctx);  // end row
    
    
    cui\_end(ctx);  // end column

\}

int main(void) \{

    cui\_ctx \*ctx = cui\_create(&(cui\_config)\{
    
        .title = "Counter", .width = 320, .height = 200,
    
    \});
    
    
    while (cui\_running(ctx)) \{
    
        cui\_begin\_frame(ctx);
    
        cui\_center(ctx);
    
            ui\_counter(ctx);
    
        cui\_end(ctx);
    
        cui\_end\_frame(ctx);
    
    \}
    
    
    cui\_destroy(ctx);
    
    return 0;

\}

## **5.4 Complex Layout & Event Handling**

This example demonstrates a more realistic application: a to-do list with a text input, add button, and scrollable list with delete actions. It exercises layout composition, text input state, dynamic lists, and event handling.

\#include "clearui.h"

\#include \<string.h\>

\#define MAX\_TODOS 256

\#define MAX\_LEN   128

typedef struct \{

    char items\[MAX\_TODOS\]\[MAX\_LEN\];
    
    bool done\[MAX\_TODOS\];
    
    int  count;

\} todo\_state;

void ui\_todo(cui\_ctx \*ctx) \{

    todo\_state \*st = cui\_state(ctx, "todo", sizeof(todo\_state));
    
    char \*input    = cui\_state(ctx, "todo.input", MAX\_LEN);
    
    
    cui\_column(ctx, &(cui\_layout)\{ .padding = 24, .gap = 16, .max\_width = 480 \});
    
    
        cui\_label\_styled(ctx, "My Tasks",
    
            &(cui\_text\_style)\{ .size = 28, .weight = CUI\_WEIGHT\_BOLD \});
    
    
        // Input row
    
        cui\_row(ctx, &(cui\_layout)\{ .gap = 8 \});
    
            cui\_text\_input(ctx, "new\_task", input, MAX\_LEN,
    
                &(cui\_input\_opts)\{ .placeholder = "What needs doing?" \});
    
    
            bool add = cui\_button(ctx, "Add");
    
            if (add && input\[0\] != '\\0' && st-\>count \< MAX\_TODOS) \{
    
                strncpy(st-\>items\[st-\>count\], input, MAX\_LEN);
    
                st-\>done\[st-\>count\] = false;
    
                st-\>count++;
    
                input\[0\] = '\\0';
    
            \}
    
        cui\_end(ctx);
    
    
        // Scrollable task list
    
        cui\_scroll(ctx, &(cui\_scroll\_opts)\{ .max\_height = 320 \});
    
            for (int i = 0; i \< st-\>count; i++) \{
    
                cui\_row(ctx, &(cui\_layout)\{
    
                    .gap = 8, .align\_y = CUI\_ALIGN\_CENTER,
    
                \});
    
                    char key\[32\];
    
                    snprintf(key, sizeof(key), "chk.%d", i);
    
                    cui\_checkbox(ctx, key, &st-\>done\[i\]);
    
    
                    cui\_push\_style(ctx, &(cui\_style)\{
    
                        .text\_color = st-\>done\[i\]
    
                            ? CUI\_COLOR\_MUTED : CUI\_COLOR\_DEFAULT,
    
                        .text\_decoration = st-\>done\[i\]
    
                            ? CUI\_DECORATION\_STRIKETHROUGH : 0,
    
                    \});
    
                    cui\_label(ctx, st-\>items\[i\]);
    
                    cui\_pop\_style(ctx);
    
    
                    cui\_spacer(ctx);  // push delete button to right edge
    
    
                    char del\_key\[32\];
    
                    snprintf(del\_key, sizeof(del\_key), "del.%d", i);
    
                    if (cui\_icon\_button(ctx, del\_key, CUI\_ICON\_TRASH)) \{
    
                        memmove(&st-\>items\[i\], &st-\>items\[i+1\],
    
                                (st-\>count - i - 1) \* MAX\_LEN);
    
                        memmove(&st-\>done\[i\], &st-\>done\[i+1\],
    
                                (st-\>count - i - 1) \* sizeof(bool));
    
                        st-\>count--;
    
                        i--;  // re-check this index
    
                    \}
    
                cui\_end(ctx);  // end row
    
            \}
    
        cui\_end(ctx);  // end scroll
    
    
        // Footer
    
        cui\_label(ctx, cui\_frame\_printf(ctx, "%d tasks remaining",
    
            st-\>count - /\* count done \*/ (\{
    
                int d = 0; for (int i = 0; i \< st-\>count; i++) d += st-\>done\[i\]; d;
    
            \})
    
        ));
    
    
    cui\_end(ctx);  // end column

\}

## **5.5 Custom Drawing & Canvas**

For applications that need custom rendering (data visualization, games, creative tools), ClearUI provides a canvas widget that yields a draw list for direct command submission.

cui\_canvas(ctx, &(cui\_canvas\_opts)\{ .width = 300, .height = 200 \});

    cui\_draw\_list \*dl = cui\_canvas\_draw\_list(ctx);
    
    cui\_draw\_rect(dl, (cui\_rect)\{10,10,280,180\}, 8.0f, CUI\_COLOR\_SURFACE);
    
    cui\_draw\_circle(dl, (cui\_vec2)\{150,100\}, 40.0f, CUI\_COLOR\_ACCENT);
    
    cui\_draw\_text(dl, (cui\_vec2)\{100, 160\}, "Custom!", NULL);

cui\_end(ctx);

# **6. Layout System**

## **6.1 Flexbox-Inspired Layout Engine**

ClearUI’s layout system is modeled on CSS Flexbox, the de-facto standard for one-dimensional layout that most UI developers already understand. The layout engine computes positions in a single pass over the declared tree (with a secondary pass for flex-grow distribution), targeting sub-microsecond performance for trees under 1,000 nodes.

The core layout containers are:

| **Function**  | **Behavior**                            | **CSS Equivalent**                     |
| ------------- | --------------------------------------- | -------------------------------------- |
| cui\_row()    | Arranges children horizontally          | display: flex; flex-direction: row     |
| cui\_column() | Arranges children vertically            | display: flex; flex-direction: column  |
| cui\_stack()  | Overlays children at same position      | position: relative + absolute children |
| cui\_center() | Centers single child in available space | display: grid; place-items: center     |
| cui\_wrap()   | Row/column with wrapping to next line   | flex-wrap: wrap                        |

## **6.2 Constraint Shorthands**

The cui\_layout struct supports the most commonly needed layout properties through a flat, non-nested struct with sensible defaults. All fields default to zero or CUI\_ALIGN\_START, meaning an empty struct literal produces a tightly-packed, top-left-aligned layout.

typedef struct \{

    float gap;         // Space between children (px)
    
    float padding;     // Uniform padding (overridden by padding\_\*)  
    
    float padding\_x;   // Horizontal padding
    
    float padding\_y;   // Vertical padding
    
    float max\_width;   // Constrain maximum width
    
    float min\_width;   // Constrain minimum width
    
    float max\_height;  // Constrain maximum height
    
    float flex;        // Flex grow factor (0 = auto size)
    
    cui\_align align;   // Main axis alignment
    
    cui\_align align\_y; // Cross axis alignment

\} cui\_layout;

# **7. Ecosystem, Tooling & Integration**

## **7.1 Distribution Model**

ClearUI uses a hybrid distribution model that balances the simplicity of single-header libraries with the compile-time performance of pre-compiled modules.

**Primary Distribution: Two-File Drop-In**

The primary distribution is a pair of files: clearui.h (the public header, approximately 800 lines) and clearui.c (the amalgamated implementation, approximately 25,000–40,000 lines). The developer drops both files into their project and compiles clearui.c as part of their build. No build system integration, no pkg-config, no library path configuration.

**Pre-Compiled Static Library**

For larger projects where recompiling 40,000 lines on every build is undesirable, ClearUI provides pre-compiled static libraries (.a / .lib) for each major platform and compiler combination, distributed as GitHub release artifacts. These are compiled with -O2 and include debug symbols.

**Render Driver Selection**

Render drivers are separate compilation units (clearui\_rdi\_vulkan.c, etc.), distributed alongside the core. The developer includes exactly one render driver in their build. A compile-time macro (CUI\_BACKEND\_VULKAN, CUI\_BACKEND\_METAL, CUI\_BACKEND\_WEBGPU) selects the active driver.

## **7.2 Windowing & Event Loop Strategy**

ClearUI is window-library agnostic by default. The Platform Layer defines a cui\_platform interface struct with function pointers for: window creation, event polling, clipboard access, cursor management, and GPU surface creation.

ClearUI ships with built-in platform adapters:

| **Adapter**               | **Dependency** | **Use Case**                                                 |
| ------------------------- | -------------- | ------------------------------------------------------------ |
| cui\_platform\_sdl3       | SDL3           | Best cross-platform support. Recommended default.            |
| cui\_platform\_glfw       | GLFW 3.4+      | Lightweight alternative for desktop-only apps.               |
| cui\_platform\_native     | None (OS APIs) | Zero-dependency mode using Win32/Cocoa/X11/Wayland directly. |
| cui\_platform\_emscripten | Emscripten     | Browser deployment via WebAssembly.                          |

The native adapter is the most ambitious component and will be developed incrementally, platform by platform. SDL3 is the recommended starting point for initial development and is the default in all examples.

**Event Loop Ownership**

ClearUI does not own the event loop. The developer writes the loop (as shown in the Hello World example), calling cui\_begin\_frame / cui\_end\_frame at their own cadence. This allows ClearUI to coexist with game loops, server ticks, or any other application architecture. For convenience, cui\_run(ctx, ui\_func) is provided as a one-liner that handles the loop, sleeping, and event polling internally.

## **7.3 Build Integration**

ClearUI provides optional integration files for common build systems, but none are required.

| **Build System** | **Integration File** | **Notes**                        |
| ---------------- | -------------------- | -------------------------------- |
| CMake            | ClearUIConfig.cmake  | find\_package(ClearUI) support.  |
| Meson            | meson.build          | Subproject or system dependency. |
| pkg-config       | clearui.pc           | Standard Unix discovery.         |
| None (manual)    | N/A                  | cc main.c clearui.c -lm -o app   |

## **7.4 Hot Reload & Developer Experience**

ClearUI is designed to support hot-reloading of UI code during development. Because the UI is declared as a function (no persistent widget objects to invalidate), reloading a shared library containing the UI function and calling it next frame is seamless. The Vault state survives reloads, meaning the application does not lose its state.

ClearUI provides a cui\_dev\_overlay(ctx) function that renders a diagnostic overlay showing: frame time, node count, arena usage, vault entry count, and a visual layout inspector that highlights bounding boxes and padding. This overlay is compiled out in release builds via a CUI\_DEBUG preprocessor guard.

# **8. Accessibility**

Accessibility is a first-class concern, not a retrofit. Because ClearUI’s Declarative Immediate model produces a retained tree internally, it has the structural information needed to expose an accessibility tree to platform screen readers.

Every widget automatically generates accessibility metadata: role (button, checkbox, text input, etc.), label (derived from the visible text or an explicit cui\_aria\_label() call), state (checked, disabled, expanded), and focus order (derived from declaration order, overridable with cui\_tab\_index()).

On Windows, ClearUI maps this tree to UI Automation. On macOS, it maps to NSAccessibility. On Linux, it maps to AT-SPI2. This mapping occurs in the Platform Layer and is transparent to the developer.

Keyboard navigation is built in: Tab/Shift-Tab cycles focus, Enter/Space activates focused elements, and arrow keys navigate within composite widgets (lists, radio groups). The default theme provides visible focus indicators that meet WCAG 2.1 AA contrast requirements.

# **9. Platform Matrix & Requirements**

| **Platform**        | **Render Driver** | **Platform Adapter**           | **Compiler**                   | **Status** |
| ------------------- | ----------------- | ------------------------------ | ------------------------------ | ---------- |
| Windows 10+         | Vulkan            | SDL3 / Native (Win32)          | MSVC 2019+, Clang, GCC (MinGW) | Tier 1     |
| macOS 12+           | Metal             | SDL3 / Native (Cocoa)          | Apple Clang 14+                | Tier 1     |
| Linux (X11/Wayland) | Vulkan            | SDL3 / Native                  | GCC 11+, Clang 14+             | Tier 1     |
| iOS 15+             | Metal             | Native (UIKit)                 | Apple Clang 14+                | Tier 2     |
| Android 10+         | Vulkan            | SDL3 / Native (NativeActivity) | NDK r25+                       | Tier 2     |
| Web (Emscripten)    | WebGPU            | Emscripten                     | Emscripten 3.1+                | Tier 2     |

Tier 1 platforms are tested in CI on every commit. Tier 2 platforms are tested on release candidates.

# **10. Risks, Trade-offs & Open Questions**

**The C99 Constraint**

Restricting to C99/C11 means no templates, no RAII, no overloading. The API must achieve ergonomics through careful struct design, designated initializers, and judicious use of \_Generic (C11). This is a deliberate trade-off: the library can be called from any language with a C FFI, including Rust, Zig, Odin, Python (ctypes), and Lua—making it potentially the most portable GUI library in existence.

**Text Input Complexity**

High-quality text input (IME support, cursor positioning in complex scripts, undo/redo) is among the hardest problems in GUI development. ClearUI’s initial release will support Latin-script text input with basic editing. Full IME and complex-script input is deferred to version 1.1 and will likely require platform-specific code in the native adapter.

**Theming & Styling Depth**

The style system (cui\_push\_style / cui\_pop\_style) is deliberately simpler than CSS. It does not support selectors, inheritance beyond parent scoping, or media queries. This keeps the implementation tractable but may limit the visual flexibility for some applications. A future extension could introduce a simple style sheet format parsed at runtime.

**Thread Safety**

ClearUI contexts are not thread-safe. A single context must be accessed from a single thread (the UI thread). Applications that need to update UI from background threads should use a message queue or atomic flag that the UI thread polls. This is the same model used by virtually all GUI frameworks and is not considered a limitation.

**Open Questions for Community Input**

- Should ClearUI provide a built-in animation API (cui\_animate\_float()), or should animation be handled entirely in user code with delta-time interpolation?

- Should the Vault support serialization to disk for session persistence, or is that out of scope for a GUI library?

- What is the right balance between the native platform adapter (zero dependencies) and the SDL3 adapter (battle-tested, cross-platform)?

- Should ClearUI provide a built-in icon set (Feather, Material), or only provide the mechanism to load icon fonts?

# **11. Conclusion**

ClearUI is an attempt to prove a thesis: that the reason developers abandoned C for GUI programming is not a fundamental inadequacy of the language, but a failure of imagination in library design. C99 provides everything needed to build a joyful, ergonomic UI framework—designated initializers give us named parameters, compound literals give us inline configuration, arena allocators give us fearless memory management, and function pointers give us polymorphism.

The Declarative Immediate paradigm resolves the long-standing tension between immediate-mode simplicity and retained-mode capability. The arena-based memory model eliminates the anxiety of manual allocation without introducing garbage collection overhead. The Render Driver Interface ensures GPU-accelerated rendering on every major platform without forcing the developer to understand Vulkan or Metal.

Most importantly, the API is designed to be read. ClearUI code should look like a description of the UI it produces, not an incantation of rituals required to appease a framework. If a developer can read a ClearUI program and immediately understand what window it will draw, we have succeeded.

This specification is a starting point, not a finished blueprint. The open questions in Section 10 are genuine invitations for community input. The best libraries are not designed in isolation—they are shaped by the collective wisdom of the developers who use them.

*End of Specification  •  ClearUI RFC-0001 v1.0*
