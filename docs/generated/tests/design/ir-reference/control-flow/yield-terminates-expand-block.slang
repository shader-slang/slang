//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/control-flow.md#terminators-returns-and-yields
//META: doc_section_digest=010f71e10305e8452b6909c94e2da52806525700f6c78aa6daffe80e8b4ca211
//META: purpose=The single block inside an expand instruction is terminated by yield carrying that iteration's pattern value.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the `yield` terminator, which the catalog gives an ExpandExpr
// origin. A variadic-pack expansion inside a generic body lowers to an
// `Expand` instruction owning one block, and the CHECK asserts that block
// ends in `yield` with an operand -- the per-iteration pattern value, here
// the result of the expanded call. Anchoring at the uninstantiated
// `func %sum` inside the generic is what keeps the observation at the point
// where the pack is still symbolic; after specialization the expansion is
// unrolled and no `yield` remains.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform int n;
uniform RWStructuredBuffer<int> buf;

void acc(inout int total, int v)
{
    total = total + v;
}

int sum<each T : IInteger>(expand each T args)
{
    int total = 0;
    expand acc(total, (each args).toInt());
    return total;
}

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = sum(n, n + 1);
}

// CHECK-LABEL: func %sum
// CHECK: Expand %{{[0-9]+}}
// CHECK: yield(%{{[0-9]+}})
