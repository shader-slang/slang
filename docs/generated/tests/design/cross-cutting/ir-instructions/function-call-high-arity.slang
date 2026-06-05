//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T20:30:00+00:00
//META: source_commit=273b0c1d7bd60576ed4d24081c0d1b9f61952281
//META: doc_ref=docs/generated/design/cross-cutting/ir-instructions.md#function-and-module-structure
//META: doc_section_digest=3cd430fe84a5b7025d059358578317eab126613a22446f8d9d7f776e96724317
//META: purpose=Stress probe of the documented `call` opcode (under the `func` family): a call to an 8-argument helper produces a single variadic `call` with all eight argument values.
//META: intent=stress
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc lists `func` as the function opcode and notes that call
// sites use a `call` opcode. The variadic-operand axis for `call` is
// the number of arguments — eight is comfortably above "trivial" and
// gives a wide-arity probe without exotic plumbing.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform RWStructuredBuffer<int> buf;

int variadic_op(int a, int b, int c, int d, int e, int f, int g, int h)
{
    return a + b + c + d + e + f + g + h;
}

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = variadic_op(1, 2, 3, 4, 5, 6, 7, 8);
}

// IR-LABEL: func %main
// IR: call %variadicx5Fop(1 : Int, 2 : Int, 3 : Int, 4 : Int, 5 : Int, 6 : Int, 7 : Int, 8 : Int)
