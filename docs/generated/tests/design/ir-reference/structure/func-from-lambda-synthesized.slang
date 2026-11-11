//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/structure.md#functions-and-generics
//META: doc_section_digest=d9c6cf4f837d25ae755b0e257d6cddda3fedea83b491aa0f828dd84b3c990065
//META: purpose=The synthesized FuncDecl that checking stores on a lambda lowers to an ordinary func whose first parameter is the lambda's capture struct.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a lambda expression reaches the IR as a plain `func`, not as a
// distinct closure opcode. The lambda is passed through an `IFunc`-constrained
// helper so it survives to lowering; CHECK matches the synthesized call
// operator's name hint and requires the next line to be a `func` header whose
// `Func(...)` type takes the lambda's capture struct followed by the declared
// parameter.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o - -entry main -stage compute

uniform int seed;
uniform RWStructuredBuffer<int> buf;

int apply<F : IFunc<int, int>>(F f, int x) { return f(x); }

[numthreads(1, 1, 1)]
void main()
{
    let g = (int y) => y * 3;
    buf[0] = apply(g, seed);
}

// CHECK: [nameHint("main._slang_Lambda{{.*}}.()")]
// CHECK-NEXT: func %{{[A-Za-z0-9_]+}}	: Func(Int, %{{[A-Za-z0-9_]+}}, Int)
