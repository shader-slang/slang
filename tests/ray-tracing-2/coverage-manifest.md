# Structural Ray-Tracing Coverage Manifest

This manifest maps the ray-tracing scenarios already tested under `tests/` to the structural API.
It tracks semantic coverage rather than copying every legacy test: target-emitter and regression
tests remain in place, while `tests/ray-tracing-2` proves that the same source-level capability is
expressible through `slang.raytracing`.

Status meanings:

- **Covered**: checked-in structural compile or runtime coverage exists.
- **Partial**: the basic path is covered, but a listed target, variant, or runtime check remains.
- **Coexists**: the test covers a ray-tracing API that the structural SBT API does not replace. Its
  legacy test remains required and structural stages must be able to use it when the stage allows.
- **Excluded**: intentionally outside version one. Only SER is excluded.

## Scenario Matrix

| Scenario | Existing test sources | Structural coverage | Compile targets | Runtime targets | Status / remaining work |
| --- | --- | --- | --- | --- | --- |
| Stage selection and native stage ABI | `tests/vkray/{anyhit,closesthit,intersection,miss,callable}.slang`, `tests/language-feature/execution-model/raytracing-stages-emission.slang` | `frontend/entry-point/struct-entry-all-stages.slang`, `target/portable/stage-entry-mandatory-abi.slang` | D3D, Vulkan, Metal | N/A | **Covered** |
| Complete triangle hit/miss pipeline | `tests/pipeline/ray-tracing/raygen.slang`, `tests/vkray/raygen.slang`, `tests/glsl-intrinsic/raytracing/glsl-rayGen*.slang` | `integrate/triangle-hit-miss.slang`, `runtime/shaders/triangle-hit-miss.slang` | D3D, Vulkan, Metal | D3D12, Vulkan; Metal pending | **Partial**: local Metal runtime remains |
| Payload mutation and native payload ABI | `tests/vkray/{multipleinout,raygen-trace-ray-param-*}.slang`, `tests/diagnostics/execution-model/ray-payload-invalid-stage.slang` | `target/portable/stage-input-payload.slang`, all portable runtime shaders | D3D, Vulkan, Metal | D3D12, Vulkan; Metal pending | **Partial**: local Metal runtime remains |
| Payload access qualifiers and `[raypayload]` | `tests/hlsl/raypayload-*.slang`, `tests/diagnostics/{raypayload-missing-access-qualifiers,invalid-paq-stage-names}.slang` | Payload type is retained through `ITraceContext`; no structural PAQ test yet | D3D, Vulkan | None | **Partial**: add valid and invalid structural PAQ coverage |
| Triangle attributes and face state | `tests/vkray/{anyhit,closesthit}.slang`, `tests/hlsl-intrinsic/ray-tracing/rt-pipeline-intrinsics-{ahit,chit}.slang` | `target/portable/stage-input-triangle-data.slang`, `target/portable/stage-input-hit-attributes.slang` | D3D, Vulkan, Metal | Triangle runtime reads stage selection only | **Partial**: runtime barycentric/front-face result |
| Common ray and launch built-ins | `tests/hlsl-intrinsic/ray-tracing/rt-pipeline-intrinsics-*.slang`, `tests/glsl-intrinsic/raytracing/glsl-ray*.slang` | `target/portable/stage-input-properties.slang`, callable and recursive runtime shaders | D3D, Vulkan, Metal | Vulkan launch state; D3D12/Metal pending | **Partial** |
| Motion trace time | `tests/hlsl-intrinsic/ray-tracing/ray-current-time-motion-blur-cap.slang`, `tests/nv-extensions/nv-ray-tracing-motion-blur.slang` | `target/metal/stage-input-time.slang`, `frontend/diagnostics/ray-time-requires-motion.slang` | Metal | None | **Partial**: local Metal motion runtime |
| Dynamic ray flags | `tests/bugs/ray-flags-non-constant.slang`, flag coverage in `rt-pipeline-intrinsics-*.slang` | `target/portable/trace-call.slang`, Metal runtime-flag lowering checks | D3D, Vulkan, Metal | Basic flags only | **Partial**: runtime accept-first, skip-stage, opacity, and cull cases |
| *AnyHit* accept/ignore control | `tests/vkray/anyhit.slang`, `tests/diagnostics/discard-in-rt.slang` | `runtime/shaders/procedural-hit-filter.slang`, `target/metal/trace-{triangle,curve,bounding-box}-any-hit.slang` | D3D, Vulkan, Metal | D3D12, Vulkan; Metal pending | **Partial**: nested `ignoreHit` / accept-and-end runtime |
| Procedural intersection and `ReportHit` | `tests/vkray/intersection.slang`, `tests/cuda/report-hit.slang`, `tests/hlsl-intrinsic/ray-tracing/rt-pipeline-intrinsics-int.slang` | `runtime/shaders/procedural-hit-filter.slang`, Metal procedural lowering tests | D3D, Vulkan, Metal | D3D12, Vulkan; Metal pending | **Partial**: zero-report and accept-and-end variants |
| Custom intersection attributes and hit kind | `tests/cuda/{optix-get-attributes-mixed,optix-hit-attributes}.slang` | `frontend/contracts/custom-hit-attributes.slang`, `target/portable/stage-input-hit-attributes.slang`, procedural runtime | D3D, Vulkan, Metal | D3D12, Vulkan; Metal pending | **Partial** |
| Callable data, records, and nested calls | `tests/vkray/{callable,callable-caller,callable-shared}.slang`, callable portions of pipeline-intrinsic tests | `target/portable/callable-dispatch*.slang`, `runtime/shaders/callable-record.slang` | D3D, Vulkan, Metal | D3D12, Vulkan; Metal pending | **Partial**: empty data blocked by #12718; Metal runtime remains |
| Shader records and logical SBT slots | `tests/diagnostics/single-shader-record.slang`, `tests/vkray/entry-point-params.slang` | `target/portable/stage-input-record.slang`, `runtime/shaders/{callable-record,multiple-slots}.slang` | D3D, Vulkan, Metal | Multiple hit/miss slots pass on Vulkan; D3D12 and Metal pending | **Partial**: remaining runtime targets |
| Recursive tracing | recursive calls in pipeline-intrinsic and OptiX tests | `runtime/shaders/recursive-trace.slang` | D3D, Vulkan, Metal | D3D12, Vulkan; Metal pending | **Partial**: local Metal runtime remains |
| Acceleration-structure binding and layout | `tests/pipeline/ray-tracing/acceleration-structure-in-compute*.slang`, `tests/spirv/{descriptor-heap-acceleration-structure*,unbounded-acceleration-structure-array,u-to-accelstruct}.slang`, `tests/reflection/acceleration-structure.slang` | `target/portable/acceleration-structure-type.slang`, descriptor reflection tests | D3D, Vulkan, Metal | D3D12, Vulkan | **Partial**: arrays, descriptor heaps, and reflection variants |
| Descriptor / parameter-block composition | `tests/pipeline/ray-tracing/ray-tracing-paramblock-regression.slang`, binding and library regressions | `integrate/triangle-hit-miss.slang`, descriptor binding/reflection tests | D3D, Vulkan, Metal | D3D12, Vulkan | **Partial**: nested parameter-block integration case |
| Linking, generics, and multiple entry points | `tests/library/precompiled-{dxil,spirv}-*.slang`, `tests/spirv/multi-entrypoint*.slang`, ray-tracing files under `tests/bugs/` | stage-interface serialization, liveness, struct entry, and component runtime coverage | D3D, Vulkan, Metal | D3D12, Vulkan | **Partial**: precompiled generic and multi-layout integration cases |
| Wave/subgroup operations in RT stages | `tests/spirv/wave-get-wave-index-raytracing.slang`, `tests/glsl/subgroup-id-num-subgroups-raytracing.slang` | Stage capability checking exists | D3D, Vulkan | None | **Partial**: add structural stage compile coverage |
| Debug info and explicit SPIR-V context | `tests/spirv/{debug-info-rtas,explicit-context-validation-raytracing-*}.slang` | Ordinary target emission is reused | Vulkan | None | **Partial**: add structural debug/context tests |
| Curves | curve-related Metal and OptiX tests | `target/metal/trace-curve-any-hit.slang`, capability diagnostics | Metal | Pending | **Partial**: local Metal curve runtime |
| Multilevel acceleration structures | `tests/cuda/optix-multilevel-traversal.slang` | `target/metal/multilevel-*.slang`, capability diagnostics | Metal | Pending | **Partial**: expose and validate the instance path; local Metal runtime |
| Ray queries | `tests/**/ray-query*.slang`, `tests/**/rayquery*.slang` | Existing ray-query API remains unchanged; structural stages may call it where capabilities permit | Existing targets | Existing tests | **Coexists**: not an SBT/pipeline replacement |
| Hit objects without reordering | non-SER tests under `tests/hlsl-intrinsic/shader-execution-reordering/` | Existing `HitObject` API remains unchanged | Target-specific | Existing tests | **Coexists**: add a structural-stage interop test where legal |
| Vendor cluster and LSS intrinsics | `tests/{hlsl-intrinsic,cuda,language-feature}/**/*{cluster,lss}*.slang` | Capability-gated intrinsics remain callable from structural stages | Target-specific | Existing tests | **Coexists**: add structural-stage compile cases on supported targets |
| OptiX payload registers and CUDA RT ABI | `tests/optix/*.slang`, `tests/optix-payload-register-closesthit.slang`, `tests/cuda/optix-*.slang` | Structural target lowering currently supports D3D, Vulkan, and Metal | CUDA/OptiX | Existing tests | **Coexists**: future OptiX adapter work, not a silent exclusion from the inventory |
| Shader execution reordering | `tests/**/*ser*.slang` and SER-only hit-object cases | None in version one | Target-specific | None | **Excluded**: version-one SER exclusion |

