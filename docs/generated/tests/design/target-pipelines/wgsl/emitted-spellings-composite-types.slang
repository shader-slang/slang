//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/target-pipelines/wgsl.md#emitted-spellings
//META: doc_section_digest=a72c747d5e5fa9684a460e1b5d345a74da1aee6a2a789ea6d65d147ba60563d3
//META: purpose=Composite type spellings on WGSL: vector<T,N> becomes vecN<T>, a one-element vector collapses to the bare scalar type, matrix<T,R,C> becomes matRxC<T>, a sized array becomes array<T, i32(N)> and an unsized one array<T>.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// One test per composite branch the doc's spelling table enumerates. They are
// grouped in one file because each produces a distinct, non-colliding token, so
// the CHECK lines cannot match each other's output.
//
// Two of these rows are the ones a reader is most likely to get wrong:
//
//   - `vector<T,1>` does not become `vec1<T>`; it collapses to the bare scalar
//     type, so `float1` is indistinguishable from `float` in the emitted text.
//   - `T[N]` becomes `array<T, i32(N)>` — the extent is itself an `int` literal,
//     so it takes the constructor spelling too rather than appearing as a bare
//     count.
//
// The array constant is module-scope so it survives as a declaration, and the
// matrix and vectors are fed from the thread id so they are not folded away.
// The two-element vector is additionally *stored whole* into a `float2` buffer:
// reading only `v2.x` lets later simplification forward the component to its
// use, and the `vec2<f32>` spelling never reaches the text at all.

//TEST:SIMPLE(filecheck=CHECK):-target wgsl -entry main -stage compute

static const int gArr[4] = { 1, 2, 3, 4 };
RWStructuredBuffer<float> outBuf;
RWStructuredBuffer<float2> outVec;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    float f = 1.5f * float(tid.x);
    float2 v2 = float2(f, -f);
    float1 v1 = float1(f);
    float3x4 m = float3x4(f, f, f, f, f, f, f, f, f, f, f, f);
    outVec[0] = v2;
    outBuf[0] = v1.x + m[0][0] + float(gArr[int(tid.y) & 3]);
}

// A sized array keeps a constructor-spelled extent; the storage buffer's
// unsized element array has none.
// CHECK-DAG: array<i32, i32(4)>
// CHECK-DAG: array<f32>
// vecN<T> for N > 1, and matRxC<T> with the two digits in source order.
// CHECK-DAG: vec2<f32>
// CHECK-DAG: mat3x4<f32>
// A one-element vector is spelled as the bare scalar, so no vec1 is emitted.
// CHECK-NOT: vec1<
