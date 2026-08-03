# Session Context

This file summarizes the design context for the draft PR. It is intentionally
compact and records only the candidate direction selected for the PR.

## Goal

Design a Slang pipeline ray tracing API that can target D3D/DXR,
Vulkan/SPIR-V, OptiX, and Metal. The main driver is Metal support: Metal can
perform traversal and call custom intersection functions, but it does not have
native miss or closest-hit pipeline stages.

Inline ray tracing and ray queries are out of scope for this first design.

## Source Material

The design was informed by:

- Metal ray tracing intrinsics: `intersector`, `intersection_result`,
  `intersection_function_table`, Metal 4 intersection function buffers, and
  Metal tag validity rules. This background spec is included in this workspace
  as `docs/design/metal-ray-tracing-intrinsics.md`.
- Existing Slang/DXR/Vulkan pipeline intrinsic notes: `TraceRay`, shader table
  indexing, payloads, miss/any-hit/intersection/closest-hit stages, and
  system-value intrinsics. Those comparison notes are not included in this
  workspace.

The workspace keeps the selected candidate API sketch, prototype code, and the
Metal background spec needed to evaluate the design.

## Selected Candidate

The selected candidate is **SBT structural dispatch**:

- Shader source declares a conceptual SBT through `ITraceProgramLayout`.
- Hit, miss, and callable groups are represented as ordered Slang type lists.
- A trace site uses `RayTraversalDesc`, `RayTracer<ProgramLayout>`, and
  `TraceProgramDescriptor<ProgramLayout>`.
- The portable hit slot is computed with the same formula as D3D/Vulkan:

```text
slot = instanceContribution
     + geometryContribution * sbtStride
     + sbtOffset
```

## Key Decisions

- Keep Slang source pipeline-oriented. Users should not have to write
  Metal-style post-`intersect` dispatch code for portable programs.
- Lower Metal miss and closest-hit behavior by synthesizing post-trace
  dispatch from the declared `ITraceProgramLayout`.
- Use generated Metal visible-function dispatch for miss and closest-hit where
  possible, with switch lowering only as semantic pseudocode or fallback.
- Use Metal function tables or function buffers only for traversal-time
  any-hit/custom-intersection behavior.
- Avoid exposing Metal's ordered template tag lists as the main Slang user
  model. Slang should derive Metal tags from trace context, hit group context,
  primitive kind, motion mode, and descriptor choice.
- Represent the target-specific binding object with
  `TraceProgramDescriptor<ProgramLayout>`. On D3D/Vulkan it corresponds to
  host-side SBT data; on Metal it lowers to the shader-visible function table
  or function-buffer resources needed for traversal and generated dispatch.

## Alternatives Excluded From This PR

The repository workspace contains other local experiments, including static
dispatch tables, isolated trace-context contracts, dispatchable contracts,
standalone Metal stage-lowering sketches, and IFB/user-data samples. They are
not staged for this PR because they are design history or narrower experiments,
not the selected candidate.

## Open Items

- Decide how callable shaders fit the `TraceProgram` model.
- Specify reflection data needed for host-side SBT/function-table construction.
- Specify exact Metal IFT versus IFB lowering selection.
- Define diagnostics for incompatible trace contexts, hit groups, primitive
  kinds, motion modes, and target capabilities.
- Decide which parts of the prototype `.slang` API should become standard
  module declarations versus compiler-recognized built-ins.
