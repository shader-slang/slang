Shader Coverage Design
======================

This document explains the current architecture of Slang's shader
coverage instrumentation, with emphasis on why the implementation uses
both AST-time synthesis and post-emit metadata.

Overview
--------

Shader coverage currently has two distinct needs:

1. The compiler must inject execution counters into the shader.
2. The host must be able to bind the counter buffer and later map
   counter slots back to source locations.

Those needs are handled by two different mechanisms:

- AST-time synthesis of `RWStructuredBuffer<uint> __slang_coverage`
- post-emit coverage metadata exposed as `ICoverageTracingMetadata`

These mechanisms are complementary.

Current pipeline
----------------

1. During semantic checking, if `-trace-coverage` is enabled, Slang
   synthesizes a module-scope `RWStructuredBuffer<uint> __slang_coverage`
   declaration unless the user already declared one.
2. The synthesized declaration participates in the normal parameter-
   binding and reflection pipeline, just like a user-declared global.
3. During AST lowering, Slang emits an `IncrementCoverageCounter` IR op
   before each instrumented statement.
4. A later IR pass rewrites each counter op into an atomic increment on
   `__slang_coverage[slot]`, assigns one slot per op, and records
   raw per-slot source attribution plus the chosen buffer binding in
   post-emit metadata.

Why AST-time synthesis exists
-----------------------------

The key architectural constraint is that shader reflection in Slang is
driven by `ProgramLayout`, and `ProgramLayout` is populated from AST-
declared parameters before IR instrumentation runs.

That means an IR pass can inject a new global parameter for codegen, but
that IR-only parameter does not automatically become visible through the
normal reflection/layout APIs that hosts use for binding.

For the current ecosystem, that matters because reflection-driven hosts
need the coverage buffer to be a first-class parameter:

- `slang-rhi` binds resources through reflection-derived offsets
- custom Vulkan / D3D12 / Metal hosts commonly walk reflection/layout
- tooling that expects resources in `ProgramLayout` benefits from the
  same discoverability

AST-time synthesis solves that problem by making the coverage buffer
indistinguishable from a user-declared global for downstream layout and
reflection.

Why metadata also exists
------------------------

Reflection visibility is not enough by itself. The host also needs to
know:

- how many counters were emitted
- which slot maps to which source file and line
- which binding was actually chosen for the coverage buffer

That information is not ordinary shader interface metadata, so it is
reported through `ICoverageTracingMetadata` and, for `slangc` workflows,
through `<output>.coverage-mapping.json`.

That metadata is intentionally a little richer than LCOV line coverage:
some slots may be unattributable to a real source file/line, and that
fact is preserved in the metadata/JSON. The LCOV conversion step then
applies gcov-style reporting semantics by filtering those entries out of
line-oriented output.

In short:

- AST synthesis answers: "how does the host discover and bind the buffer?"
- coverage metadata answers: "how does the host interpret the counter data?"

Why raw binding does not automatically replace AST synthesis
------------------------------------------------------------

A future `slang-rhi` API that allows explicit `(space, binding)` binding
for reflection-invisible resources could be useful. It would let some
hosts bind the coverage buffer without depending on reflection.

However, that does not automatically make AST synthesis obsolete.

AST synthesis still has value because it serves a broader set of
consumers than a `slang-rhi`-specific raw-binding path:

- reflection-driven hosts outside `slang-rhi`
- tools that inspect `ProgramLayout`
- integrations that benefit from the buffer remaining visible as a
  normal shader parameter

Because of that, AST-time synthesis should be viewed as the default
architecture for coverage today, not as obvious cleanup debt.

Retirement policy
-----------------

Any future removal of AST synthesis should be treated as optional
contingency, not expected cleanup.

It would only make sense if all of the following become true:

- a raw-binding path exists and is stable where needed
- the important reflection-driven consumers have an equivalent
  alternative path
- the project is willing to accept the loss of reflection visibility
- AST synthesis is causing enough concrete cost or confusion to justify
  the change

Absent those conditions, keeping both access paths is reasonable.

Design direction
----------------

The intended direction for shader coverage is:

- keep AST-time synthesis as the binding/discoverability mechanism
- keep `ICoverageTracingMetadata` as the reporting/attribution mechanism
- add future coverage features, such as branch or function coverage, on
  top of the same split architecture

This gives phase 1 a solid base without forcing all future integrations
through one narrow host API path.
