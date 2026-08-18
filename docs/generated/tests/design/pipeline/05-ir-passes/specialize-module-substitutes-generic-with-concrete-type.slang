//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#specialization-and-generics
//META: doc_section_digest=2f10610b6a8497f9b2cec62d411162a4585b32e9698a95e5288845967ffba16d
//META: purpose=The specialize pass substitutes generic parameters with concrete types — before the pass the generic function is generic-shaped, after the pass it carries a concrete signature.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `pickFirst<T>` is a generic function called with `T = int`. Before
// `specializeModule` the IR carries a generic function shape (the
// function's type is over generic type parameters); after the pass
// the function carries a concrete `Func(Int, Int, Int)` signature.
// This is the main specialization driver in the
// `slang-ir-specialize.cpp` row of the doc table.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir-before specializeModule -dump-ir-after specializeModule -o /dev/null -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;

T pickFirst<T>(T x, T y) { return x; }

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = pickFirst(a, a + 1);
}

// Before the pass: `pickFirst` is generic (Func over type variables).
// CHECK-LABEL: ### BEFORE specializeModule:
// CHECK: func %pickFirst{{[^:]*}}: Func(%

// After the pass: `pickFirst` carries a concrete `Int, Int, Int` signature.
// CHECK-LABEL: ### AFTER specializeModule:
// CHECK: func %pickFirst{{[^:]*}}: Func(Int, Int, Int)
