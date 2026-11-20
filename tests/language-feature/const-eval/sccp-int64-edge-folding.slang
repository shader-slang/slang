//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

// IR SCCP folds a cast of a floating-point constant to an integer type and the negation of an
// integer constant, both in `static const` initializers, which are lowered to global constants,
// and in function bodies. Two of these folds used to be undefined behavior in the compiler:
//
// - converting 2^63 with `(IRIntegerValue)` overflows `int64_t`, although the value fits in
//   `uint64_t`;
// - negating `INT64_MIN` overflows `int64_t`.
//
// SCCP now converts a float with a range check against the destination type, and negates in
// `uint64_t`. The sanitizer CI job reports any remaining undefined behavior on these paths.

//TEST_INPUT:ubuffer(stride=8, count=4):out,name=outBuf
RWStructuredBuffer<uint64_t> outBuf;

static const uint64_t kTopBitFromFloat = (uint64_t)9223372036854775808.0;
static const int64_t kNegatedMin = -int64_t(0x8000000000000000ull);

[numthreads(1, 1, 1)]
void computeMain()
{
    outBuf[0] = kTopBitFromFloat;
    outBuf[1] = uint64_t(kNegatedMin);

    uint64_t topBit = (uint64_t)9223372036854775808.0;
    int64_t minValue = int64_t(0x8000000000000000ull);
    outBuf[2] = topBit;
    outBuf[3] = uint64_t(-minValue);
}

// CHECK: type: uint64_t
// CHECK-NEXT: {{^}}9223372036854775808{{$}}
// CHECK-NEXT: {{^}}9223372036854775808{{$}}
// CHECK-NEXT: {{^}}9223372036854775808{{$}}
// CHECK-NEXT: {{^}}9223372036854775808{{$}}
