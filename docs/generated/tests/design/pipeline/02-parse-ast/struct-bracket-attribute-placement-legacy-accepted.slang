//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#modifier-parsing
//META: doc_section_digest=c08befb0cda99dc013ab0ceb9ba3b786beb2359fadcd06a399a784137456d81d
//META: purpose=A bracketed attribute list written after the `struct` keyword is silently accepted at language versions before 2025, the earliest of the three placement outcomes the parser gates on.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the oldest arm of the struct-attribute placement gate. This is the
// one placement rule the parser decides itself rather than deferring to the
// checker, and at `-std 2018` it decides to say nothing: the same source that
// warns at 2025 and errors at 2026 compiles here, with the struct reaching
// codegen intact. The CHECK on the emitted struct is what shows the attribute
// list was consumed as a modifier rather than derailing the declaration.

//TEST:SIMPLE(filecheck=CHECK):-std 2018 -target hlsl -entry main -stage compute

struct [__NonCopyableType] Foo { int x; };

//CHECK: struct Foo_{{[0-9]+}}

// The struct is handed to a `[noinline]` callee and fed from the dispatch id so
// that neither inlining nor constant folding can dissolve it before emit.
[noinline]
int readIt(inout Foo f) { return f.x * 2; }

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    Foo f;
    f.x = int(tid.x);
    outputBuffer[0] = readIt(f);
}
