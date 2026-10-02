# Whole-helper specialization prototype

This prototype addresses the shared helper-based request in
[#13267](https://github.com/shader-slang/slang/issues/13267) and
[#13268](https://github.com/shader-slang/slang/issues/13268). It introduces the internal
optimization hint `[__specializePerConformance]`. The spelling and limits are experimental,
not a proposed stable language contract.

## Source contract

Consider a traversal-owned helper:

```slang
interface IGeometry
{
    static float distance(float3 origin, float limit);
}

[__specializePerConformance]
void intersectCase<G : IGeometry>(G geometry, inout RayQuery<0> query)
{
    float distance = G.distance(query.CandidateObjectRayOrigin(), query.CommittedRayT());
    if (distance > query.RayTMin())
        query.CommitProceduralPrimitiveHit(distance);
}
```

Inside a traversal loop, call
`intersectCase(createDynamicObject<IGeometry>(geometryType, 0u), query)` using the existing
type-conformance registrations. The compiler selects a concrete specialization of the entire
helper. Query reads, intersection, and acceptance remain together. Geometry providers do not
need another interface, a callback requirement, or additional registrations.

Without the hint, Slang specializes the helper for a set of possible conformances and dispatches
its interface operations internally. That can leave reads before the dispatch and acceptance
after the result merge. Merely inlining the ordinary helper does not change that boundary.

The prototype applies when all of the following hold:

- There is exactly one dynamic type argument and one finite witness-table set, with at most
  eight conformers. Other generic arguments may be static.
- The canonical type set equals the set of concrete types carried by those witnesses.
- The helper returns `void`. Results can use concrete `out` parameters.
- Parameter types are identical across the shared and concrete signatures, except for direct
  by-value dynamic payload parameters. Dynamic payload references and nested type-changing
  parameters retain ordinary specialization.
- The original helper's block-child instruction count multiplied by the conformer count is
  at most 1,024. This bounds directly duplicated IR, not native code after downstream inlining.

Unsupported cases retain ordinary specialization. Static calls continue to specialize normally.
This is a best-effort hint, not a guarantee about final driver code or performance. Early forced
inlining can erase the helper boundary before this optimization runs and is not needed here.

The optimization uses existing tag construction and dispatcher defaults. It does not change or
strengthen the contract of `createDynamicObject` for unregistered IDs.

## Compiler boundary and invariants

`analyzeSpecialize` in `slang-ir-typeflow-specialize.cpp` records the finite type and witness
sets. `specializeGenericWithSetArgs` constructs the signature used to pass runtime witness tags.
Before cloning a shared body, `trySpecializeHelperPerConformance` checks the hint and limits.

Each witness's `getConcreteType()` supplies the corresponding type argument. Types and witnesses
are never paired by collection index. Canonical set identity verifies the applicable shape;
there is no new structural equivalence relation or witness representation.

The existing `specializeGeneric` path constructs each concrete helper. The existing
`createDispatchFunc` and `emitWitnessTableWrapper` adapt its signature and generate selection.
The resulting dispatcher has the same tag-parameter calling convention as normal set
specialization, so typeflow call lowering needs no special case. The temporary shared-function
shell is discarded when the concrete path succeeds.

Concrete state references pass through unchanged. Only direct by-value union payloads can
require unpacking. In particular, the prototype does not marshal dynamic reference arguments
through separate temporaries, which would require an aliasing design. Query operations retain
their source order inside each concrete body; no new query-effect analysis or read-motion rule
is introduced.

The input shape is valid existing typeflow IR, not malformed semantic data. The optional change
is the scope of specialization: the whole helper instead of its individual interface calls.

## Relationship to #13245

The associated-result test demonstrates the relevant overlap: constructing and consuming a
concrete associated result inside the helper removes the intermediate `AnyValue` box in emitted
HLSL. This is a small instance of keeping consumers inside dispatch branches.

This prototype does not unify tag namespaces, optimize arbitrary callers, remove all repeated
dispatch, or guarantee handwritten-code parity. It deliberately excludes non-void dynamic
results and multiple independent dynamic type parameters. Broader #13245 work can share the
specialization boundary without becoming a prerequisite for this experiment.

## Validation and measurements

Validation used checkout `e57a377b3` plus this prototype, a Windows Release build, an RTX 4090,
and driver 591.86. These are separate measurements from the reporter's RTX 5090 results.

The two public standalone repros were extended with an ordinary generic helper (mode 3) and
the same helper with the hint (mode 4). The existing callback is mode 2. All 30 source/mode/target
controls compiled for HLSL, DXIL, and SPIR-V at `-O3`; SPIR-V validation was enabled.

| Control, for either repro | DXIL commits | DXIL phi nodes |
| --- | ---: | ---: |
| Ordinary generic helper | 1 | 16 |
| Existing callback | 2 | 13 |
| Annotated generic helper | 2 | 13 |

In the context-read repro, SPIR-V has one object-ray-origin read site for the ordinary helper
and two for the callback and annotated helper. The latter sites belong to mutually exclusive
branches; static site counts are not executed instruction counts.

GPU checks used both backends, five variants, and two independently linked copies per variant.
Outputs matched exactly within each backend. The runner independently checked 512 rays against
double-precision CPU intersection calculations, including inside starts and clipped roots.
The 1,048,576-ray timing runs used four warmup rounds and 24 measured rounds, with rotated and
reversed variant order, four warmup dispatches, and sixteen timestamped dispatches per sample.
Outputs were checked again after timing.

The annotated-helper versus callback paired median difference was 0% in each copy of all four
source/backend runs, including a repeat on the final build. Dispatches were about 56–69
microseconds and timestamps were visibly
quantized; this does not establish sub-percent equivalence or a speedup. Independently linked
copies produced identical backend binaries for each source variant. The shared-commit D3D12
helper and callback also produced identical DXIL binaries; other equal-sized outputs are not
claimed to be identical.

| Captured shader binary | Callback bytes | Annotated helper bytes |
| --- | ---: | ---: |
| Context reads, DXIL | 7,080 | 7,080 |
| Context reads, SPIR-V | 7,780 | 7,676 |
| Shared commit, DXIL | 6,744 | 6,744 |
| Shared commit, SPIR-V | 7,796 | 7,860 |

These are intermediate shader binaries, not native driver binaries. Five compilation samples
per source/mode/target gave the following medians, including process startup:

| Compilation | Callback ms | Annotated helper ms |
| --- | ---: | ---: |
| Context reads, DXIL | 205.76 | 208.30 |
| Context reads, SPIR-V | 195.92 | 198.68 |
| Shared commit, DXIL | 205.14 | 202.44 |
| Shared commit, SPIR-V | 194.33 | 193.67 |

The sample ranges overlap, and the first samples overlapped the tail of test execution.
These measurements do not establish a compilation-time improvement or regression.

Regression coverage includes module-separated providers, success and miss paths, state
mutation, payload values, aliased concrete state, multiple-type/non-void/reference fallback,
the conformer budget, RayQuery code shape, and associated-result consumption. The D3D12/Vulkan
dynamic-dispatch suite passes 529/529 tests, with 314 unsupported/excluded tests skipped.
All 14 new checks pass. Removing the attribute from copied fixtures causes exactly four
intended code-shape failures while the other ten checks pass. A generated 600-call helper
also confirms that the instruction-count budget retains one shared body.

The matching larger-renderer source snapshot, scene, and launch command have not been identified. Its register allocation,
native binary size, and application performance remain unverified. Those measurements are
required before claiming that the original larger-shader observations are resolved. The public
excerpts cannot substitute for a runnable full renderer.

## Review decisions still needed

Decide whether the function-level opt-in and best-effort fallback are the desired user contract,
and choose stable spelling and budget policy if this becomes a supported feature. Evaluate
larger-shader measurements before enabling any automatic heuristic. A focused PR can cover
both helper-based requests; arbitrary caller-tail duplication and automatic query-read placement
remain separate optimization projects.

## Attribute and specialization policy review

The prototype establishes the requested specialization boundary, but these choices need
agreement before exposing a supported attribute:

| Decision | Prototype behavior | Recommendation for design review |
| --- | --- | --- |
| Placement and spelling | Internal function attribute `__specializePerConformance` | Keep the experiment internal; a function attribute naturally expresses helper ownership. Decide public spelling separately. |
| Hint or requirement | Silently falls back to shared specialization | Add an opt-in optimization remark explaining success or the fallback reason. Consider a strict diagnostic mode if users need a reliable specialization contract. Neither promises final driver instructions. |
| Copy budget | At most 8 conformers and 1,024 block-child instructions times conformer count | Treat these as provisional safety limits, not measured profitability thresholds. Adding a ninth provider or enlarging a helper can change code shape. |
| Budget scope | Counts one original helper before concrete specialization and downstream inlining | Does not bound transitive callees, aggregate copies across many helpers, compile time, or final native code size. Existing recursion-depth protection is separate. |
| Results | Only `void`; concrete `out` parameters work | Fixed concrete return types are a reasonable first extension: the existing dispatcher and wrapper already forward returns. Add specialization/code-shape coverage before enabling them. Dynamic or associated returns need separate conversion analysis. |
| Dynamic inputs | One dynamic type and one witness set; direct by-value payloads | Keep this initial scope. Multiple sets require correlation or Cartesian-product policy, and multiple constraints on one type can also exceed this implementation's one-witness-set restriction. |
| References | Identical concrete reference types pass through; type-changing references fall back | Preserve this restriction until aliasing and copy-back semantics are established. |
| Other attributes | Early forced inlining can remove the helper boundary | Define precedence or diagnose incompatible combinations rather than letting a supported request disappear silently. |

The budget constants are not correctness requirements. The signature restrictions avoid
introducing new conversion and aliasing behavior. Canonical witness/type correspondence is
an invariant of the supported input shape, not a profitability heuristic.

These decisions do not require demonstrating a speedup over the callback implementation.
The feature's value is expressing the same specialization boundary with simpler shader code.
Larger-renderer validation remains useful to check applicability and preserve the existing
workaround's performance characteristics.
