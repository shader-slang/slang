//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-02T14:19:54+00:00
//META: source_commit=6815ed82d5e8c95b0e1258ca2eba0ca9c7825d86
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#specialization-and-generics
//META: doc_section_digest=ad41019b62f65b8926fb421c96faf20ade0eadeccc6562a5b0ee3bfcf86755bd
//META: purpose=The type-flow specialization pass emits no code of its own; its one direct user surface is the dynamic-dispatch-site report, which names each call site it could not resolve statically.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the only user-facing surface the type-flow specialization row
// claims. The interface call below cannot be resolved statically because
// which implementer reaches it depends on a uniform, so the report must
// name that call site. The CHECK pins note E50102, the caret to the
// method call, and the number of types that can reach it — a call the
// pass *had* resolved would produce no report line at all. The order of
// the listed type names follows the type-flow merge order, so it is not
// pinned.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -stage compute -entry main -report-dynamic-dispatch-sites

uniform int sel;
RWStructuredBuffer<int> outBuf;

interface IShape
{
    int area();
}

struct Square : IShape
{
    int s;
    int area() { return s * s; }
}

struct Rect : IShape
{
    int w;
    int h;
    int area() { return w * h; }
}

[numthreads(1, 1, 1)]
void main()
{
    IShape shape;
    if (sel > 0)
    {
        Square a;
        a.s = 3;
        shape = a;
    }
    else
    {
        Rect b;
        b.w = 2;
        b.h = 5;
        shape = b;
    }
    outBuf[0] = shape.area();
//CHECK:                  ^ generated dynamic dispatch code for this site. 2 possible types:
}
