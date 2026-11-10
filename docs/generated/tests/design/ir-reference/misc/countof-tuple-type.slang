//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:59:05+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/ir-reference/misc.md#size-alignment-count
//META: doc_section_digest=2dd2bfcaa2aa059f39f24fda445895796cfd89712938d9d37e820a7ca67b9e63
//META: purpose=`countof` accepts a tuple and yields its element count, folded by `maybeSpecializeCountOf` once the operand is a tuple type.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the tuple half of the count query's domain. Two tuples of different
// arity are counted, so the asserted 3 / 2 sequence shows a real per-tuple
// element count.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    Tuple<int, float, uint> three = makeTuple(1, 2.0f, 3u);
    Tuple<int, float> two = makeTuple(1, 2.0f);
    outputBuffer[0] = countof(three);
    outputBuffer[1] = countof(two);
}

// CHECK: {{^3$}}
// CHECK-NEXT: 2
