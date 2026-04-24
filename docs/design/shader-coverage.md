Shader Coverage Design
======================

This document describes the phase-1 shader coverage implementation in
Slang, the background that led to it, and the role of the main pieces in
the pipeline.

Overview
--------

Shader coverage has two separate jobs:

1. insert execution counters into generated shader code
2. let the host discover the counter buffer and map counter slots back
   to source locations

In the current implementation, those jobs are handled by two different
mechanisms:

- AST-time synthesis of `RWStructuredBuffer<uint> __slang_coverage`
- post-emit coverage metadata exposed as `ICoverageTracingMetadata`

The first is about binding and reflection visibility. The second is
about reporting and attribution.

Current pipeline
----------------

1. During semantic checking, if `-trace-coverage` is enabled, Slang
   synthesizes a module-scope `RWStructuredBuffer<uint> __slang_coverage`
   declaration unless the user already declared one.
2. That declaration participates in the normal parameter-binding and
   reflection pipeline, the same way as any other global parameter.
3. During AST lowering, Slang emits an `IncrementCoverageCounter` IR op
   before each instrumented statement.
4. A later IR pass rewrites each counter op into an atomic increment on
   `__slang_coverage[slot]`, assigns one slot per op, and records
   per-slot source attribution together with the chosen buffer binding in
   post-emit metadata.

Background for the chosen approach
----------------------------------

An earlier direction considered synthesizing the coverage buffer only in
IR. That approach can work for code generation, but phase 1 also needs
the buffer to participate in the existing binding and reflection model.

The main reason is that shader reflection in Slang is driven by
`ProgramLayout`, and `ProgramLayout` is built from AST-declared
parameters before IR instrumentation runs. An IR pass can introduce a
new global for lowering, but that IR-only global does not automatically
become visible through the ordinary reflection/layout APIs.

For phase 1, that visibility is useful because current hosts and tools
often discover resources through reflection:

- `slang-rhi` uses reflection-derived offsets for binding
- custom Vulkan / D3D12 / Metal integrations commonly walk layout data
- tools that inspect `ProgramLayout` benefit from the coverage buffer
  appearing as a normal shader parameter

For those reasons, phase 1 uses AST-time synthesis for the coverage
buffer instead of relying on an IR-only parameter.

Role of coverage metadata
-------------------------

Reflection visibility alone is not enough for coverage reporting. The
host also needs information such as:

- how many counters were emitted
- which slot maps to which source file and line
- which binding was chosen for the coverage buffer

That information is reported through `ICoverageTracingMetadata` and, for
`slangc` workflows, through `<output>.coverage-mapping.json`.

The metadata is intentionally a little richer than LCOV line coverage.
Some slots may not map to a real source file and line, and that fact is
preserved in the metadata and JSON sidecar. The LCOV conversion step
then applies gcov-style reporting rules by filtering those entries out
of line-oriented output.

In short:

- AST synthesis answers: "how does the host discover and bind the
  buffer?"
- coverage metadata answers: "how does the host interpret the recorded
  counter data?"

Alternative designs
-------------------

Other designs are possible. For example, a future API could allow some
hosts to bind the coverage buffer through explicit `(space, binding)`
information without requiring the resource to appear in reflection.

That would change the tradeoff for some integrations, but it would solve
a different problem from source attribution metadata. It would also need
to account for hosts and tools that currently rely on reflection-visible
resources.

This document does not treat the current phase-1 design as the only
possible long-term design. It records the approach chosen for the
current implementation and the constraints that made it a practical fit.

Design direction for phase 1
----------------------------

The phase-1 implementation therefore uses this split:

- AST-time synthesis for binding and discoverability
- `ICoverageTracingMetadata` for reporting and attribution

That separation is intended to make incremental coverage work practical
without committing later phases to one specific host integration model.
