---
generated: true
model: claude-opus-5[1m]
generated_at: 2026-08-13T00:00:00+00:00
source_commit: c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
watched_paths_digest: 222f123db9618a770d0c176108af6e0d45268a23ffc2b9fb944eef03bbe467fa
source_doc: docs/generated/design/ir-reference/generics-and-existentials.md
source_doc_digest: 15c2c97989955a40a237787b15b31b11aaf1d718c59916ad109eee00afb7ba02
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Tests for design/ir-reference/generics-and-existentials

## Intent

This bundle is the per-opcode regression suite for the IR family described in
[`docs/generated/design/ir-reference/generics-and-existentials.md`](../../../../design/ir-reference/generics-and-existentials.md):
the opcodes that apply generic arguments (`specialize`,
`global_generic_param`), resolve an interface requirement through a witness
table (`lookupWitness`), build and take apart existential values
(`makeExistential`, `createExistentialObject`, the three
`extractExistential*` projections), record conformance (`witness_table`,
`witness_table_entry`, `interface_req_entry`, `thisTypeWitness`,
`TypeEqualityWitness`, `builtinRequirementKey`) and carry runtime type
information (`GetSequentialID`, `GetDynamicResourceHeap`). Every opcode the
doc attributes to a lowering-time producer gets at least one test; every
opcode the doc attributes to a later IR pass, or records as having no
producer at all, is listed under `## Untested claims` instead.

The observation mechanism is uniform: `-target spirv-asm -dump-ir -o
/dev/null -entry main -stage compute`, FileChecked against the first
`### LOWER-TO-IR:` block. Patterns anchor at the opcode mnemonic
(`specialize`, `lookupWitness`, `makeExistential`, `witness_table_entry`,
`interface_req_entry`, `global_generic_param`, `GetSequentialID`) or at a
user-named symbol, because the dump preamble is large. Results escape into
an `RWStructuredBuffer` and operands come from `uniform` globals so neither
dead-code elimination nor constant folding removes the surface under test.
The three negative tests use `DIAGNOSTIC_TEST` at the check stage, where the
claim is that no `specialize` can be built at all.

What separates this bundle from its neighbours: `ir-reference/structure`
anchors `witness_table`, `witness_table_entry`, `interface_req_entry` and
`lookupWitness` from the parent-child structural side, and
`cross-cutting/ir-instructions` samples each IR family once. Here every test
asserts an **operand list, a result type, or a composition of opcodes that
the dispatch path consumes** — which witness a cast packs, which key a
lookup names, which value a row pairs with a key, and in what order the
projections feed a call.

## Claims

Enumerated per [`_claims.md` §1](../../../_meta/prompts/_claims.md). This page is
half user-observable and half internal compiler structure, so the list is split:
**C1–C11 and the flagged rows below are internal-source facts** (enumerator
naming, wrapper-generation mechanics, table markers, producer file/line
attribution) with no consequence a compiled program can reveal, while the rest
are claims about what a Slang surface lowers to and what the compiler accepts or
rejects.

### Source (internal-source facts)

1. The generated `IROp` enumerator is `kIROp_` plus the entry's `struct_name` rather than its Lua key, so `key` becomes `kIROp_StructKey`, `lookupWitness` becomes `kIROp_LookupWitnessMethod` and `rtti_object` becomes `kIROp_RTTIObject`.
2. The Lua key survives as the mnemonic printed by `-dump-ir`.
3. Where `struct_name` is omitted, `process` derives it from the key with `to_pascal_case`.
4. Twenty-one wrappers in this family are hand-written `struct IRFoo` declarations while every other wrapper is emitted by the FIDDLE template, which skips any entry whose `IR<struct_name>` is already defined.
5. `IRTypeEqualityWitness` gets `getSubType()` / `getSuperType()` from the generator, while hand-written `IRTypeSet` gets `getCount()` / `getElement(i)` from `IRSetBase` instead.
6. Seven opcodes — `rtti_object`, `makeExistentialWithRTTI`, `extractTaggedUnionTag`, `extractTaggedUnionPayload`, `UnboundedGenericElement`, `GetDispatcher` and `GetSpecializedDispatcher` — have an `IRBuilder` emitter that nothing calls at `source_commit`.
7. `createExistentialObject` and `GetDynamicResourceHeap` are produced from a core-module `__intrinsic_op` declaration rather than from a `visit*` method.

### Family hierarchy and table markers (internal-source facts)

8. `SetBase` is the only abstract Lua parent entry in this family; the other boxes group opcodes for the reader and do not correspond to Lua entries.
9. A `†` on an operand name means the Lua entry does not declare that operand, so the name and index come from the C++ wrapper's accessors or from the construction site.
10. A `‡` after a wrapper name means the wrapper is hand-written rather than FIDDLE-generated.
11. Flag codes in the tables are `H` hoistable, `P` parent, `G` global.

### Generic application

12. `specialize(base, args...)` applies generic arguments to a generic value — function, type, or witness table — and is hoistable, so identical specializations dedupe.
13. `specialize` originates from any decl-ref carrying a `GenericAppDeclRef`, via `emitDeclRef`.
14. `global_generic_param` declares a generic parameter at module scope, takes no operands, and carries the `G` (global) flag.
15. `bind_global_generic_param(param, val)` binds a global generic parameter to a concrete value when a specialized component type is generated.
16. `globalValueRef(value)` is a non-hoistable "pin" that keeps a global value referenced from inside a function body so dependent computation can be emitted locally.
17. The Slang spelling that declares a `global_generic_param` is `type_param T;` at module scope.
18. `type_param T : IFoo;` lowers to two module-scope parameters rather than one: a `Type`-typed parameter for `T` and a `witness_table_t(%IFoo)`-typed parameter for the conformance.
19. A conjunction `type_param T : IFoo & IBar;` produces a witness parameter for each half, one per constraint.
20. A module that actually uses such a parameter cannot be compiled on its own: every `global_generic_param` that still has a use once specialization has run is reported as `E38207`, "global generic parameter used in code without a concrete binding".
21. The `E38207` message is emitted once per source declaration, not once per lowered parameter — the reporting loop skips the synthesized witness parameters unless they are the only ones left with a use.

### Witness lookup

22. `lookupWitness(witnessTable, requirementKey)` resolves an interface requirement through a witness table, declares `min_operands = 2`, and is hoistable.
23. Its two operands are read back by `getWitnessTable()` / `getRequirementKey()` rather than by index.
24. Its AST origin is an interface requirement reached through a `LookupDeclRef` via `emitDeclRef`, plus `visitWitnessLookupIntVal`, `visitTransitiveSubtypeWitness` and `emitCastToInterfaceSuperTypeRec`.

### Existential construction

