# Structural Ray-Tracing Design

This directory defines a structural ray-tracing API that maps to native D3D, Vulkan, and OptiX
pipeline stages and to synthesized Metal dispatch.

[PROPOSAL.md](PROPOSAL.md) is the sole normative design. Start there for the source contract,
target semantics, Metal tag inference, and host/reflection boundary. Use
[TUTORIAL.md](TUTORIAL.md) for a compact shader-and-host walkthrough and
[IMPLEMENTATION_PLAN.md](IMPLEMENTATION_PLAN.md) for compiler ownership, repository structure, and
tests.

The focused analyses give additional rationale for the
[dynamic SBT](DYNAMIC_SBT_DESIGN.md), [payload-partition model](PAYLOAD_SAFE_TRACE_VARIANTS.md),
[Metal tag inference](METAL_TAG_LIST_ANALYSIS.md), and
[callable shaders](CALLABLE_SHADER_CONCERNS.md). The
[D3D12 `Record` RHI implementation](D3D12_RECORD_RHI_IMPLEMENTATION.md) explains how reflected
constant-buffer bindings become local-root arguments and per-entry SBT data. Historical analysis
is non-normative when it conflicts with the proposal.

## Design Summary

The API separates the shader-owned program schema from the host-owned SBT:

```text
ITraceProgramSchema                         runtime SBT
├── TraceContext                            ├── any number of hit records
├── HitGroups : IHitGroupList               ├── any number of miss records
├── MissShaders : IMissShaderList           └── any number of callable records
└── CallableShaders : ICallableShaderList
          │                                           │
          └──────────── Slang reflection ─────────────┘
```

- Stage logic is written as structs implementing `IClosestHitShader`, `IAnyHitShader`,
  `IIntersectionShader`, `IMissShader`, or `ICallableShader`.
- `IHitGroup` associates the _ClosestHit_, _AnyHit_, and _Intersection_ stages that form one native
  hit group. _Miss_ and _Callable_ sections list stage structs directly.
- Contexts define payload, primitive, record data, acceleration-structure topology, and motion.
- A schema lists the finite executable entries but assigns no physical record positions.
- The compiler reflects a dense function index for each finalized entry. The host may reuse one
  entry in any number of physical records with different record data.
- Stage inputs expose built-in state as zero-storage properties. Reachable optional-property use
  drives additional native parameters and Metal tag inference; mandatory payload, hit-attribute,
  and callable-data ABI parameters remain present.

`RayTracer<Schema>` provides trace and callable operations.
`TraceProgramDescriptor<Schema>` is erased on D3D, Vulkan, and OptiX, and specializes to Metal
function tables plus a dynamic record buffer on Metal.

## Target Mapping

| Source contract | D3D / Vulkan / OptiX | Metal |
| --- | --- | --- |
| Schema entries | Native pipeline programs and shader identifiers | Functions and generated dispatch arms |
| Runtime records | Native SBT | Compiler-defined record buffer |
| `RayTracer.trace` | Native trace operation | Intersector traversal plus generated dispatch |
| _ClosestHit_, _Miss_, _Callable_ | Native stages | Visible-function-table dispatch |
| Triangle/curve _AnyHit_ | Native _AnyHit_ where supported | Per-primitive candidate-dispatch arm |
| Procedural _Intersection_ + _AnyHit_ | Native report-intersection control transfer | Generated `reportHit` composition inside the candidate arm |

The module is activated only by `import slang.raytracing` with the experimental-feature flag.
Without that import, the compiler does no structural ray-tracing work.

Version one excludes Shader Execution Reordering, Metal intersection-function-buffer arguments,
and Metal `[[user_data]]`.
