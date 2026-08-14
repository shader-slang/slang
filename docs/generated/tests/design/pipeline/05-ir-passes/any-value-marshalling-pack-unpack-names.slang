//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#specialization-and-generics
//META: doc_section_digest=2f10610b6a8497f9b2cec62d411162a4585b32e9698a95e5288845967ffba16d
//META: purpose=AnyValue marshalling synthesizes one packAnyValue<N> / unpackAnyValue<N> function per inferred size and replaces every pack / unpack inst with a call to it, so those two names appear in a dynamic-dispatch shader's emit.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the one name a reader can look for from the AnyValue pair of
// passes. Two different implementers of `IShape` are stored into the same
// interface-typed local behind a runtime condition, so the value has to be
// boxed rather than statically resolved. The CHECKs assert both synthesized
// marshalling functions appear by name, and that the `N` in them is the
// concrete size AnyValue inference settled on rather than a generic
// placeholder.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage compute -entry main

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
}

// The size suffix is the inferred AnyValue size, not a generic parameter.
// HLSL-DAG: packAnyValue{{[0-9]+}}_{{[0-9]+}}(
// HLSL-DAG: unpackAnyValue{{[0-9]+}}_{{[0-9]+}}(
