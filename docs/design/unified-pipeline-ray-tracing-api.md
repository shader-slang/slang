# Unified Pipeline Ray Tracing API Draft

Status: draft PR entry point.

This document points to the selected candidate design for a unified Slang
pipeline ray tracing API. The proposal targets D3D/DXR, Vulkan/SPIR-V, OptiX,
and Metal while keeping inline ray tracing and ray queries out of scope.

## Selected Candidate

The current candidate is **SBT structural dispatch**:

- Shader source declares a conceptual shader binding table through Slang types.
- Hit, miss, and callable groups are ordered type lists in an
  `ITraceProgramLayout`.
- Trace sites use `RayTraversalDesc`, `RayTracer<ProgramLayout>`, and
  `TraceProgramDescriptor<ProgramLayout>`.
- D3D and Vulkan can map the layout to native SBT records.
- Metal can use the same layout to synthesize the miss and closest-hit dispatch
  that Metal does not provide as native ray tracing stages.

Start here:

- [Structural Dispatch Proposal](
  rt-api-workspace/design-sbt-structural-dispatch/PROPOSAL.md)
- [Tutorial](
  rt-api-workspace/design-sbt-structural-dispatch/TUTORIAL.md)
- [Session Context](
  rt-api-workspace/design-sbt-structural-dispatch/SESSION_CONTEXT.md)

Prototype source:

- [`rt_pipeline.slang`](
  rt-api-workspace/design-sbt-structural-dispatch/rt_pipeline.slang)
- [`rt_basic_types.slang`](
  rt-api-workspace/design-sbt-structural-dispatch/rt_basic_types.slang)
- [`rt_stage_contracts.slang`](
  rt-api-workspace/design-sbt-structural-dispatch/rt_stage_contracts.slang)
- [`rt_structural_dispatch_table.slang`](
  rt-api-workspace/design-sbt-structural-dispatch/rt_structural_dispatch_table.slang)
- [`rt-structural-dispatch-table-example.slang`](
  rt-api-workspace/design-sbt-structural-dispatch/rt-structural-dispatch-table-example.slang)

## PR Scope

This PR intentionally excludes local background notes and alternate experiments,
including detailed Metal intrinsic notes, current Slang pipeline intrinsic
notes, static dispatch table sketches, standalone trace-context experiments,
isolated Metal stage-lowering sketches, and IFB/user-data samples.

Those files can remain useful design history, but they are not part of the
candidate being proposed here.
