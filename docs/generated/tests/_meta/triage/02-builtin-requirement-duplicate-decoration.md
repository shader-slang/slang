# 02 — `builtinRequirementKey` carries its role decoration twice

**Verdict: new-finding.** Filed as
`docs/generated/tests/_meta/findings/builtin-requirement-decoration-added-twice.yaml`.

One defect explains three of the expected-failure entries:

- `design/ir-reference/decorations/builtin-requirement-key-decoration.slang`
- `design/cross-cutting/ir-instructions/decoration-builtin-requirement-key-ir.slang`
- `design/ir-reference/differentiation/builtin-requirement-decoration.slang`

All three assert a `CHECK-NEXT` relationship between the decoration and the
`builtinRequirementKey` line, and all three break the same way.

## Evidence

Eleven lines are enough to reproduce:

```slang
[Differentiable]
float f(float x) { return x * x; }
RWStructuredBuffer<float> gOut;
[shader("compute")][numthreads(1,1,1)]
void main() { gOut[0] = f(2.0); }
```

```
$ build/Release/bin/slangc repro.slang -target spirv-asm -dump-ir -o /dev/null \
    -entry main -stage compute
### LOWER-TO-IR:
Poison
[BuiltinRequirementDecoration(25 : Int)]
[BuiltinRequirementDecoration(25 : Int)]
let  %1 : _ = builtinRequirementKey(25 : Int)
[BuiltinRequirementDecoration(26 : Int)]
[StaticRequirementDecoration]
[BuiltinRequirementDecoration(26 : Int)]
let  %2 : _ = builtinRequirementKey(26 : Int)
```

The failure surfaces as:

```
CHECK-NEXT: is not on the line after the previous match
  // CHECK-NEXT: {{.*}}= builtinRequirementKey([[KIND]] : Int)
note: non-matching line after previous match is here
  [BuiltinRequirementDecoration(25 : Int)]
```

## Why this is the compiler and not the tests

Two observations rule out a dump artifact:

1. For kind 26 an unrelated `StaticRequirementDecoration` sits *between* the
   two copies. A repeated print of a single decoration could not interleave a
   third one.
2. Both decoration lines precede one `let ... = builtinRequirementKey(...)`
   line, and the dump prints an inst's decorations immediately before it — so
   these are two decorations on one inst, not two insts.

The duplication is present in the first `-dump-ir` snapshot (`LOWER-TO-IR`),
which places the producer in AST-to-IR lowering, before linking or
specialization.

## What makes it surprising

The single call site is already guarded, and its comment says exactly why:

```cpp
// Also tag the role as a decoration so role-scanning consumers (autodiff's
// `getInterfaceEntryByBuiltinRequirement`) work unchanged. The key is
// shared, so add the decoration only once.
if (!requirementKey->findDecoration<IRBuiltinRequirementDecoration>())
    builder->addBuiltinRequirementDecoration(requirementKey, (IRIntegerValue)builtinRole);
```
— `source/slang/slang-lower-to-ir.cpp:1817-1820`

`addBuiltinRequirementDecoration` has exactly one caller in the tree
(confirmed by grep over `source/`), so a second *add* is not the obvious
mechanism. The more likely one is a path that copies decorations onto the
shared key — cloning, or a specialization/linking helper that runs during
lowering. That was not pinned down here and is left for whoever picks the
finding up.

## Impact

Not merely cosmetic. Consumers scan for the role decoration
(`getInterfaceEntryByBuiltinRequirement` in autodiff); a duplicate is at best
redundant work and at worst a double-visit. It also makes any `CHECK-NEXT`
assertion against the key impossible to write, which is what these three
tests were trying to do.
