//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T20:30:00+00:00
//META: source_commit=273b0c1d7bd60576ed4d24081c0d1b9f61952281
//META: doc_ref=docs/generated/design/cross-cutting/ir-instructions.md#memory-instructions
//META: doc_section_digest=784218f4c390fad67e31ccc3c7b8d48cc6fcafc60da93f9e66b0de850b7bb63b
//META: purpose=Boundary probe of the `load` memory opcode: reading a whole struct value out of an `RWStructuredBuffer` emits a `load(ptr)` whose pointer is the result of `rwstructuredBufferGetElementPtr`.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc lists `load` with operand `ptr`. The bootstrap test
// exercises `load` for a local-variable read; here we exercise the
// same opcode for a buffer-resident pointer where the pointer is the
// result of a resource-family opcode. Both call sites flow through
// the same `load` opcode.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

struct Foo { int x; int y; }
uniform RWStructuredBuffer<int> buf;
uniform RWStructuredBuffer<Foo> in_buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    Foo f = in_buf[0];
    buf[0] = f.x;
}

// IR-LABEL: func %main
// IR: rwstructuredBufferGetElementPtr(%inx5Fbuf, 0 : Int)
// IR: load(%{{.*}})
