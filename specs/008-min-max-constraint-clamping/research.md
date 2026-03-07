# Research: Min/Max Constraint Clamping in Layout

## R1: Where to clamp (measure vs position pass)

**Decision**: Clamp in both passes—after assigning `layout_w`/`layout_h` in the measure pass and again in the position pass (run_layout) when we assign or confirm a node’s size. Use a single helper (e.g. `clamp_node_size(cui_node *n)`) called from every code path that sets `n->layout_w` or `n->layout_h` so behavior is consistent and we avoid missing a site.

**Rationale**: Measure sets size for containers and leaves; run_layout can overwrite or refine (e.g. stretch). Clamping in both ensures the final value always respects constraints whether the value came from measure or from run_layout.

**Alternatives considered**:
- Clamp only at the end of run_layout: would miss nodes that get their size only in measure and are not revisited in run_layout with a new size; leaves would not be clamped if run_layout doesn’t rewrite their size (it does for many containers). So we still need to clamp after measure. Clamping in both is simpler and safe.
- Clamp only in measure: run_layout can assign larger/smaller sizes (e.g. stretch); those would escape the constraint. So we need clamping after any assignment.

## R2: Convention for “no constraint” (0)

**Decision**: Treat **0 as “no constraint”** for both min and max. If `min_width <= 0` we do not enforce a minimum width; if `max_width <= 0` we do not enforce a maximum width. Same for height. This matches the existing pattern in the codebase where 0 often means “unset” (e.g. layout_w > 0 checks).

**Rationale**: Existing `cui_layout` is zero-initialized; without setting min/max, all are 0. So “0 = no constraint” gives backward compatibility and simple mental model.

**Alternatives considered**:
- Use FLT_MAX for “no max”: would require including float.h and documenting; 0 is already the natural “unset” in the struct.
- Separate “isset” flags: would change struct or API; out of scope.

## R3: Helper vs inline

**Decision**: Add a static helper `clamp_node_size(cui_node *n)` that, when `n->layout_opts.min_width > 0` or `max_width > 0`, clamps `n->layout_w` to `[min_width, max_width]`, and similarly for height. Call it after every place that sets `n->layout_w` or `n->layout_h` (both in measure and in run_layout).

**Rationale**: One place to implement the rule (including handling “only min set” or “only max set”); avoids copy-paste and ensures consistent behavior.

**Alternatives considered**:
- Inline at each site: more duplication and risk of missing a case.
- Macro: constitution prefers clarity; a small function is clearer than a macro.
