//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Verifies `Optional<Ptr<T>>` is laid out as the bare pointer with no separate flag, because a pointer has a spare bit pattern for the absent case.
//META: intent=characterization
//META: covers=source/slang/slang-type-layout.cpp
//META: pipeline_stage=layout
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// This is the other arm of the `OptionalType` case that
// `optional-scalar-tuple-layout.slang` covers: when
// `doesTypeHaveAnUnusedBitPatternThatCanBeUsedForOptionalRepresentation` says
// the value type is nullable, the layout is the value type's own layout and no
// discriminant is added. A pointer qualifies, so the emitted CPU struct should
// contain exactly one `float *` member for `maybe` and then `tail`; if the
// tuple arm had been taken instead there would be an extra bool member between
// them and `tail` would move.
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry main -stage compute

struct S { Optional<Ptr<float> > maybe; float tail; }
uniform ConstantBuffer<S> cb;
RWStructuredBuffer<float> outp;
[numthreads(1, 1, 1)]
void main()
{
    outp[0] = cb.tail;
}

//CPP: struct S{{[A-Za-z0-9_]*$}}
//CPP: float * maybe{{[A-Za-z0-9_]*}};
//CPP-NEXT: float tail{{[A-Za-z0-9_]*}};
