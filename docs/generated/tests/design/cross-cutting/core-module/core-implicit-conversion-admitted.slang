//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/core-module.md#what-the-core-module-provides
//META: doc_section_digest=9e789f76018ea954bfb264040a694991a83baab0f112111a9b519188a6174ec6
//META: purpose=The core module's __implicit_conversion declarations admit int-to-float argument conversion while an exact-match overload still wins over a converting one.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the implicit-conversion rules the core module declares are
// both present and ranked. `widen` accepts only float, so calling it
// with an int proves a conversion is admitted at all; `pick` offers
// int and float overloads, so calling it with an int proves the
// zero-cost exact match outranks the conversion. The CHECK pins
// pick=1 (the int overload) together with widen's converted result.

//TEST:INTERPRET(filecheck=CHECK):

float widen(float x) { return x * 2.0; }
int pick(int) { return 1; }
int pick(float) { return 2; }

void main()
{
    int i = 3;
    // CHECK: widened=6.0 pick=1
    printf("widened=%.1f pick=%d\n", widen(i), pick(i));
}
