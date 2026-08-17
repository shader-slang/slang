//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/ir-instructions.md#value-instructions
//META: doc_section_digest=f9ced424f75b0995671347e9ab72c4c596438a42cdf6bd9ecf359a2b13da7f4e
//META: purpose=Arithmetic on a generic value parameter lowers to the compile-time-folded `constexprAdd` variant rather than the ordinary runtime `add`.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the `constexprAdd ... constexprEnumCast` value row, whose note
// calls these the compile-time-folded arithmetic variants with 1-3 fixed
// operands. The surface that reaches them is arithmetic on a generic value
// parameter: `int b[N + 1]` inside `wrap<let N : int>` needs `N + 1` as a
// type-level value, so it cannot be an ordinary `add` on a runtime SSA
// value. The CHECK asserts the dump shows `constexprAdd(1 : Int, %N)` — the
// distinct opcode with the generic parameter as an operand — which is what
// distinguishes the type-level arithmetic from the value-level one.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform RWStructuredBuffer<int> outbuf;

int sum<let N : int>(int arr[N])
{
    int s = 0;
    for (int i = 0; i < N; ++i)
        s += arr[i];
    return s;
}

int wrap<let N : int>(int arr[N])
{
    int b[N + 1];
    for (int i = 0; i < N; ++i)
        b[i] = arr[i];
    b[N] = 1;
    return sum<N + 1>(b);
}

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int a[3] = { 1, 2, 3 };
    outbuf[0] = wrap<3>(a);
}

// IR: = constexprAdd(1 : Int, %N{{[0-9]*}})
