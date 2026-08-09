//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/core-module.md#core-module
//META: doc_section_digest=8489fad23ce3c21f6f3d24794bba24b5d2a69ab1e82e6e16bfa45871b21462f3
//META: purpose=float32_t distinguishes -0.0 from +0.0 in its bit pattern while still comparing them equal.
//META: intent=boundary
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the core module's 32-bit float base type keeps the IEEE
// signed-zero distinction. Equality is checked first (it must hold),
// then the sign is recovered by dividing 1.0 by each zero: +0.0 yields
// +inf and -0.0 yields -inf. The CHECK pins `eq=1 posRecip=inf
// negRecip=-inf`, which separates the two zeros without needing a
// bit-cast intrinsic.

//TEST:INTERPRET(filecheck=CHECK):

float32_t recip(float32_t x) { return float32_t(1.0) / x; }

void main()
{
    float32_t pz = float32_t(0.0);
    float32_t nz = -pz;
    // CHECK: eq=1 posRecip=inf negRecip=-inf
    printf("eq=%d posRecip=%f negRecip=%f\n", int(pz == nz), recip(pz), recip(nz));
}
