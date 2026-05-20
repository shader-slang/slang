# Prompt: tests-agentic/ast-reference/types/

See [`_common.md`](_common.md) for universal rules. See
[`ast-reference-declarations.md`](ast-reference-declarations.md) and
[`ast-reference-expressions.md`](ast-reference-expressions.md) for the
sibling AST bundles whose "translation rule" (claims to observations)
applies to this one unchanged.

## Target

Produce the test bundle at `tests-agentic/ast-reference/types/`,
anchored to
[`docs/llm-generated/ast-reference/types.md`](../../../docs/llm-generated/ast-reference/types.md).

Audience: nightly CI. The bundle exercises the concrete `Type`
subclasses in the AST — `DeclRefType` and its arithmetic descendants
(`BasicExpressionType`, `VectorExpressionType`, `MatrixExpressionType`),
the aggregate / collection family (`ArrayExpressionType`, `TupleType`,
`OptionalType`, `EnumTypeType`), the function-typed leaf (`FuncType`),
the pointer / parameter-passing types (`PtrType`,
`BorrowInParamType` / `OutParamType` / `BorrowInOutParamType`,
`RefParamType`), the parameter-group / buffer / resource / sampler
type families (`ConstantBufferType`, `ParameterBlockType`,
`HLSLStructuredBufferType`, `HLSLRWStructuredBufferType`,
`HLSLByteAddressBufferType`, `TextureType`, `SamplerStateType`),
the typedef-preserving `NamedExpressionType`, the conjunction
`AndType`, the `ThisType` of an interface, the `ModifiedType`,
the `NullPtrType` / `NoneType` / `StringType` leaves, and the
type-checking pseudotypes (`ErrorType`, `OverloadGroupType`,
`InitializerListType`) — through their **observable consequences**
at type-resolution and type-mismatch-diagnostic time.

## The translation rule (carried from `ast-reference-declarations.md`)

`types.md` is a per-class catalog. Slang does not expose its AST. So
a claim such as "`VectorExpressionType` carries `(element-type,
count)` in its operand list" is testable only via the observable
consequence: a `floatN` value behaves as an N-component vector of
float (its components are read by swizzle, its length is N, an
assignment from a different-arity vector diagnoses).

- **Testable** ⇔ "if the doc's claim about this type class were
  false, the program-text behavior we wrote would change in a way
  `slangc` reports."
- **Not testable through slangc** ⇔ "the claim is about which C++
  class the checker allocated for the type, the name of its operand
  slot, whether two textually identical types share the same
  pointer-equal `Val*` in the `ASTBuilder`, or that an abstract
  intermediate exists in the C++ hierarchy."

### Observable claims (write tests for these)

The `## Nodes` tables group types by family. Pick one representative
claim per concrete class and write the smallest source program that
demonstrates it.

- **`DeclRefType`** (`#nodes`, `#declreftype`) — a struct/class/enum/
  interface name acts as a value-type, can be used to declare a
  variable, and field/member resolution follows the underlying decl.
- **`BasicExpressionType`** (`#nodes`,
  `#basicexpressiontype-vectorexpressiontype-matrixexpressiontype`)
  — each scalar built-in (`int`, `uint`, `float`, `bool`, `void`)
  behaves as the corresponding scalar type; group several scalars in
  one test rather than emitting one file per scalar.
- **`VectorExpressionType`** — `vector<T,N>` / `floatN` carries
  element-type and count; swizzles read those components; size
  mismatch diagnoses.
- **`MatrixExpressionType`** — `matrix<T,R,C>` / `floatRxC` carries
  row and column counts; element access reaches the right slot.
- **`ArrayExpressionType`** — `T[N]` is a sized array; `T[]` is
  unsized; indexing past a known size is a non-claim here (that's a
  runtime claim, not an AST one).
- **`TupleType`** — `Tuple<T1, T2>` is a tuple of fields named `_0`,
  `_1`, ...; the i-th element has type `Ti`.
- **`OptionalType`** — `Optional<T>` holds either `none` or a wrapped
  `T`; `.value` / `.hasValue` are observable.