25. An existential whose concrete type is not statically known depends on the linkage for the set of types it can hold.
26. When that set comes back empty at a site that has to resolve the existential, the pass reports `E50100`, "no type conformances found", and code generation stops.
27. A conformance that nothing else in the program references is forced into the link with `slangc -conformance <Type>:<Interface>[=<id>]`; that is what makes a `createDynamicObject` program compile.
28. `makeExistential(value, witness)` packs a value plus the witness that its concrete type conforms to the target interface; its AST origin is `CastToSuperTypeExpr`, and also `assign` when storing back into an opened existential.
29. `makeExistentialWithRTTI(value, witness, typeRTTI)` is `makeExistential` carrying the value's type as an explicit operand, and has no producer at HEAD.
30. `createExistentialObject(typeID, value)` builds an existential from a runtime type id plus a value rather than from a static witness, and comes from the core-module `createDynamicObject<T, U>(uint typeId, U value)`.
31. `wrapExistential(wrappedValue, slotArgs...)` converts a value of type `BindExistentials<T, ...>` back to `T`.

### Existential destructuring

32. `extractExistentialValue(existential)` reads the packed concrete-typed value from an existential, and is not hoistable.
33. `extractExistentialType(existential)` reads the packed concrete type, and is hoistable.
34. `extractExistentialWitnessTable(existential)` reads the packed witness table, and is hoistable.
35. `getValueFromBoundInterface(value)` reads the concrete-typed value out of a `BoundInterface<I, T, w>` value.
36. `isNullExistential(val)` is true when an existential holds the "null" placeholder, and lowers `Optional<ISomeInterface>`'s has-value test.
37. `extractTaggedUnionTag(val)` reads the discriminator of a tagged-union existential representation and has no producer at HEAD.
38. `extractTaggedUnionPayload(unionVal, tag)` reads the payload; the Lua entry declares only `unionVal` while the builder supplies a second `tag` operand, and it is uncalled at HEAD.

### Witness tables and witness facts

39. Operand 0 of a `witness_table` is the conforming concrete type; the interface it satisfies lives in the table's result type, read with `getConformanceType()`.
40. A `witness_table_entry` is the keyed child holding the satisfying value, and resolving a `lookupWitness` substitutes that value rather than the entry itself.
41. An `InterfaceType` gets one `interface_req_entry` per lowered interface requirement, which is not the same as per member — subscript, property and default-implementation declarations are excluded while their accessors are requirements in their own right.
42. An associated-type bound such as `associatedtype A : IBar` is a sibling requirement carrying a `WitnessTableType(bound)` value rather than a nested one.
43. A `requirementKey` operand may be either a `key` / `StructKey` or a `builtinRequirementKey`, which is why the schema and generated accessors type it as `IRInst`.
44. `thisTypeWitness` is the abstract witness that `ThisType` conforms to the enclosing interface, and a `lookupWitness` through it stays abstract until specialization.
45. `TypeEqualityWitness` is the witness form used when a constraint is discharged by a type-equality fact instead of an interface implementation.
46. `indexedFieldKey` is a placeholder key for a tuple-like field and is not part of interface dispatch.
47. The worked example `interface IHasProp { property int val { get; set; } int plain(int x); }` lowers to an `interface` with exactly three `interface_req_entry` children, keyed by `%IHasPropx5Fvalx5Fget`, `%IHasPropx5Fvalx5Fset` and `%IHasPropx5Fplain`, and none for `val` itself.
48. The `x5F` in those key names is `scrubName` escaping the `.` that a name hint uses as its separator.
49. The sibling entry a bound such as `associatedtype A : IBar` produces is keyed off a synthesized constraint decl carrying no name, so its key prints as a bare `%N` rather than under a hint.
50. A witness table is an unordered key-to-value map: entry order is not part of the representation and lowering does not guarantee it matches the `interface_req_entry` order on the interface type.
51. A `lookupWitness` is a first-class unevaluated value — the IR carries the lookup itself, not the value it will resolve to.
52. When an interface requirement is itself a `GenericDecl`, lowering stores a requirement-local `IRGeneric` as the witness-table entry value.
53. A use of such a requirement therefore reads `specialize(lookupWitness(table, key), methodArgs...)` rather than a flat `lookupWitness`.
54. The intermediate lookup is typed `Generic`, not a function type, so the dump reads `let %g : Generic = lookupWitness(table, key)` and only the `specialize` wrapped around it is callable.

### Runtime type information

55. `rtti_object` is a materialized runtime type-info record for the type in operand 0, with details carried as `RTTI*Decoration`s, and has no producer at HEAD.
56. `GetSequentialID(RTTIOperand)` returns a stable `uint` ID for its operand and is hoistable.
57. Despite the operand's Lua name, every caller passes a witness table, not an `rtti_object`.
58. `GetDynamicResourceHeap(bindingIndex)` yields the bindless descriptor-heap array of a given binding index, comes from the core-module `__getDynamicResourceHeap<T : IOpaqueDescriptor>(constexpr uint bindingIndex = 0)`, and is hoistable.
59. Its result type is an array of the descriptor type.
60. `lowerDynamicResourceHeap` replaces `GetDynamicResourceHeap` with a laid-out `global_param`, so the opcode never reaches emit.

### AnyValue marshalling

61. `packAnyValue(value)` packs a typed value into an `AnyValueType` blob.
62. `unpackAnyValue(value)` reads a typed value out of an `AnyValueType` blob, with the concrete type being the result type of the op.
63. Both originate from `Result<T, E>` legalization and from the type-flow passes.

### Type-flow specialization — sets and set elements

64. The type-flow specialization pass replaces dynamic dispatch through interface witnesses with tag-driven dispatch over a closed set of conforming types, witness tables, functions, or generics.
65. Every set is hoistable, so set equality is inst identity.
66. A set has at least one operand, its operands must be concrete non-set insts, and operand order must be consistent.
67. `IRBuilder::getSet` maintains those invariants through a persistent unique-ID map, so sets should never be built operand-by-operand.
68. `TypeSet`, `FuncSet`, `WitnessTableSet` and `GenericSet` are the four closed-set opcodes, each variadic and hoistable and each produced by the type-flow specialization pass.
69. The four `Unbounded*` opcodes and the two `Uninitialized*` opcodes are elements of a set, not sets.
70. `IRSetBase::isUnbounded()` and `containsUninitializedElement()` scan a set's operands for them, which is why `TypeSet(A, B, UnboundedTypeElement(I))` reads as "A, B, and any other type conforming to `I`".
71. `UnboundedFuncElement`'s Lua entry declares a `funcType` operand, but the builder creates it with zero operands and no other producer supplies one.
72. `NoneTypeElement` and `NoneWitnessTableElement` are the default "none" elements used with `OptionalType`.
73. When the witness-table set reaching an `extractExistentialWitnessTable` carries an `Uninitialized*` element, the pass reports `DynamicDispatchOnPotentiallyUninitializedExistential` (`E50101`) and yields no info instead of specializing against a set that includes garbage.
74. An interface local must therefore be assigned on every path that reaches a dynamic dispatch on it.

