//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/misc.md#expand-and-each
//META: doc_section_digest=76e0405b8d1a78f45b2e312d649615a56ef8fe59e54b73ee93f9c862b0f85d26
//META: purpose=An `expand` statement over a variadic pack lowers to an `Expand` inst that owns its pattern body as child blocks rather than as an ordinary operand list.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `Expand` is a small nested region, not a flat value inst: the
// pattern the user wrote after `expand` must appear as a block owned by the
// inst. The CHECKs walk the IR dump in order — the `Expand` line, the opening
// brace of its body, a `block` with a slot-index parameter, and the pattern
// itself yielding a value — which is only the shape a region-shaped inst can
// print.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

RWStructuredBuffer<int> outputBuffer;

int sumAll<let each D : int>()
{
    int s = 0;
    expand s += each D;
    return s;
}

[numthreads(1, 1, 1)]
void main()
{
    outputBuffer[0] = sumAll<10, 20, 30>();
    outputBuffer[1] = sumAll<1, 2, 3, 4>();
}

// IR-LABEL: func %sumAll
// IR: Expand %{{[A-Za-z0-9_]+}}
// IR-NEXT: {
// IR-NEXT: block %{{[A-Za-z0-9_]+}}(
// IR: yield(%{{[A-Za-z0-9_]+}})
