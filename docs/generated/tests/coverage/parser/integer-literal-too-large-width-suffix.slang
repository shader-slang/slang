//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Verifies a decimal literal at least two past INT64_MAX with an explicit `ll` or `z` width suffix is reported as too large for a signed type and reinterpreted as unsigned.
//META: intent=characterization
//META: covers=source/slang/slang-parser.cpp
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterization: for a decimal literal with an explicit width suffix and no
// `u`, `_determineIntegerLiteralType` splits three ways. Values up to INT64_MAX
// get the signed type; exactly INT64_MAX+1 sets `signedMinimumIntException` and
// defers the decision until `parsePrefixExpr` has seen whether a unary `-`
// follows; anything at INT64_MAX+2 or above can never be a signed value even
// after negation, so it is diagnosed right there in the parser and the literal
// falls back to the unsigned type.
//
// 9223372036854775809 is INT64_MAX+2, so both the `ll` (LongLong) and the `z`
// (Pointer) arms of that three-way split fire. This is a warning, so the
// compile still succeeds.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry main -stage compute

static const uint64_t a = 9223372036854775809ll;
//CHECK:                  ^^^^^^^^^^^^^^^^^^^^^ integer literal is too large to be represented in a signed integer type, interpreting as unsigned

static const uintptr_t b = 9223372036854775809z;
//CHECK:                   ^^^^^^^^^^^^^^^^^^^^ integer literal is too large to be represented in a signed integer type, interpreting as unsigned

[numthreads(1, 1, 1)]
void main() {}