### Type-flow specialization — tagged unions and tag operations

75. `MakeTaggedUnion(tag, value)` builds a `TaggedUnionType` value from a `SetTagType` tag and an `UntaggedUnionType` payload.
76. `CastInterfaceToTaggedUnionPtr(ptr, witnessTableSet, typeSet)` casts an interface-typed pointer to a tagged-union pointer, carrying the two sets on the cast so they survive replacement of the `TaggedUnionType`.
77. `GetTagFromTaggedUnion(taggedUnionValue)` extracts the witness-table tag; its result type is `SetTagType(witnessTableSet)`.
78. `GetTypeTagFromTaggedUnion(taggedUnionValue)` extracts the type tag; its result type is `SetTagType(typeSet)`.
79. `GetValueFromTaggedUnion(taggedUnionValue)` extracts the payload; its result type is `UntaggedUnionType(typeSet)`, or the single element type when the type set is a singleton.
80. `GetTagForSuperSet` and `GetTagForSubSet` translate a tag to its equivalent in a super-set and a sub-set respectively.
81. `GetTagForMappedSet(tag, lookupKey)` translates a witness-table-set tag through the mapping a requirement key induces — the tag-domain replacement for a `lookupWitness`.
82. `GetTagForSpecializedSet(tag, specializationArgs...)` translates a generic-set tag into the corresponding specialized set.
83. `GetTagFromSequentialID` and `GetSequentialIDFromTag` convert between a global sequential ID plus interface type and a set-local tag.
84. `GetElementFromTag(tag)` resolves a tag to its concrete set element; its result type is `ElementOfSetType(set)`.
85. `GetTagOfElementInSet(element, set)` returns the tag for a concrete element of a set and is hoistable; the element must resolve to a concrete inst before lowering.

### Type-flow specialization — dispatchers and existential specialization

86. `GetDispatcher(witnessTableSet, lookupKey, paramBindings...)` returns a `FuncType`-typed dispatcher for one requirement key over a witness-table set, whose first parameter is a `SetTagType(witnessTableSet)`, and has no producer at HEAD.
87. `GetDispatcher`'s `lookupKey` is typed `IRInst`, not `IRStructKey`, because a built-in requirement reached through dynamic dispatch uses a `BuiltinRequirementKey`.
88. `GetSpecializedDispatcher(witnessTableSet, lookupKey, specializationArgs...)` is the same for a key that points at a generic, and also has no producer at HEAD.
89. `SpecializeExistentialsInFunc(func, bindings...)` is a reference to a function with specific existential-parameter bindings, one binding per parameter, with `VoidLit` for "any".
90. `SpecializeExistentialsInType(baseType, bindings...)` is the compiler-dictionary key for a specialized `BindExistentialsType` result.
91. `WeakUse(inst)` marks a use that must not pin its operand.
92. `FuncTypeOf(funcAsType)` is a compile-time projection of a function's type, used so a func-dependent type can name a callable without embedding the callable itself.
93. `FuncTypeOf` is the one row in that table the type-flow pass does not produce: `lowerFuncDependentType` is its only construction site, so it appears at lowering and is reachable from ordinary source — calling `fwd_diff` on a `[Differentiable]` function gives the result a `FwdDiffFuncType` whose lowering wraps the original callable in a `FuncTypeOf`.

### Notable opcodes — `specialize`

94. The Lua entry declares only `base, arg`, but `IRSpecialize` exposes the whole tail through `getArgCount()` / `getArg(i)`, so the arguments must be read through those rather than assuming a fixed arity.
95. The opcode is hoistable, so two references to the same generic with the same arguments collapse to one IR value.
96. A conformance declared on a generic type — `struct Box<T> : IFoo` — puts the `InheritanceDecl` under the generic, so the conformance lowers to a `generic %N : witness_table_t(%IFoo)`.
97. Calling a constrained helper `int use<U : IFoo>(U u, int x)` with `Box<int>` therefore emits a nested `specialize` in the outer call's argument list — `call specialize(%use, specialize(%Box, Int), specialize(%boxWitness, Int))` — where the second argument is the specialized type and the third the specialized witness table.
98. A `generic` body ends in `return_val`, not `yield`.

### Notable opcodes — `lookupWitness`

99. The first operand of a `lookupWitness` is a witness-table value, most often flowing in from the caller as a generic argument or extracted from an existential; the second is the requirement key.
100. A lookup whose table is a `thisTypeWitness` stays abstract, with `isAbstractWitnessTable` recognizing that shape and recursing through nested lookups.

### Notable opcodes — `makeExistential`

101. `makeExistential(value, witness)` packs a value of concrete type `C` and a witness that `C` conforms to interface `I` into a single value of type `I` — the IR form of a widening cast to an interface type.
102. The AST node it comes from is `CastToSuperTypeExpr`, the same node that models any widening cast, so the lowering entry point is `visitCastToSuperTypeExpr` rather than anything existential-specific.
103. When the cast crosses more than one inheritance step, `emitCastToInterfaceSuperTypeRec` walks the transitive witness first and feeds the resulting table in as operand 1.

### Notable opcodes — `wrapExistential`

104. Given a value of type `BindExistentials<T, ...>` whose existential parameters have been bound to concrete types, `wrapExistential` produces a value of type `T` so a callee expecting `T` can be invoked.
105. Operand 0 is the wrapped value and the remaining operands are `(concrete type, witness table)` slot pairs, read with `getSlotOperandCount()` / `getSlotOperand(i)`.
106. `IRBuilder::emitWrapExistential` short-circuits two cases at construction time: with zero slot arguments it returns the value unchanged, and when the target type is an `InterfaceType` it produces a `makeExistential` instead, so those two forms never coexist as alternative spellings of the same value.

### Notable opcodes — the three existential projections

107. The three projections reverse `makeExistential`: value recovers the packed value, type recovers the packed type, and witness table recovers the conformance witness.
108. The type and witness-table projections are hoistable — their results are determined purely by the existential operand, so repeated dispatches through the same existential share one of each — whereas the value projection is not.
109. All three have real AST origins rather than being pass-introduced.

### Notable opcodes — `TypeEqualityWitness`

110. `TypeEqualityWitness(subType, superType)` certifies that two types are equal, with the two types passed in that order.
111. It is not pass-introduced: the checker's `TypeEqualityWitness` `Val` lowers through `visitTypeEqualityWitness`.
112. The opcode is hoistable, so a given `subType` / `superType` pair resolves to a single witness inst.

### Notable opcodes — `builtinRequirementKey`

