# Research: Review Fixes

**Branch**: `012-review-fixes` | **Date**: 2026-03-07

## Research Tasks

### R1: How to implement `cui_draw_circle` correctly

**Decision**: Use `CUI_CMD_ROUNDED_RECT` with `r = radius` (where the rounded rect has equal width and height = 2*r, making it a circle). This reuses the existing command type without adding a new one.

**Rationale**: A rounded rectangle with corner radius equal to half its shortest side is geometrically a circle (or ellipse). The `CUI_CMD_ROUNDED_RECT` type and `cui_cmd_rounded_rect` struct already exist but have no push function. Adding `cui_draw_buf_push_rounded_rect` and using it from `cui_draw_circle` kills two birds with one stone: circles work correctly, and the dead `CUI_CMD_ROUNDED_RECT` gets a proper API.

**Alternatives considered**:
- New `CUI_CMD_CIRCLE` type: Rejected — adds a new enum value and struct to the union for something a rounded rect already handles. RDI drivers would need yet another case.
- Keep as rect: Rejected — violates the API contract and Clarity principle.

### R2: How to compose accessibility state strings

**Decision**: Use a small static buffer per entry (e.g. `char state_buf[32]`) inside `cui_a11y_entry` to hold composed state like `"checked focused"`. Build the string by appending tokens separated by spaces.

**Rationale**: The current `const char *state` pointing to string literals ("checked", "focused", "") is simple but mutually exclusive. A small fixed buffer avoids dynamic allocation while supporting multiple states. 32 bytes is sufficient for foreseeable state combinations.

**Alternatives considered**:
- Bitmask `uint32_t state_flags`: Cleaner for programmatic use but loses human-readable string for screen readers. Would require a separate formatting step.
- Dynamic allocation: Rejected — would need `free()` and violates the arena/frame model.
- Frame allocator: Possible but the a11y tree is built after frame reset. The a11y tree is rebuilt each frame in `cui_end_frame`, so using a small fixed buffer in the entry struct is simpler.

### R3: How to handle allocation failure in push_container

**Decision**: Check return value of `cui_node_alloc` and return early (do not push NULL onto parent stack). The container function (`cui_center`, `cui_row`, etc.) becomes a no-op on allocation failure.

**Rationale**: Pushing NULL is the root cause — subsequent children get orphaned. Returning early means children in the block will attach to the enclosing parent (or root), which is a graceful degradation. The `cui_end()` call will pop the wrong parent, but since nothing was pushed, the stack stays balanced if we also skip the push.

**Alternatives considered**:
- Return error code from `cui_center`/`cui_row`/etc.: Would change the public API signature (currently `void`). Rejected for backwards compatibility.
- Abort/assert: Too aggressive for a library; callers should handle gracefully.

### R4: Type safety for `cui_set_platform` / `cui_set_rdi`

**Decision**: Change signatures from `const void *` to `const cui_platform *` and `const cui_rdi *` respectively. Forward-declare the types in `clearui.h` or include the necessary headers.

**Rationale**: The current `void *` accepts any pointer without warning. Using typed pointers gives compile-time checking with zero runtime cost. The `cui_platform` and `cui_rdi` types are already defined in `clearui_platform.h` and `clearui_rdi.h`. We can forward-declare them in `clearui.h` to avoid forcing users to include the sub-headers.

**Alternatives considered**:
- Keep `void *` and document: Rejected — violates Clarity Over Cleverness.
- Require `#include "clearui_platform.h"` before `clearui.h`: Rejected — forces include order on users.

### R5: Arena alignment validation approach

**Decision**: Check `align > 0 && (align & (align - 1)) == 0` at the top of `cui_arena_alloc_aligned`. Return NULL if invalid.

**Rationale**: This is the standard power-of-2 check. Returning NULL is consistent with the existing OOM return path. An `assert` in debug builds would also be reasonable but we choose runtime safety for library code.

**Alternatives considered**:
- `assert` only: Rejected — library code should not abort the caller's process.
- Silently round up to next power of 2: Rejected — masks bugs in caller code.

### R6: `cui_text_input_opts.placeholder` type

**Decision**: Change from `int placeholder` to `const char *placeholder`. The widget implementation should store it in the node for rendering (even if rendering is still stubbed).

**Rationale**: The field is semantically a string (placeholder text). The current `int` type is clearly a mistake or placeholder that was never updated.

**Alternatives considered**:
- Remove the field entirely: Rejected — the concept is valid and will be needed when text input is fully implemented.
- Keep `int` as an enum index: Rejected — no enum exists, and string is the natural type.

### R7: Test coverage strategy

**Decision**: Add 4 new unit test files: `test_a11y.c`, `test_frame_alloc.c`, `test_draw_cmd.c`, `test_focus.c`. Update Makefile to include them in `unit-tests` target. Fix `test_arena.c` and `test_vault.c` to print "PASS".

**Rationale**: These cover the 4 largest gaps: accessibility (untested), frame allocator (untested), draw commands (only capacity tested), and keyboard focus (untested). Each is independently valuable and testable.

**Alternatives considered**:
- Test framework (Unity, CMocka): Rejected — adds a dependency. Plain `assert.h` is consistent with existing tests and the zero-dependency philosophy.
- Single mega test file: Rejected — harder to isolate failures and run selectively.

### R8: Static library target

**Decision**: Add a `lib` target to the Makefile that produces `libclearui.a` using `ar rcs`. Include all object files except platform stubs and RDI stubs (those are backend-specific).

**Rationale**: Users currently must compile all sources manually. A static library is the standard distribution mechanism for C libraries and aligns with the Zero-to-Window principle.

**Alternatives considered**:
- Shared library (.so/.dylib): Rejected for now — adds complexity (visibility, PIC) and isn't needed for the primary use case.
- Amalgamated single .c file: Mentioned in constitution (two-file drop-in) but out of scope for this feature.
