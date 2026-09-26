# Prompt for `coverage/specialize`

Read `docs/generated/tests/_meta/prompts/_common.md` first and follow all of
its metadata, FileCheck, README, and verification rules.

## Target

Maintain white-box characterization coverage for Slang's generic,
existential, and type-flow specialization pipeline. Read these sources before
regenerating the bundle:

- `source/slang/slang-ir-specialize.cpp`
- `source/slang/slang-ir-typeflow-specialize.cpp`
- `source/slang/slang-ir-lower-dynamic-dispatch-insts.cpp`
- `docs/generated/design/ir-reference/generics-and-existentials.md`

Preserve the existing focused tests unless a compiler change makes their
observation point stale. When that happens, update the test to assert the
documented or user-observable invariant instead of preserving an obsolete
implementation proxy.

## Dynamic-dispatch emission

For `dynamic-dispatch-two-conformers-emit.slang`, keep the runtime-dependent
choice between two concrete conformers and the buffer sink so specialization
cannot fold the dispatch to one implementation or remove it.

Non-CPU targets force-inline the synthesized witness-table wrapper and
dispatch functions. Do not require a separately emitted `s_dispatch_*` or
`*_wtwrapper_*` function. Check the emitted CUDA entry point for the surviving
runtime-tag `switch` and for both concrete conformer implementations being
reachable from its cases. Match the two implementations order-independently
and avoid generated numeric suffixes.

## Output and validation

Regenerate only files whose claims or observations changed, keep the README's
coverage row consistent with each regenerated test, and run:

```bash
python3 docs/generated/tests/_meta/regenerate.py lint coverage/specialize
python3 docs/generated/tests/_meta/regenerate.py verify coverage/specialize
```
