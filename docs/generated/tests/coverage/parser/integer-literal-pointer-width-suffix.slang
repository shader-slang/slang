//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Verifies the `z`/`uz` integer-literal width suffix selects intptr_t/uintptr_t for hexadecimal literals, and that `-9223372036854775808z` is demoted from uintptr_t back to intptr_t.
//META: intent=characterization
//META: covers=source/slang/slang-parser.cpp
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterization: `_determineIntegerLiteralType` has a separate
// non-decimal-base arm, and inside it the `Pointer` width suffix (spelled `z`
// or `Z`) is the only case that is never reached by a decimal literal. So
// `0xFFz` picks `intptr_t` and `0xFFuz` picks `uintptr_t` without consulting
// the magnitude at all.
//
// The last two arguments cover the signed-minimum exception: the decimal
// literal `9223372036854775808z` is one past INT64_MAX, so
// `_determineIntegerLiteralType` types it `uintptr_t` and sets
// `signedMinimumIntException`; when `parsePrefixExpr` then sees the unary `-`
// it rewrites the suffix type from `UIntPtr` back to `IntPtr`. Without the
// minus the literal stays unsigned.
//
// `which` is overloaded on every candidate integer type, so the printed
// selector number is a direct read-out of the type the parser assigned.

//TEST:INTERPRET(filecheck=CHECK):

int which(intptr_t x) { return 1; }
int which(uintptr_t x) { return 2; }
int which(int64_t x) { return 3; }
int which(uint64_t x) { return 4; }
int which(int x) { return 5; }
int which(uint x) { return 6; }

void main()
{
//CHECK: 1 2 1 2
    printf(
        "%d %d %d %d\n",
        which(0xFFz),
        which(0xFFuz),
        which(-9223372036854775808z),
        which(9223372036854775808z));
}
