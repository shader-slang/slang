//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:50:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/llm-generated/ir-reference/types.md#arrays
//META: doc_section_digest=fa1a8caf9cb2dd874328b2b826017621c7f1df8e509df2e908e25b520003a421
//META: purpose=Stress: a large `static const`-bounded array surfaces the full extent as Array(Float, 256 : Int).
//META: intent=stress
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's "Arrays" row says `elementCount` is "typically an
// integer literal". A `static const int N = 256;` used as the
// extent is folded to the integer literal `256 : Int` on the
// `Array` opcode.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

static const int N = 256;
struct Big { float data[N]; }
uniform Big b;
uniform RWStructuredBuffer<float> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = b.data[N - 1];
}

// IR-LABEL: struct %Big
// IR: field(%{{.*}}, Array(Float, 256 : Int))
