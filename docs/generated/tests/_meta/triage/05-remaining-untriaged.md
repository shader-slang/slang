# 05 — Not yet triaged

One entry remains, and one link above is unconfirmed.

## `design/pipeline/05-ir-passes/typeflow-report-dynamic-dispatch-sites.slang`

Fails a `DIAGNOSTIC_TEST` expecting a typeflow report:

```
//CHECK: ^ generated dynamic dispatch code for this site. 2 possible types: 'Square, Rect'
```

No finding exists for it, and the failure mode has not been established. The
two shapes to distinguish:

- the report is no longer emitted, or its wording/site changed (test-bug), or
- dynamic dispatch is no longer generated for this shape because typeflow
  specialization improved (doc-and-test-stale, like 04).

The second is worth checking first: `slang-ir-typeflow-specialize.cpp` changed
by 400 lines in the range this work covered, so a specialization improvement
that removes the dispatch site is plausible, and would make the *doc* claim in
`05-ir-passes.md#specialization-and-generics` stale too.

## `builtinoperationintval-enum-operands-fold` → `enum-cast-in-generic-array-bound-rejected`

Matched by keyword only. Read both before relying on the link.
