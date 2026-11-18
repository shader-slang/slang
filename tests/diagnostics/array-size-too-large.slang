//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// An array type stores its size as an `int`, so a constant array size must fit in `int`. A size
// written with an unsigned or 64-bit type can exceed `INT32_MAX`, and the checker reports it
// instead of converting it to a negative `int`. An unsigned size with bit 63 set, including an
// enum case whose tag type is unsigned, is too large rather than negative.

static const uint kLargeSize = 2147483648u;

enum LargeEnum : uint64_t
{
    TopBit = 0x8000000000000000ull,
}

void f()
{
    float uintSize[2147483648u];
//CHECK:           ^^^^^^^^^^^ error E30034
//CHECK:           ^^^^^^^^^^^ array size must not exceed 2147483647.
    float uint64Size[0x100000001ull];
//CHECK:             ^^^^^^^^^^^^^^ error E30034
//CHECK:             ^^^^^^^^^^^^^^ array size must not exceed 2147483647.
    float uintMax[0xFFFFFFFFu];
//CHECK:          ^^^^^^^^^^^ error E30034
//CHECK:          ^^^^^^^^^^^ array size must not exceed 2147483647.
    float topBitSize[0x8000000000000000ull];
//CHECK:             ^^^^^^^^^^^^^^^^^^^^^ error E30034
//CHECK:             ^^^^^^^^^^^^^^^^^^^^^ array size must not exceed 2147483647.
    float enumSize[LargeEnum.TopBit];
//CHECK:                     ^^^^^^ error E30034
//CHECK:                     ^^^^^^ array size must not exceed 2147483647.
    float constSize[kLargeSize];
//CHECK:            ^^^^^^^^^^ error E30034
//CHECK:            ^^^^^^^^^^ array size must not exceed 2147483647.
    float negativeSize[-1];
//CHECK:                ^ error E30025
//CHECK:                ^ array size must be non-negative.
}