- **`EnumTypeType`** — the type-of-an-enum-type itself (e.g., the
  enum is usable as a type name).
- **`FuncType`** — a function-typed value (taken via a function
  reference or `FuncTypeExpr` spelling like `(int) -> int`) is
  callable.
- **`PtrType`** — `T*` is the raw-pointer type; a pointer field can
  be dereferenced when the surface supports it (the user-spelling is
  `T*`; pointer-feature gating may apply).
- **Parameter-passing modes** (`#nodes` — `OutParamType`,
  `BorrowInOutParamType`, `BorrowInParamType`, `RefParamType`) — `in`
  is read-only, `out` requires assignment before read, `inout`
  threads a value through.
- **`ConstantBufferType`** / **`ParameterBlockType`** — a uniform of
  the corresponding wrapper makes its fields visible at the entry
  point.
- **`HLSLStructuredBufferType`** / **`HLSLRWStructuredBufferType`** /
  **`HLSLByteAddressBufferType`** / **`HLSLRWByteAddressBufferType`**
  — each spelling is accepted at a uniform parameter and elements /
  bytes are addressable.
- **`TextureType`** — `Texture2D<T>` is a sampled texture type that
  is accepted at uniform parameters.
- **`SamplerStateType`** — `SamplerState` is accepted as a uniform.
- **`NamedExpressionType`** (`#nodes`) — a `typealias` / `typedef`
  preserves the alias name in diagnostics. Verify by triggering a
  type-mismatch on the alias and checking the alias name appears in
  the rendered type in the message.
- **`AndType`** (`#andtype`) — `T : IA & IB` requires both
  conformances; a satisfying type compiles and dispatches both
  methods; a type missing one conformance diagnoses.
- **`ThisType`** (`#thistype-and-assoctypedecl`) — an interface
  method whose return type is `This` returns the concrete-type's
  value when dispatched.
- **`ModifiedType`** (`#nodes`) — a `const T` qualifies the base
  type with a modifier; assigning to a `const`-modified value
  diagnoses.
- **`NullPtrType`** / **`NoneType`** — `nullptr` and `none` carry
  these singleton types; they are accepted at any matching
  `Optional` / pointer-typed target without explicit cast.
- **`StringType`** — a `String`-typed literal preserves its text
  through to emitted target code.
- **`ErrorType`** (`#errortype-and-bottomtype`) — when an expression
  fails to check, the type of the failing sub-expression becomes
  `ErrorType` and **does not cascade** further diagnostics; i.e.,
  the test for ErrorType is "given a single root cause, only one
  diagnostic comes out, not a flood".
- **`OverloadGroupType`** — when an overloaded name cannot be
  resolved, the diagnostic mentions the candidate set; observe the
  fact that overload resolution happens (the overload set is the
  observable surface for `OverloadGroupType`).
- **`InitializerListType`** — `{a, b}` carries `InitializerListType`
  before being matched; once matched, it lowers to the target type
  (struct field-wise or array element-wise).

### Per-target emit observations (use sparingly)

Most type claims are target-independent: the type checker accepts or
rejects the program in the same way regardless of backend. INTERPRET
is the right primary. A small number of types observably change
emitted text per target:

- **`VectorExpressionType`** — `floatN` → `float3` (HLSL/Metal),
  `vec3` (GLSL), etc. Already covered by `ast-reference/expressions`
  swizzle multi-target test; do **not** duplicate.
- **`HLSLRWStructuredBufferType`** + **`ConstantBufferType`** — the
  wrapper spelling appears in the emitted text of HLSL backend. A
  one-shot multi-target test here is allowed where the cross-target
  spelling difference is exactly the documented "this type lowers to
  the documented surface form" claim.

Pre-backend type claims do not need multi-target directives. Test
each claim once with `INTERPRET` or once with `DIAGNOSTIC_TEST`.

### Not testable through slangc (record under `## Out of scope`)

The doc carries many claims about the **internal AST shape** of
these types. These are unobservable:

- The C++ parent class of any `Type` (e.g. that `VectorExpressionType
  : ArithmeticExpressionType`, that `Fp8Type : DeclRefType`, that
  `BuiltinGenericType : BuiltinType`). Only the user-observable role
  of each leaf is testable.
- The `m_operands : List<ValNodeOperand>` storage and the operand
  indices each concrete class uses (e.g. that operand 0 of
  `VectorExpressionType` is the element type). Surface tests cannot
  see operand layout.
- Whether two textually identical types share the same `Val*` (the
  hash-cons / hoistability invariant). The dump shows the
  post-hoist result.
- That singleton types (`ErrorType`, `BottomType`, `NullPtrType`,
  `NoneType`) are one-per-`ASTBuilder`.
- The abstract intermediates from the family hierarchy
  (`ArithmeticExpressionType`, `Fp8Type`, `BuiltinType`,
  `DataLayoutType`, `PointerLikeType`, `BuiltinGenericType`,
  `ResourceType`, `TextureTypeBase`, `TextureShapeType`,
  `HLSLStructuredBufferTypeBase`, `ParameterGroupType`,
  `OutParamTypeBase`, `PtrTypeBase`, `ParamPassingModeType`,
  `UniformParameterGroupType`, `VaryingParameterGroupType`,
  `UntypedBufferResourceType`, `StringTypeBase`).
- **Synthesized-only / lowering-only types**:
  `ExtractExistentialType`, `ExistentialSpecializedType`,
  `GenericDeclRefType`, `NamespaceType`, `NativeRefType`,
  `OverloadGroupType` (the overload set is observable, the class
  identity is not), `InitializerListType` (the brace-initializer is
  observable, the class identity is not). Record under
  `## Out of scope`.
- **`Fp8Type`** family (`FloatE4M3Type`, `FloatE5M2Type`,
  `BFloat16Type`) — surface support varies by target; not portable
  on the no-GPU runner. If the type is reachable from a portable
  Slang program, write one observation; otherwise record as out of
  scope.
- **`CoopVectorExpressionType`**, **`DifferentialPairType`**,
  **`DifferentialPtrPairType`** — these are subsystem-specific
  (cooperative math, autodiff); their observable surface belongs to
  those feature bundles.
- **GLSL-only types** (`GLSLImageType`, `GLSLShaderStorageBufferType`,
  `GLSLInputParameterGroupType`, `GLSLOutputParameterGroupType`,
  `GLSLAtomicUintType`, `GLSLInputAttachmentType`) — observable only
  via GLSL-source ingest; not exercised here.
- **Data-layout marker types** (`DefaultDataLayoutType`,
  `Std430DataLayoutType`, `Std140DataLayoutType`,
  `ScalarDataLayoutType`, `CDataLayoutType`,
  `DefaultPushConstantDataLayoutType`, `IBufferDataLayoutType`) —
  visible only inside the buffer-type encoding; not user-spellable
  in surface Slang.
- **Geometry / tessellation IO types** (`HLSLPatchType`,
  `HLSLInputPatchType`, `HLSLOutputPatchType`, the stream-output
  family, the mesh-output family) — require a real GPU pipeline
  stage and are not exercisable on the no-GPU runner.
- **Sampler-feedback type** (`FeedbackType`), **subpass-input type**
  (`SubpassInputType`), **raytracing-acceleration-structure type**
  (`RaytracingAccelerationStructureType`),
  **dynamic-resource type** (`DynamicResourceType`),
  **descriptor-handle type** (`DescriptorHandleType`),
  **tensor-view type** (`TensorViewType`) — GPU-only / target-only
  surfaces; their existence is documented but not exercisable from
  a portable .slang on the no-GPU runner.
- **All pack / variadic types** (`EachType`, `ExpandType`,
  `PackBranchType`, `FirstPackElementType`, `LastPackElementType`,
  `TrimFirstTypePack`, `TrimLastTypePack`, `ConcreteTypePack`,
  `ValuePackType`) — observable through the pack-expression family
  (covered by `ast-reference/expressions` and the future
  `language-feature/generics-and-packs` bundle). The type-side
  claim is the same observation; do not duplicate.
