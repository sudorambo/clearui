# Quickstart: API Demo Example

## Build & Run

```bash
make demo && ./demo
```

Expected output:

```
=== ClearUI API Demo ===
Platform: stub (headless)
RDI: software
--- Frame 1: Build full UI ---
  Layout: center > column > row, scroll, wrap, stack
  Widgets: label, button x2, checkbox, icon_button, text_input, spacer
  Canvas: rect, circle, text
  Style: push/pop (red label)
  A11y: aria_label, tab_index
--- Frame 2: Inject input ---
  Injected click + Tab key
--- Frame 3: Verify state ---
  Counter: 1
  Checkbox: toggled
demo: PASS
```

## What It Covers

Every public function in `clearui.h` (except `cui_run` and `cui_dev_overlay`) is called at least once, with comments explaining when and why you'd use each one.

Read `examples/demo.c` as a reference for how to wire up and use the full ClearUI API.
