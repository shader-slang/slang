# Entry Point Signature Validation

This document describes the entry-point signature validation system in the Slang
compiler frontend. The system catches invalid types in entry-point parameters and
return types during semantic checking, before IR generation, preventing crashes
and invalid code generation downstream.

## Motivation

Without frontend validation, certain types can appear as entry-point varying
parameters or return types and cause the compiler to either crash (segfault in IR
passes like `legalizeEntryPointsForGLSL`) or silently generate invalid SPIR-V.
The validation system rejects these cases with clear error diagnostics.

## Architecture

Validation happens at two sites in the frontend:

### Site 1: Entry-Point Varying Type Validation

**Location**: `validateEntryPoint()` in `source/slang/slang-check-shader.cpp`

After the existing return-type checks (resource types, array types), a new
validation pass walks each varying parameter type and the return type, checking
them against a **data-driven rule table**.

#### Rule Table

Each rule specifies:

- A **type predicate**: a function that returns `true` if the type matches (is
  invalid)
- A **reason string**: human-readable explanation for the diagnostic
- An optional **target predicate**: when non-null, the rule only applies to
  targets matching this predicate (e.g., SPIR-V only)

```
Rule                    Type Predicate                          Target
----                    --------------                          ------
DifferentialPair<T>     as<DifferentialPairType>                all
Atomic<T>               as<AtomicType>                          all
CoopVec<T, N>           as<CoopVectorExpressionType>            all
CoopMat<T,S,M,N,R>     IntrinsicTypeModifier with              all
                        irOp == kIROp_CoopMatrixType
vector<bool>            VectorExpressionType with               SPIR-V only
                        Bool element type
```

New rules are added by appending entries to the static array. No other code
changes are needed to add a new "type X is never valid as a varying" rule.

#### Recursive Type Traversal

The type walker recursively descends into:

- **Struct fields**: If a struct is used as a varying parameter, each field's
  type is checked against the rules. This catches cases like
  `struct Foo { DifferentialPair<float> x; }` used as a parameter.
- **Array element types**: `DifferentialPair<float>[4]` is also caught.
- **ModifiedType wrappers**: Unwrapped transparently.

Cycle detection prevents infinite recursion on recursive struct types.

#### Uniform vs. Varying

Only varying parameters are checked. Parameters that are uniform (explicit
`uniform` modifier, or resource types like textures/samplers/buffers detected by
`isUniformParameterType()`) are skipped. An `Atomic<T>` inside a constant buffer
is valid; it only fails as a varying input/output.

#### Target-Specific Rules

For rules with a target predicate, the validation checks all compilation targets
in `linkage->targets`. If any target matches the predicate, the error is emitted.
This handles the case where a user compiles for both SPIR-V and HLSL -- the
SPIR-V restriction still applies.

### Site 2: Matrix Type Validation

**Location**: `CoerceToUsableType()` in `source/slang/slang-check-type.cpp`

Matrix types are validated at type-construction time (not just at entry points)
because invalid matrix dimensions and element types produce broken output
regardless of where they appear.

#### Dimension Validation

Row count and column count are checked when they are compile-time constants
(`ConstantIntVal`). Values > 4 are rejected. Non-constant dimensions (generic
parameters) are not validated here -- they will be checked when the generic is
instantiated with concrete values.

#### Element Type Validation

The following element types are supported for matrices:

- `float`, `half`, `double`
- `int` (32-bit), `uint` (32-bit)

The following are rejected:

- `int8_t`, `int16_t`, `int64_t`
- `uint8_t`, `uint16_t`, `uint64_t`
- `bool`

## Diagnostics

| Code  | Name | Description |
|-------|------|-------------|
| 38050 | `invalid-entry-point-varying-type` | Type cannot be used as entry-point varying parameter or return type |
| 38051 | `invalid-entry-point-varying-type-for-target` | Type cannot be used as entry-point varying for a specific target |
| 38206 | `matrix-dimension-out-of-range` | Matrix row or column count is outside 1..4 |

## Extending the System

### Adding a new "never valid as varying" type

Add an entry to `kEntryPointVaryingTypeRules[]` in `slang-check-shader.cpp`:

```cpp
{
    [](Type* type) -> bool { return as<YourType>(type) != nullptr; },
    "YourType is not a valid varying type",
    nullptr,  // all targets, or a target predicate
},
```

### Adding a target-specific restriction

Same as above, but provide a target predicate:

```cpp
{
    [](Type* type) -> bool { /* match logic */ },
    "reason message",
    [](CodeGenTarget target) -> bool { return isSPIRV(target); },
},
```

## Future Work

The senior developer's comment on issue #9759 outlines a more comprehensive
system with per-stage validation objects, semantic-level checking, and attribute
validation. This first pass addresses the immediate crashes and invalid codegen
by rejecting known-bad types. The rule table design is compatible with the
broader vision -- rules could later be made stage-aware by adding a stage
predicate field.