- **Differentiable-function marker types**
  (`DifferentiableType`, `DifferentiablePtrType`,
  `DefaultInitializableType`, `FunctionBaseType`,
  `DifferentiableFuncBaseType`, `ForwardDiffFuncInterfaceType`,
  `BwdCallableBaseType`, `BwdDiffFuncInterfaceType`,
  `LegacyBwdDiffFuncInterfaceType`, `FwdDiffFuncType`,
  `BwdDiffFuncType`, `BwdCallableFuncType`, `ApplyForBwdFuncType`,
  `RematFuncType`) — produced by autodiff machinery; observable
  through the differentiate-expression family
  (`ast-reference/expressions` covers that surface).
- **`DynamicType`** — the dynamic-dispatch erased type, produced by
  existential elimination. Not user-spellable; observable through
  the existential-feature bundle.
- The `## Family hierarchy` mermaid diagram itself.

If you find yourself thinking "this would verify that the type
allocated is class X" or "this would assert that operand i is the
element type" — stop, that is a source-targeting probe.

## Avoid duplication with sibling bundles

This bundle is **AST-type-class centric**. Adjacent bundles cover
adjacent surfaces:

- `tests-agentic/ast-reference/declarations/` — covers the **`Decl`**
  side that `DeclRefType` references (`StructDecl`, `InterfaceDecl`,
  `EnumDecl`, ...). Do not retest "a struct has a field of type T";
  that bundle owns that claim. The angle here is "a struct's name is
  a `DeclRefType` that resolves through the decl-ref" — same
  observation but anchored at the type side.
- `tests-agentic/ast-reference/expressions/` — covers the
  **type-expression family** (`PointerTypeExpr`, `FuncTypeExpr`,
  `TupleTypeExpr`, `AndTypeExpr`, `ThisTypeExpr`, `ModifiedTypeExpr`).
  Those tests verify the **spelling** is accepted in expression
  context; this bundle verifies the **resulting type** behaves as
  documented when used in real positions (a `ThisType`-returning
  method dispatches to the concrete; a `ModifiedType` `const T`
  refuses assignment).
- `tests-agentic/ir-reference/types/` — covers the **IR** type
  catalog (`Vec`/`Mat`/`Array`/`Func`/`Ptr`/`Optional`/...). That
  bundle's claims observe the `-dump-ir` output. Do not re-test IR
  spellings here; this bundle stays at the AST / type-checker
  surface (interpreter values and diagnostics).
- `tests-agentic/pipeline/03-semantic-check/` — covers the
  type-checker's mechanics (overload resolution, implicit-cast
  insertion). Do not re-test resolution mechanics here; cite that
  bundle's anchors if needed.

If a claim is best tested in a sibling bundle, do not duplicate. If
two bundles seem to want the same test, prefer the one whose primary
anchor names the type involved.

## Allowed secondary doc citations

- `docs/llm-generated/ast-reference/base.md`
- `docs/llm-generated/ast-reference/values.md`
- `docs/llm-generated/ir-reference/types.md` (cross-link only, not a
  test anchor)
- `docs/llm-generated/pipeline/03-semantic-check.md`

If you would cite anything else, stop and record a doc-gap finding
in `BUNDLE.md`.

## Source files you may consult for verification only

- `source/slang/slang-ast-type.h`
- `source/slang/slang-ast-base.h`
- `source/slang/slang-ast-val.h`

You may look at these files to confirm a type class is reachable
through a particular surface form (e.g. that `T*` produces a
`PtrType`, that `out T` produces an `OutParamType`). You may
**not** mine them for behavioral claims that the doc does not make.

## Required structure

1. `BUNDLE.md` with the structure named in `_common.md`. Use
   `## Out of scope` for the internal-shape claims and the
   GPU-only / synthesized-only classes named above.
2. 18 to 35 `.slang` test files (size cap 60). The doc has ~150
   concrete classes but most are abstract intermediates,
   synthesized-only nodes, or GPU-only surfaces. Aim for one
   observation per **user-spellable** type family.