113. `BuiltinRequirementKey` is the requirement-key variant used for a requirement the front end recognizes as a built-in interface member — the `Differential` associated type, `dzero`, or `dadd` of `IDifferentiable`, for instance.
114. Where an ordinary `key` / `StructKey` is a distinct `global` symbol per requirement decl, unified across modules only through its `key_<mangled>` linkage name, a `builtinRequirementKey` is hoistable and its identity is its `kindOperand` (a `BuiltinRequirementKind` integer).
115. Two references to the same built-in role therefore resolve to one key inst by construction, even across the precompiled-core-module boundary, which is what makes a `lookupWitness` and its matching `witness_table_entry` agree without either side relying on entry order.
116. Because either key spelling can appear, `GetDispatcher`'s `lookupKey` operand and `getInterfaceRequirementKey`'s return type are both `IRInst`, not `IRStructKey`.

### Notable opcodes — `GetSequentialID` and the RTTI opcodes

117. `GetSequentialID` returns a stable `uint` index for its operand, which lets dynamic-dispatch lowering key a jump table by integer rather than by pointer comparison.
118. `visitIsTypeExpr` uses it for the optional-constraint form, whose surface has two halves — a constraint written `where optional T : IFoo` on the generic, plus a `T is IFoo` test in the body.
119. That pair lowers to a `GetSequentialID` on the constraint's witness-table parameter, compared for inequality against the all-ones "none" sentinel.
120. A run-time type test on an ordinary existential — `i is A` where `i` has interface type — takes the other arm of the same visitor and emits `IsType` instead, with no `GetSequentialID` involved.
121. The other caller, dynamic-dispatch lowering, assigns and reads per-table IDs through `Linkage::mapMangledNameToRTTIObjectIndex`.
122. `IRBuilder::emitMakeRTTIObject` exists but has no caller, so the runtime-type-info records the C++ and C-like emitters know how to print are unreachable at `source_commit`.

### Notable opcodes — the `*Set` opcodes and the set-theoretic types

123. The four set opcodes are the operands of the four set-theoretic types documented in `types.md`.
124. `UntaggedUnionType`, `ElementOfSetType` and `SetTagType` are not variadic — each takes exactly one `IRSetBase` operand, read with `getSet()`.
125. `TaggedUnionType` takes two operands and its operand order is the reverse of its own Lua comment: the witness-table set at operand 0 and the type set at operand 1.
126. A tagged-union value is therefore a tag of type `SetTagType(witnessTableSet)` paired with a payload of type `UntaggedUnionType(typeSet)` — exactly the pair `GetTagFromTaggedUnion` and `GetValueFromTaggedUnion` project back out.

### Notable opcodes — `packAnyValue` / `unpackAnyValue`

127. The result type of `packAnyValue` / `unpackAnyValue` carries the size of the erased blob.
128. Two unrelated lowerings use them: `Result<T, E>` legalization, which packs the error value into a fixed-size blob so both arms of a result share a representation, and the type-flow passes, which pack a concrete value into an `UntaggedUnionType` payload when a tagged union's type set is not a singleton.


## Functional coverage

