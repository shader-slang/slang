# Whole-helper specialization prototype

The internal `[__specializePerConformance]` attribute requests concrete copies of an
ordinary generic helper when called with a dynamically selected type. It addresses
the helper-based requests in [#13268](https://github.com/shader-slang/slang/issues/13268)
and [#13267](https://github.com/shader-slang/slang/issues/13267), without adding another
interface, callback requirement, or conformance registration. Its spelling and policy
remain experimental. It is a general generic-function feature, with no geometry or
RayQuery-specific implementation.

## Source contract

```slang
interface IA { uint a(); }
interface IB { uint b(); }
interface IValue : IA, IB {}

[__specializePerConformance]
uint combine<T : IA & IB>(T value)
{
    return value.a() * value.b();
}

// Given existing IValue conformance registrations:
// combine(createDynamicObject<IValue>(typeID, payload));
```

The compiler selects a concrete copy of the entire helper. Both interface calls and
their surrounding computation stay together. Multiple constraints on the same type
do not require a Cartesian product of independent choices. Additional generic
arguments may be static, including a traversal handler and its query-state type.

This supports `void` and fixed concrete results such as `bool`, `float`, and concrete
structs. A result is fixed when the ordinary and every concrete function signature
have the identical result type; no new result conversion is introduced. Concrete
`out` and `inout` parameters are forwarded unchanged. A direct by-value dynamic
payload uses the existing dispatcher marshalling.

The current implementation requires one dynamic type represented by typeflow as an
untagged union, with finite witness sets whose witnesses uniquely identify its
concrete alternatives. Each witness parameter must explicitly constrain that same
generic type parameter. Type-changing results, type-changing references, nested
payload conversions, independent dynamic type arguments, and parameter-pack arity
changes are unsupported. Some partially specialized generic type representations
also fall outside this contract and are diagnosed rather than structurally guessed.

## Diagnostics and resource policy

An unsupported marked dynamic specialization emits warning
`specialize-per-conformance-not-applied` (55219), explaining the reason, and retains
ordinary specialization. Suppress it explicitly when fallback is acceptable, or
make the request mandatory with:

```text
-warnings-as-errors specialize-per-conformance-not-applied
```

Static calls continue to specialize normally. Combining the attribute with
`[__unsafeForceInlineEarly]` produces the same warning because early inlining removes
the boundary on which the request operates. Ordinary `[ForceInline]` is compatible.

The experiment retains resource limits of eight concrete copies and 1,024 original
block-child instructions multiplied by copy count. These are provisional compiler
resource safeguards, not measured profitability thresholds or promises of final
native code size. Exceeding either limit now produces a diagnostic. They do not
bound transitive callees, aggregate copies across helpers, downstream inlining, or
register allocation. Existing recursive-specialization depth protection remains
separate. Public spelling, configurable budgets, and whether a supported version
should diagnose failure as an error by default remain maintainer decisions.

The requested compiler transformation does not guarantee final driver instruction
placement or faster execution. Unknown conformance IDs retain the existing
`createDynamicObject` contract; this feature does not define a new one.

## Representation and compiler boundary

`emitGenericConstraintValue` already knows the checked subtype of each constraint.
It now records a `ConstrainedTypeDecoration` on direct generic witness parameters.
For `T : IA & IB`, both witness parameters point to the same T parameter. Interface
`This` witness parameters carry the corresponding relation too. This preserves
semantic information at its producer: equal possible-type sets or matching interface
names alone cannot prove that witnesses constrain the same parameter.

`analyzeSpecialize` supplies typeflow's canonical type and witness sets.
`specializeGenericWithSetArgs` builds the ordinary signature with one leading tag
per witness set. Before cloning a shared helper body, it attempts the requested
whole-helper specialization and diagnoses any unsupported shape.

`trySpecializeHelperPerConformance` verifies the constrained-type relation, then
indexes each witness set by `getConcreteType()`. Every set must cover exactly the
canonical union alternatives, with one witness per type. It uses these keys to
specialize the original generic with the matching witnesses through
`specializeGeneric`. No witness-entry positions, numeric-tag equality, structural
equivalence relation, or new substitution algorithm are used.

`createDispatchFunc` accepts the count of leading witness tags (one by default).
For this caller they all describe the same concrete T, so only the first selects a
branch. The remaining tags preserve the ordinary caller ABI but are not passed to
the concrete body. Existing callers retain their one-tag behavior. Existing
`emitWitnessTableWrapper` handles direct by-value payloads and fixed returns;
identical concrete references pass through without additional temporaries.

This is valid existing dynamic IR, not malformed data being patched downstream.
The producer-side metadata supplies a relationship needed to safely handle multiple
constraints. The feature changes the specialization boundary; it does not move
individual query reads across `Proceed`, commits, or other state mutations.

## Relationship to #13245

The associated-result regression constructs and consumes a concrete associated
result within the helper, avoiding an intermediate `AnyValue` in emitted HLSL.
That is distinct from returning a dynamically changing result across the helper
boundary. Tag-namespace unification, arbitrary caller-tail duplication, and removal
of all existential conversions remain separate work.

## Validation

The broadened implementation was built on master `feb2452bf` with Windows Release
tools. On RTX 4090 / driver 591.86:

- Dynamic-dispatch suite: 539/539 passed; 318 unsupported/excluded checks skipped.
- Interface suite: 78/78 passed; 34 skipped.
- Generic suite: 229/229 passed; 87 skipped.
- Six focused diagnostic/budget checks pass, including warning-to-error promotion,
  eight versus nine concrete copies, unsupported signatures, and early inlining.
- Concrete scalar/struct return and multiple-constraint tests check D3D12/Vulkan
  results and emitted structure. Detailed IR validation passes the multiple-constraint case.
- A generated 600-call helper diagnoses the instruction budget; promoting the
  named warning to an error rejects compilation.
- Both original repros compile in five modes for HLSL, DXIL, and SPIR-V (30 controls).
  GPU correctness checks pass for both repros on D3D12/Vulkan, with five modes,
  two independently linked copies, 65,536 rays, and 512 CPU-reference rays.

Earlier compact timing experiments on `e57a377b3` plus the initial prototype found
no measurable helper/callback difference. Those are historical measurements, not
timings of this revision. No speedup is claimed. The matching larger-renderer
snapshot, scene, and launch command remain unidentified; its native register count,
native code size, and performance with this feature remain unverified. The
demonstrated benefit is cleaner source with per-conformer helper specialization.
