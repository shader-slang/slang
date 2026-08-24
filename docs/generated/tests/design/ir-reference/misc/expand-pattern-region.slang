//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/misc.md#expand-and-each
//META: doc_section_digest=c56fdab34ebb59c29d5ab494f1d9c8bdd444aa7c43b0c37779d21f2e324dfa2b
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
