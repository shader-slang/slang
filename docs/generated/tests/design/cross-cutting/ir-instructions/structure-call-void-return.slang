//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T12:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/generated/design/cross-cutting/ir-instructions.md#function-and-module-structure
//META: doc_section_digest=3cd430fe84a5b7025d059358578317eab126613a22446f8d9d7f776e96724317
//META: purpose=A call to a `void`-returning helper with an `out` parameter lowers to a `call` opcode whose first non-callee operand is the address of the local backing the `out` argument, exercising the documented call-site form of `func`.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc lists `func` with "call sites use a `call` opcode". The
// `function-call-high-arity.slang` test exercises the variadic
// argument axis on an int-returning function; this test exercises
// the void-return path with an `out` parameter — the function's
// effect is its `out` write, so the call survives DCE without a
// return value being used by the caller.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;

void helper(out int r, int x) { r = x + 1; }

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int r;
    helper(r, a);
    buf[0] = r;
}

// IR-LABEL: func %main
// IR: call %helper(%{{.*}}, %a)