## Test directives

Most type-class claims are target-independent (the type's
representation and its checker-attached behavior are the same
regardless of backend):

- `//TEST:INTERPRET(filecheck=CHECK):` — **primary** directive.
  Use `printf` to observe the value, the field, or the result of a
  method dispatched through a type.
- `//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):` — for negative claims:
  type-mismatch, conformance-missing, const-violation,
  cascading-suppressed (ErrorType), out-parameter not assigned.
- Multi-target `//TEST:SIMPLE` is appropriate only when **type
  lowering observably differs per target** AND the difference is
  not already covered by `ir-reference/types` or
  `ast-reference/expressions`. The clearest case is
  `HLSLRWStructuredBufferType` (the `RWStructuredBuffer<T>`
  spelling appears in HLSL emit). Use one such test in this bundle
  as the canonical multi-target type-lowering observation.

## Cast and observation reminders (carried from `_common.md`)

- Use **constructor-style casts** (`int('A')`), not C-style
  (`(int)'A'`). The C-style form returns 0 in `slangi`.
- `slangi` `printf` does **not** support `%s`. For string-typed
  observation, use `//TEST:SIMPLE(filecheck=CHECK):-target cpp`.
- `slangi` **cannot host `cbuffer` or non-const `static` module
  globals**. For `ConstantBufferType` claims, switch to
  `//TEST:SIMPLE(filecheck=CHECK):-target hlsl`.
- `slangi` **cannot instantiate `class`** — for class / reference
  claims, use `-target hlsl`.
- The runner's "Suggested annotations" output is the source of truth
  for diagnostic caret positions. Hand-counting carets is
  unreliable; copy from suggested annotations.
- Caret `^` placement in `//CHECK:` requires column `>= 10`. For
  earlier columns, use the `/*CHECK: ... */` block-comment form.
- For `DIAGNOSTIC_TEST`, the `non-exhaustive` flag goes **inside**
  the parens: `(diag=CHECK,non-exhaustive)`. Omit it when all
  diagnostics are matched.

## Quality checklist (in addition to `_common.md`'s)

- [ ] Every test's `doc_ref` resolves to an anchor in
      `ast-reference/types.md` (or one of the listed secondary docs).
- [ ] The bundle covers all major **user-spellable** type families:
      DeclRefType, BasicExpressionType, VectorExpressionType,
      MatrixExpressionType, ArrayExpressionType, TupleType,
      OptionalType, FuncType, PtrType (if pointer feature supported
      via `T*` spelling), parameter-passing-mode types
      (`in`/`out`/`inout`), ConstantBufferType / ParameterBlockType,
      HLSL buffer family, TextureType, SamplerStateType,
      NamedExpressionType, AndType, ThisType, ModifiedType,
      NullPtrType / NoneType, StringType, ErrorType (cascading
      suppression), InitializerListType (collapse-into-target).
- [ ] At least one negative / diagnostic test per family that has a
      natural negative form: vector-size-mismatch, const-write,
      AndType-conformance-missing, out-parameter-not-assigned,
      typedef-preserved-in-diagnostic.
- [ ] No test asserts the C++ class identity of an AST type, the
      name of an operand slot, the parent class in the C++
      hierarchy, or the singleton-ness of any pseudotype.
- [ ] No test depends on a GPU. INTERPRET and diagnostic-only
      directives carry the bundle. One multi-target SIMPLE test
      maximum.
- [ ] No test was written by inspecting an uncovered source line.
      If you find yourself thinking "this would cover the branch at
      `slang-ast-type.h:NNNN`", stop and re-read the doc.
- [ ] `BUNDLE.md` `## Out of scope` lists the internal-shape and
      synthesized-only / GPU-only / autodiff-only / pack-only type
      classes that this bundle does not exercise, with one-line
      reasons.
- [ ] `BUNDLE.md` `## Doc gaps observed` is honest. If a claim
      wanted a test but its anchor was too coarse, record the gap.