## Legacy Regression Inventory

The matrix also covers the individual ray-pipeline regression files that do not form a reusable
family:

```text
tests/bindings/binding-spv-storage-class.slang
tests/bugs/gh-10092.slang
tests/bugs/gh-11082.slang
tests/bugs/gh-8590.slang
tests/bugs/gh-9073-spirv-pointer-double-dereference.slang
tests/bugs/gh-9509.slang
tests/bugs/gh-9756-nv.slang
tests/bugs/gh-9756.slang
tests/bugs/gh-9757.slang
tests/bugs/spirv-opt-SROA-of-globals.slang
tests/compute/ray-tracing-inline.slang
tests/modules/hit.slang
tests/spirv/c-layout-buffer-2.slang
tests/spirv/pointer-access.slang
tests/spirv/wave-get-wave-index-raytracing.slang
```

Their source-language scenarios map to the linking/generics, resource-layout, stage-input, and
capability rows above. The original files stay enabled to protect legacy lowering.

## Completion Rule

Before the structural API is complete:

1. Every **Partial** row must become **Covered**, or be reclassified as **Coexists** only when the
   row exercises an API that structural SBT declaration does not replace.
2. Portable runtime rows must pass on D3D12 and Vulkan. The same shader semantics and expected
   records must pass through the local native Metal host.
3. Metal-only curve and multilevel rows must compile and run through that local host.
4. The legacy tests represented above remain enabled. Structural coverage supplements them; it does
   not delete target-emitter regressions.