| Claim | Intent | Anchor | Tests |
| --- | --- | --- | --- |
| C43, C113, C114: A struct conforming to IDifferentiable produces witness-table rows keyed by builtinRequirementKey insts carrying an integer BuiltinRequirementKind operand. | functional | [#builtinrequirementkey--builtinrequirementkey](../../../../design/ir-reference/generics-and-existentials.md#builtinrequirementkey--builtinrequirementkey) | [`builtin-requirement-key-idifferentiable.slang`](builtin-requirement-key-idifferentiable.slang) |
| C92, C93: A func-dependent derivative type lowers to FuncTypeOf(callable) — a Type-valued projection that names a callable without embedding it. | functional | [#dispatchers-and-existential-specialization](../../../../design/ir-reference/generics-and-existentials.md#dispatchers-and-existential-specialization) | [`func-type-of-fwd-diff.slang`](func-type-of-fwd-diff.slang) |
| C25, C26: A createExistentialObject whose interface has no conformance in the linkage stops code generation with E50100, "no type conformances found". | negative | [#existential-construction](../../../../design/ir-reference/generics-and-existentials.md#existential-construction) | [`create-existential-object-no-conformance-diagnostic.slang`](create-existential-object-no-conformance-diagnostic.slang) |
| C7, C27, C30: The core-module createDynamicObject builds an existential from a runtime type id, lowering to createExistentialObject(typeID, value). | functional | [#existential-construction](../../../../design/ir-reference/generics-and-existentials.md#existential-construction) | [`create-existential-object-dynamic-object.slang`](create-existential-object-dynamic-object.slang) |
| C33, C107, C109: Dispatching through an interface-typed value produces extractExistentialType(%i) whose result type is Type. | functional | [#existential-destructuring](../../../../design/ir-reference/generics-and-existentials.md#existential-destructuring) | [`extract-existential-type.slang`](extract-existential-type.slang) |
| C32, C107, C109: Dispatching through an interface-typed value produces extractExistentialValue(%i) reading the packed concrete-typed value. | functional | [#existential-destructuring](../../../../design/ir-reference/generics-and-existentials.md#existential-destructuring) | [`extract-existential-value.slang`](extract-existential-value.slang) |
| C34, C107, C109: Dispatching through an interface-typed value produces extractExistentialWitnessTable(%i) whose result type is witness_table_t(%I). | functional | [#existential-destructuring](../../../../design/ir-reference/generics-and-existentials.md#existential-destructuring) | [`extract-existential-witness-table.slang`](extract-existential-witness-table.slang) |
| C107: Interface dispatch through an existential follows the sequence extractExistentialType, extractExistentialValue, extractExistentialWitnessTable, lookupWitness, call. | functional | [#existential-destructuring](../../../../design/ir-reference/generics-and-existentials.md#existential-destructuring) | [`existential-dispatch-sequence.slang`](existential-dispatch-sequence.slang) |
| C108: Two dispatches through the same existential share a single hoistable extractExistentialType inst, while each dispatch keeps its own non-hoistable extractExistentialValue. | boundary | [#extractexistentialvalue--extractexistentialtype--extractexistentialwitnesstable](../../../../design/ir-reference/generics-and-existentials.md#extractexistentialvalue--extractexistentialtype--extractexistentialwitnesstable) | [`extract-existential-type-hoistable.slang`](extract-existential-type-hoistable.slang) |
| C108: Two dispatches through the same existential share a single hoistable extractExistentialWitnessTable inst that feeds both lookupWitness sites. | boundary | [#extractexistentialvalue--extractexistentialtype--extractexistentialwitnesstable](../../../../design/ir-reference/generics-and-existentials.md#extractexistentialvalue--extractexistentialtype--extractexistentialwitnesstable) | [`extract-existential-witness-table-hoistable.slang`](extract-existential-witness-table-hoistable.slang) |
| C12: A 5-deep nested Box<Box<Box<Box<Box<int>>>>> lowers to five nested specialize(%Box, ...) applications. | stress | [#generic-application](../../../../design/ir-reference/generics-and-existentials.md#generic-application) | [`stress-recursive-generic-depth-five.slang`](stress-recursive-generic-depth-five.slang) |
| C19: A conjunction constraint on a module-scope type_param emits one witness global_generic_param per half, so `T : IFoo & IBar` lowers to three module-scope parameters. | boundary | [#generic-application](../../../../design/ir-reference/generics-and-existentials.md#generic-application) | [`global-generic-param-conjunction-constraint.slang`](global-generic-param-conjunction-constraint.slang) |
| C18: A constrained module-scope type_param emits a second global_generic_param for the constraint's witness table alongside the one for the type. | boundary | [#generic-application](../../../../design/ir-reference/generics-and-existentials.md#generic-application) | [`global-generic-param-with-constraint.slang`](global-generic-param-with-constraint.slang) |
| C20, C21: A module that uses a module-scope type_param without a concrete binding is rejected with E38207, reported once per source declaration rather than once per lowered global_generic_param. | negative | [#generic-application](../../../../design/ir-reference/generics-and-existentials.md#generic-application) | [`global-generic-param-used-without-binding-diagnostic.slang`](global-generic-param-used-without-binding-diagnostic.slang) |
| C14, C17: A module-scope type_param T declaration lowers to let %T : Type = global_generic_param. | functional | [#generic-application](../../../../design/ir-reference/generics-and-existentials.md#generic-application) | [`global-generic-param.slang`](global-generic-param.slang) |
| C12: A reference to a generic struct at a concrete type produces a specialize(type, T) type-level value. | functional | [#generic-application](../../../../design/ir-reference/generics-and-existentials.md#generic-application) | [`specialize-generic-type.slang`](specialize-generic-type.slang) |
| C12: Doubly-nested instantiation Box<Box<int>> lowers to a nested specialize(%Box, specialize(%Box, Int)) type-level value. | boundary | [#generic-application](../../../../design/ir-reference/generics-and-existentials.md#generic-application) | [`recursive-generic-depth-two.slang`](recursive-generic-depth-two.slang) |
| C120: A run-time type test on an ordinary existential emits IsType and involves no GetSequentialID, unlike the optional-constraint form. | boundary | [#getsequentialid-and-the-rtti-opcodes](../../../../design/ir-reference/generics-and-existentials.md#getsequentialid-and-the-rtti-opcodes) | [`is-type-on-existential-no-sequential-id.slang`](is-type-on-existential-no-sequential-id.slang) |
| C56, C57, C117, C118, C119: An optional-constraint check lowers to GetSequentialID on the constraint's witness table, yielding a UInt compared against the sentinel none id. | functional | [#getsequentialid-and-the-rtti-opcodes](../../../../design/ir-reference/generics-and-existentials.md#getsequentialid-and-the-rtti-opcodes) | [`get-sequential-id-optional-constraint.slang`](get-sequential-id-optional-constraint.slang) |
| C22, C23, C24, C99: A method call on a generic-constrained value lowers to lookupWitness(witnessTable, requirementKey) with that two-operand shape. | functional | [#lookupwitness--lookupwitnessmethod](../../../../design/ir-reference/generics-and-existentials.md#lookupwitness--lookupwitnessmethod) | [`lookup-witness-operands.slang`](lookup-witness-operands.slang) |
| C22: Two distinct interface methods called on the same generic-constrained value produce two lookupWitness sites keyed by different requirement keys. | boundary | [#lookupwitness--lookupwitnessmethod](../../../../design/ir-reference/generics-and-existentials.md#lookupwitness--lookupwitnessmethod) | [`lookup-witness-multi-method-dispatch.slang`](lookup-witness-multi-method-dispatch.slang) |
| C103: A cast that crosses two inheritance steps feeds the transitively-derived witness table in as makeExistential's second operand. | functional | [#makeexistential](../../../../design/ir-reference/generics-and-existentials.md#makeexistential) | [`make-existential-transitive-witness.slang`](make-existential-transitive-witness.slang) |
| C28: A makeExistential whose concrete-type payload contains a vector field still produces a let %i : %IFoo with the two-operand value/witness shape. | boundary | [#makeexistential](../../../../design/ir-reference/generics-and-existentials.md#makeexistential) | [`make-existential-vector-payload.slang`](make-existential-vector-payload.slang) |
| C28: A makeExistential whose payload is a multi-field struct still emits the same two-operand let %i : %IFoo = makeExistential(value, witness) shape. | boundary | [#makeexistential](../../../../design/ir-reference/generics-and-existentials.md#makeexistential) | [`make-existential-struct-payload.slang`](make-existential-struct-payload.slang) |
| C28: Assigning a second conforming value into an existing interface-typed variable emits its own makeExistential carrying that value's witness. | boundary | [#makeexistential](../../../../design/ir-reference/generics-and-existentials.md#makeexistential) | [`make-existential-assign-into-opened.slang`](make-existential-assign-into-opened.slang) |
| C28, C101, C102: Casting a concrete value to an interface variable lowers to makeExistential(value, witness) with that two-operand shape. | functional | [#makeexistential](../../../../design/ir-reference/generics-and-existentials.md#makeexistential) | [`make-existential-operands.slang`](make-existential-operands.slang) |
| C101: The result type of makeExistential is the target interface type, not a wrapper type. | functional | [#makeexistential](../../../../design/ir-reference/generics-and-existentials.md#makeexistential) | [`make-existential-result-type.slang`](make-existential-result-type.slang) |
| C58: A non-default binding index reaches GetDynamicResourceHeap as its literal operand rather than being normalised to zero. | boundary | [#runtime-type-information](../../../../design/ir-reference/generics-and-existentials.md#runtime-type-information) | [`get-dynamic-resource-heap-nonzero-binding.slang`](get-dynamic-resource-heap-nonzero-binding.slang) |
| C7, C58, C59: The dynamic-resource-heap intrinsic lowers to GetDynamicResourceHeap(bindingIndex) whose result type is an unsized array of the descriptor type. | functional | [#runtime-type-information](../../../../design/ir-reference/generics-and-existentials.md#runtime-type-information) | [`get-dynamic-resource-heap-result-type.slang`](get-dynamic-resource-heap-result-type.slang) |
| C13: A 0-arg `<>` call to a single-type-param generic with no argument-driven inference is rejected. | negative | [#specialize](../../../../design/ir-reference/generics-and-existentials.md#specialize) | [`negative-zero-type-args-no-inference.slang`](negative-zero-type-args-no-inference.slang) |
| C12: A 0-constraint generic call lowers to specialize(%base, T) without any trailing witness operand. | boundary | [#specialize](../../../../design/ir-reference/generics-and-existentials.md#specialize) | [`specialize-no-constraint.slang`](specialize-no-constraint.slang) |
| C94: A call to a 2-type-param generic produces specialize(%base, T1, T2) — the argument list grows with arity. | boundary | [#specialize](../../../../design/ir-reference/generics-and-existentials.md#specialize) | [`specialize-two-type-params.slang`](specialize-two-type-params.slang) |
| C94: A call to a 4-type-param generic produces specialize(%base, T1, T2, T3, T4). | boundary | [#specialize](../../../../design/ir-reference/generics-and-existentials.md#specialize) | [`specialize-four-type-params.slang`](specialize-four-type-params.slang) |
| C2, C12, C13: A call to a generic function with a concrete type argument lowers to a call whose callee is a specialize(generic, T) opcode. | functional | [#specialize](../../../../design/ir-reference/generics-and-existentials.md#specialize) | [`specialize-generic-function.slang`](specialize-generic-function.slang) |
| C96, C97: A generic struct's conformance lowers to a generic witness table, so the witness argument at a call site is itself a specialize of that generic. | functional | [#specialize](../../../../design/ir-reference/generics-and-existentials.md#specialize) | [`specialize-generic-witness-table.slang`](specialize-generic-witness-table.slang) |
| C12: A generic taking an interface-typed parameter composes a specialize call site with a makeExistential argument. | stress | [#specialize](../../../../design/ir-reference/generics-and-existentials.md#specialize) | [`stress-specialize-with-existential-payload.slang`](stress-specialize-with-existential-payload.slang) |
| C12: A generic with a compile-time integer value parameter lowers to specialize(%base, N : Int) — the literal appears in the argument list. | boundary | [#specialize](../../../../design/ir-reference/generics-and-existentials.md#specialize) | [`specialize-value-param.slang`](specialize-value-param.slang) |
| C94: A generic with two interface constraints lowers to specialize(%base, T, %w1, %w2) — one witness operand per constraint. | boundary | [#specialize](../../../../design/ir-reference/generics-and-existentials.md#specialize) | [`specialize-multi-constraint.slang`](specialize-multi-constraint.slang) |
| C12: A specialize on an interface-constrained generic passes the type argument followed by the conforming witness table. | functional | [#specialize](../../../../design/ir-reference/generics-and-existentials.md#specialize) | [`specialize-with-witness-arg.slang`](specialize-with-witness-arg.slang) |
| C94: An 8-type-parameter generic call still lowers to specialize(%base, T1..T8) with eight argument operands. | stress | [#specialize](../../../../design/ir-reference/generics-and-existentials.md#specialize) | [`stress-eight-type-params.slang`](stress-eight-type-params.slang) |
| C95: Five textually identical specialize(%base, Int) call sites all share the same hoistable callee value. | stress | [#specialize](../../../../design/ir-reference/generics-and-existentials.md#specialize) | [`stress-many-specialize-call-sites.slang`](stress-many-specialize-call-sites.slang) |
| C13: Specializing a constrained generic with a non-conforming type is rejected before lowering — no specialize IR is produced. | negative | [#specialize](../../../../design/ir-reference/generics-and-existentials.md#specialize) | [`negative-constraint-not-satisfied.slang`](negative-constraint-not-satisfied.slang) |
| C13: Specializing with an undefined type identifier is rejected at the check stage — no specialize IR is produced. | negative | [#specialize](../../../../design/ir-reference/generics-and-existentials.md#specialize) | [`negative-undefined-type-arg.slang`](negative-undefined-type-arg.slang) |
| C12, C95: The specialize opcode is hoistable; two textually-identical specialize references collapse to one value in the IR. | functional | [#specialize](../../../../design/ir-reference/generics-and-existentials.md#specialize) | [`specialize-hoistable.slang`](specialize-hoistable.slang) |
| C45, C110, C111, C112: A constraint discharged by a type-equality fact lowers to TypeEqualityWitness(subType, superType) typed witness_table_t of that type. | functional | [#typeequalitywitness](../../../../design/ir-reference/generics-and-existentials.md#typeequalitywitness) | [`type-equality-witness-operand-order.slang`](type-equality-witness-operand-order.slang) |
| C22: The result type of lookupWitness is the requirement's function type, including the receiver as the second parameter. | functional | [#witness-lookup](../../../../design/ir-reference/generics-and-existentials.md#witness-lookup) | [`lookup-witness-result-type.slang`](lookup-witness-result-type.slang) |
| C41: A 3-method interface emits three interface_req_entry instructions, one per method requirement. | boundary | [#witness-tables-and-witness-facts](../../../../design/ir-reference/generics-and-existentials.md#witness-tables-and-witness-facts) | [`interface-three-req-entries.slang`](interface-three-req-entries.slang) |
| C44, C100: A lookup that has to resolve a conformance on ThisType while the interface is still being defined goes through a thisTypeWitness, which stays abstract until specialization. | functional | [#witness-tables-and-witness-facts](../../../../design/ir-reference/generics-and-existentials.md#witness-tables-and-witness-facts) | [`this-type-witness-interface-lowering.slang`](this-type-witness-interface-lowering.slang) |
| C51: A lookupWitness is a first-class unevaluated value: it is bound in the enclosing generic's block and the function body calls that binding rather than a resolved callee. | functional | [#witness-tables-and-witness-facts](../../../../design/ir-reference/generics-and-existentials.md#witness-tables-and-witness-facts) | [`lookup-witness-unevaluated-value.slang`](lookup-witness-unevaluated-value.slang) |
| C41, C47, C48: A property requirement contributes one interface_req_entry per accessor rather than one for the property itself. | boundary | [#witness-tables-and-witness-facts](../../../../design/ir-reference/generics-and-existentials.md#witness-tables-and-witness-facts) | [`interface-req-entry-per-accessor.slang`](interface-req-entry-per-accessor.slang) |
| C39: A struct conforming to an interface with zero methods emits an empty witness_table (no witness_table_entry rows). | boundary | [#witness-tables-and-witness-facts](../../../../design/ir-reference/generics-and-existentials.md#witness-tables-and-witness-facts) | [`empty-interface-witness-table.slang`](empty-interface-witness-table.slang) |
| C39: A struct implementing an interface produces a witness_table whose operand 0 is the conforming concrete type, with the conformed interface carried by the result witness_table_t type. | functional | [#witness-tables-and-witness-facts](../../../../design/ir-reference/generics-and-existentials.md#witness-tables-and-witness-facts) | [`witness-table-type-shape.slang`](witness-table-type-shape.slang) |
| C50: A witness table is a key-to-value map, not a positional list: a conformer declaring its members in a different order than the interface still pairs each requirement key with its own satisfying value. | boundary | [#witness-tables-and-witness-facts](../../../../design/ir-reference/generics-and-existentials.md#witness-tables-and-witness-facts) | [`witness-table-three-method-rows.slang`](witness-table-three-method-rows.slang) |
| C42, C49: An associated-type bound is a sibling interface_req_entry carrying witness_table_t(bound), not an entry nested inside the associated type's own entry. | boundary | [#witness-tables-and-witness-facts](../../../../design/ir-reference/generics-and-existentials.md#witness-tables-and-witness-facts) | [`interface-req-entry-assoc-type-bound-sibling.slang`](interface-req-entry-assoc-type-bound-sibling.slang) |
| C41: An interface declaration appears as let %I : Type = interface(%req...) whose operands are the interface_req_entry value-ids. | functional | [#witness-tables-and-witness-facts](../../../../design/ir-reference/generics-and-existentials.md#witness-tables-and-witness-facts) | [`interface-lists-req-entries.slang`](interface-lists-req-entries.slang) |
| C41: An interface requirement lowers to interface_req_entry(requirementKey, requirementFuncType) — first operand is the key, second is the function type. | functional | [#witness-tables-and-witness-facts](../../../../design/ir-reference/generics-and-existentials.md#witness-tables-and-witness-facts) | [`interface-req-entry-operands.slang`](interface-req-entry-operands.slang) |
| C41: An interface with zero requirements lowers to an interface opcode with no interface_req_entry operands. | boundary | [#witness-tables-and-witness-facts](../../../../design/ir-reference/generics-and-existentials.md#witness-tables-and-witness-facts) | [`empty-interface-no-req-entries.slang`](empty-interface-no-req-entries.slang) |
| C40: Each row inside a witness_table body is witness_table_entry(requirementKey, satisfyingVal) — first operand is the key, second is the satisfying value. | functional | [#witness-tables-and-witness-facts](../../../../design/ir-reference/generics-and-existentials.md#witness-tables-and-witness-facts) | [`witness-table-entry-operands.slang`](witness-table-entry-operands.slang) |
| C41: The function type carried in an interface_req_entry uses this_type(%I) for the receiver parameter. | functional | [#witness-tables-and-witness-facts](../../../../design/ir-reference/generics-and-existentials.md#witness-tables-and-witness-facts) | [`interface-req-this-type.slang`](interface-req-this-type.slang) |
| C52, C53, C54: When an interface requirement is itself generic the lookup yields a Generic-typed value that the use site wraps in a specialize. | functional | [#witness-tables-and-witness-facts](../../../../design/ir-reference/generics-and-existentials.md#witness-tables-and-witness-facts) | [`lookup-witness-generic-requirement-specialize.slang`](lookup-witness-generic-requirement-specialize.slang) |

## Untested claims

| Claim | Reason | Anchor | Why untested |
| --- | --- | --- | --- |
| C61, C62, C63, C127, C128: `packAnyValue` and `unpackAnyValue` move values across the `AnyValueType` boundary, with the erased blob size carried by the result type, and are used by `Result<T, E>` legalization and by the type-flow passes. | link-stage-only | [#anyvalue-marshalling](../../../../design/ir-reference/generics-and-existentials.md#anyvalue-marshalling) | Both are introduced by `Result<T, E>` legalization and by the type-flow passes; neither runs before the LOWER-TO-IR snapshot. |
| C115: A `key` / `StructKey` is unified across modules only through its `key_<mangled>` linkage name, whereas a `builtinRequirementKey` is identical by construction even across the precompiled-core-module boundary. | needs-multi-file-test | [#builtinrequirementkey--builtinrequirementkey](../../../../design/ir-reference/generics-and-existentials.md#builtinrequirementkey--builtinrequirementkey) | The cross-module half of the claim needs two translation units linked together; a single `.slang` file cannot show two independently-produced keys converging. |
| C116: Because either key spelling can appear, `GetDispatcher`'s `lookupKey` operand and `getInterfaceRequirementKey`'s return type are both `IRInst` rather than `IRStructKey`. | internal-source-fact | [#builtinrequirementkey--builtinrequirementkey](../../../../design/ir-reference/generics-and-existentials.md#builtinrequirementkey--builtinrequirementkey) | A C++ static-type choice. The dump prints the key operand identically whichever spelling reaches it, so no directive distinguishes the declared parameter type. |
| C89, C90, C91: `SpecializeExistentialsInFunc`, `SpecializeExistentialsInType` and `WeakUse` carry existential bindings and non-pinning uses through specialization. | link-stage-only | [#dispatchers-and-existential-specialization](../../../../design/ir-reference/generics-and-existentials.md#dispatchers-and-existential-specialization) | Introduced by the specialization and type-flow passes; they do not exist in the LOWER-TO-IR block. |
| C6, C29, C37, C38, C55, C86, C87, C88, C122: `rtti_object`, `makeExistentialWithRTTI`, `extractTaggedUnionTag`, `extractTaggedUnionPayload`, `UnboundedGenericElement`, `GetDispatcher` and `GetSpecializedDispatcher` each have an `IRBuilder` emitter that nothing calls. | implementation-detail | [#existential-construction](../../../../design/ir-reference/generics-and-existentials.md#existential-construction) | The doc states these have no producer at all at `source_commit`, so no input — surface Slang or IR pass — can make one appear in a dump. Only a C++ test that calls the emitter directly could observe their shape. |
| C35: `getValueFromBoundInterface` reads the concrete-typed value out of a `BoundInterface<I, T, w>` value. | link-stage-only | [#existential-destructuring](../../../../design/ir-reference/generics-and-existentials.md#existential-destructuring) | Its only producer is `IRBuilder::emitWrapExistential`, so it appears only alongside `wrapExistential` after existential binding — not at lowering. |
| C36: `isNullExistential` is true when an existential holds the null placeholder, and lowers `Optional<ISomeInterface>`'s has-value test. | link-stage-only | [#existential-destructuring](../../../../design/ir-reference/generics-and-existentials.md#existential-destructuring) | Produced by optional-type lowering, an IR pass downstream of the observation point; the `Optional<I>` surface at lowering still carries the optional type rather than the null test. |
| C1, C3, C4, C5, C9, C10, C11: The enumerator-naming rule (`kIROp_` + `struct_name`), the `to_pascal_case` fallback, the hand-written-versus-FIDDLE wrapper split, the per-operand accessor generation, and the `†` / `‡` / `H` / `P` / `G` table markers. | internal-source-fact | [#generic-application](../../../../design/ir-reference/generics-and-existentials.md#generic-application) | These are facts about how the C++ header and the doc's own tables are produced. The dump prints the Lua mnemonic regardless of how a wrapper was generated, so no compiled program distinguishes them. |
| C15: `bind_global_generic_param` binds a global generic parameter to a concrete value when a specialized component type is generated. | needs-unit-test | [#generic-application](../../../../design/ir-reference/generics-and-existentials.md#generic-application) | The opcode is emitted only while a specialized component type is built through the compilation API; there is no `slangc` command line that requests one. A C++ unit test driving `ISession` specialization and inspecting the resulting module would verify it. |
| C16: `globalValueRef` is a non-hoistable pin that keeps a global value referenced from inside a function body so dependent computation can be emitted locally. | link-stage-only | [#generic-application](../../../../design/ir-reference/generics-and-existentials.md#generic-application) | The opcode is introduced by global-value legalization, which runs long after the LOWER-TO-IR snapshot this bundle observes, so no natural surface produces it at this stage. |
| C8: `SetBase` is the only abstract Lua parent entry in this family, and the other boxes in the hierarchy diagram group opcodes for the reader without corresponding to Lua entries. | internal-source-fact | [#generic-application](../../../../design/ir-reference/generics-and-existentials.md#generic-application) | An abstract Lua entry generates no opcode, so nothing in any IR dump reports whether a grouping is a real parent or a reading aid. |
| C100: A `lookupWitness` whose table is a `thisTypeWitness` stays abstract, with `isAbstractWitnessTable` recursing through nested lookups. | implementation-detail | [#lookupwitness--lookupwitnessmethod](../../../../design/ir-reference/generics-and-existentials.md#lookupwitness--lookupwitnessmethod) | "Stays abstract" is a decision inside witness resolution; the dump spelling of a recursive nested lookup is the same as any other, so no directive distinguishes the recursive case. |
| C60, C121: `lowerDynamicResourceHeap` replaces `GetDynamicResourceHeap` with a laid-out `global_param` so the opcode never reaches emit, and dynamic-dispatch lowering assigns per-table IDs through `Linkage::mapMangledNameToRTTIObjectIndex`. | link-stage-only | [#runtime-type-information](../../../../design/ir-reference/generics-and-existentials.md#runtime-type-information) | Both are the output of later IR passes; this bundle pins the two opcodes at lowering, where they still exist in their lowering-time form. |
| C64, C65, C66, C68, C69, C70, C71, C72: The `*Set` opcodes and their `Unbounded*` / `Uninitialized*` / `None*` elements form the closed sets that type-flow analysis discovers, with set equality being inst identity. | link-stage-only | [#sets-and-set-elements](../../../../design/ir-reference/generics-and-existentials.md#sets-and-set-elements) | Every listed producer is the type-flow specialization pass, which runs post-specialization. A test would need a `-dump-ir-after` snapshot of that pass, which the bundle deliberately avoids as unstable. |
| C67: `IRBuilder::getSet` maintains the set invariants (at least one operand, concrete non-set operands, consistent operand order) through a persistent unique-ID map, so sets should never be built operand-by-operand. | needs-unit-test | [#sets-and-set-elements](../../../../design/ir-reference/generics-and-existentials.md#sets-and-set-elements) | This is a constraint on how compiler code must call the builder, not a property of any compiled program. A C++ unit test asserting `getSet` normalisation would verify it. |
| C73, C74: A witness-table set reaching an `extractExistentialWitnessTable` that carries an `Uninitialized*` element is reported as `E50101`, so an interface local must be assigned on every path that reaches a dynamic dispatch on it. | (unclassified) | [#sets-and-set-elements](../../../../design/ir-reference/generics-and-existentials.md#sets-and-set-elements) | Testable as a `DIAGNOSTIC_TEST` pinning `E50101` on a conditionally-assigned interface local, but the bundle is at its 60-file `size_cap_files` limit and the four free slots went to `E38207`, `E50100`, the conjunction-constraint shape and the `IsType` carve-out. Raise the cap to add it. |
| C98: A `generic` body ends in `return_val`, not `yield`. | out-of-bundle | [#specialize](../../../../design/ir-reference/generics-and-existentials.md#specialize) | The doc hands the `generic` terminator off to `structure.md` and `control-flow.md`; `design/ir-reference/control-flow` owns the `return_val` / `yield` terminator distinction. |
| C75, C76, C77, C78, C79, C80, C81, C82, C83, C84, C85: The tagged-union and tag-translation opcodes replace witness-driven dispatch with tag-driven dispatch over a closed set. | link-stage-only | [#tagged-unions-and-tag-operations](../../../../design/ir-reference/generics-and-existentials.md#tagged-unions-and-tag-operations) | All of them are emitted by the type-flow specialization pass or by dynamic-dispatch lowering, both downstream of lowering. |
| C123, C124, C125, C126: The four set opcodes are the operands of the four set-theoretic types, `UntaggedUnionType` / `ElementOfSetType` / `SetTagType` take exactly one `IRSetBase` operand, and `TaggedUnionType` stores the witness-table set at operand 0 and the type set at operand 1 — the reverse of its own Lua comment. | out-of-bundle | [#the-set-opcodes-and-the-set-theoretic-types](../../../../design/ir-reference/generics-and-existentials.md#the-set-opcodes-and-the-set-theoretic-types) | These are claims about type opcodes, which `types.md` owns and the sibling bundle `design/ir-reference/types` covers; this bundle tests only the value opcodes. |
| C39, C40, C43, C44, C45: The structural definitions (operand shapes, flags and AST origins) of `witness_table`, `witness_table_entry`, `interface_req_entry`, `key` / `StructKey`, `thisTypeWitness`, `TypeEqualityWitness` and `indexedFieldKey` belong to `structure.md`. | out-of-bundle | [#witness-tables-and-witness-facts](../../../../design/ir-reference/generics-and-existentials.md#witness-tables-and-witness-facts) | This page carries only the dispatch-side reading of those opcodes, which is what this bundle tests; the parent-child structural shape is covered by the sibling bundle `design/ir-reference/structure`. |
| C46: `indexedFieldKey` is a placeholder key for a tuple-like field and is not part of interface dispatch. | out-of-bundle | [#witness-tables-and-witness-facts](../../../../design/ir-reference/generics-and-existentials.md#witness-tables-and-witness-facts) | The doc explicitly excludes it from the dispatch path, and its structural row lives in `structure.md`; the sibling bundle `design/ir-reference/structure` owns it. |
| C31, C104, C105, C106: `wrapExistential` converts a `BindExistentials<T, ...>` value back to `T` so an unspecialized callee can be invoked, reads its `(type, witness)` slot pairs after operand 0, and short-circuits to the value itself with zero slot arguments or to a `makeExistential` when the target is an interface type. | link-stage-only | [#wrapexistential](../../../../design/ir-reference/generics-and-existentials.md#wrapexistential) | The opcode is introduced by the existential-binding and specialization passes, both of which run after the LOWER-TO-IR block this bundle FileChecks. |

## Doc gaps observed

(none) — every gap this bundle previously reported (the missing `type_param` surface, the unreported `E38207`, the `-conformance` / `E50100` requirement behind `createDynamicObject`, the absent witness-table `specialize` example, the property-requirement entry list, the unnamed associated-type-bound key, the `Generic` result type of a generic-requirement lookup, the two-part optional-constraint surface, and the reachability of `FuncTypeOf`) is now written into the source doc, and re-running each observation against the compiler at `source_commit` reproduced what the doc now says.
