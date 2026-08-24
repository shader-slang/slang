//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#builtin-operators-bypass-call-lowering
//META: doc_section_digest=84a14c8c06fb62977462b222065d70ef81479ad93846db3d8c9e968027e956d2
//META: purpose=An array extent `N / 2` on a generic value parameter arrives as a `BuiltinOperationIntVal` and lowers to the hoistable `constexprDiv(%N, 2 : Int)` opcode keyed on its BuiltinOperationKind.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the compile-time `Val`-side sibling of the builtin-operator fast
// path: arithmetic on a generic *value* parameter whose result is needed as a
// type-level value lowers to a `constexpr*` inst rather than to a runtime `div`.
// The array extent `N / 2` inside `f<let N : int>` is the surface that reaches
// it — `/` is one of the operators that forms a `BuiltinOperationIntVal`. The
// CHECK pins `constexprDiv(%N{{[0-9]*}}, 2 : Int)`, i.e. the generic parameter
// itself as the first operand, which is only possible if the extent stayed
// symbolic through lowering instead of being folded against a concrete `N`.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

RWStructuredBuffer<int> outputBuffer;

int f<let N : int>()
{
    int a[N / 2];
    a[0] = 1;
    return a[0];
}

[numthreads(1, 1, 1)]
void main()
{
    outputBuffer[0] = f<8>();
}

// The generic body keeps the extent as a symbolic compile-time expression.
// CHECK: constexprDiv(%N{{[0-9]*}}, 2 : Int)
