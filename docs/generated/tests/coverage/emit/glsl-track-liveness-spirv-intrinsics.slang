//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Pins that `-track-liveness` on the GLSL target turns live-range markers into declared `spirv_instruction` functions (`livenessStart_N` / `livenessEnd_N`, ids 256 and 257) called around each local's live range.
//META: intent=characterization
//META: covers=source/slang/slang-emit-glsl.cpp
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterizes how the GLSL back end renders liveness information under
// `-track-liveness`. GLSL has no syntax for a live-range marker, so the emitter
// declares a forward-declared function per marked type carrying the
// `GL_EXT_spirv_intrinsics` `spirv_instruction(id = ...)` decoration -- 256 for
// `OpLifetimeStart`, 257 for `OpLifetimeStop` -- and emits ordinary calls to those
// at the range boundaries. Because the declaration is per *type*, a shader with three
// distinct local types produces three `livenessStart_N` overloads and three
// `livenessEnd_N` overloads; the parameters use the extension's `spirv_by_reference`
// and `spirv_literal` qualifiers so the call lowers to the right SPIR-V operands.
// The CHECKs pin the extension line, both intrinsic ids paired with their declaration,
// the by-reference/literal parameter qualifiers, and one actual call site.
//
// The shader deliberately declares a struct local, an int loop counter and a float
// accumulator so that more than one marker type exists, and it uses a loop body whose
// temporary has a live range strictly inside the function, which is what makes the
// end markers interesting. Values are sunk into a buffer so nothing is dead-stripped.
//
// This is a GLSL-only observation: on the non-Khronos targets (`cpp`, `cuda`, `hlsl`)
// the same `-track-liveness` compile emits no marker at all, which is recorded in the
// bundle README's `## Unreachable gaps` as the reason
// `CLikeSourceEmitter::emitLivenessImpl` stays uncovered.
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry computeMain -stage compute -track-liveness

struct Big { float4 a; float4 b; };

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    Big v;
    v.a = float4(float(tid.x), 1, 2, 3);
    v.b = float4(4, 5, 6, float(tid.y));
    float s = 0;
    for (int i = 0; i < 4; ++i)
    {
        Big t;
        t.a = v.a * float(i);
        t.b = v.b + float(i);
        s += t.a.x + t.b.w;
    }
    outputBuffer[0] = s;
}

// The marker functions are declared through the SPIR-V intrinsics extension.
//GLSL-DAG: #extension GL_EXT_spirv_intrinsics : require
// id 256 is OpLifetimeStart, id 257 is OpLifetimeStop; one overload per marked type.
//GLSL-DAG: spirv_instruction(id = 256)
//GLSL-DAG: void livenessStart_{{[0-9]+}}(spirv_by_reference Big_{{[0-9]+}} _0, spirv_literal int _1);
//GLSL-DAG: spirv_instruction(id = 257)
//GLSL-DAG: void livenessEnd_{{[0-9]+}}(spirv_by_reference Big_{{[0-9]+}} _0, spirv_literal int _1);
//GLSL-DAG: void livenessStart_{{[0-9]+}}(spirv_by_reference int _0, spirv_literal int _1);
//GLSL-DAG: void livenessStart_{{[0-9]+}}(spirv_by_reference float _0, spirv_literal int _1);
// The markers appear as ordinary calls around the local's range.
//GLSL-DAG: livenessStart_{{[0-9]+}}(v_{{[0-9]+}}, 0);
