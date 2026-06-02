//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#specialization-and-generics
//META: doc_section_digest=80bc5973053356e62dbd6f6ded27b7e93976729f33655c137440d5c954689e89
//META: purpose=The specialize-arrays pass resolves a fixed-size-array generic parameter so the emit reflects the concrete N.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The "Specialize arrays" row of the specialization / generics
// table (`slang-ir-specialize-arrays.cpp`) specializes array-typed
// generic parameters. A function generic over `let N : int` is
// observably resolved when called with `N = 4`: the inlined / emit
// text contains the literal index range.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage compute -entry main

uniform RWStructuredBuffer<int> buf;

[ForceInline]
int sumN<let N : int>(int data[N])
{
    int s = 0;
    [ForceUnroll] for (int i = 0; i < N; ++i) s += data[i];
    return s;
}

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int arr[4] = { int(tid.x), int(tid.y), int(tid.z), 0 };
    buf[0] = sumN<4>(arr);
}

// HLSL: void main
// HLSL-NOT: sumN
