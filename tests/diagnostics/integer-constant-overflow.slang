//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void funcInt8(int8_t x) {}
void funcInt16(int16_t x) {}
void funcInt32(int x) {}
void funcInt64(int64_t x) {}
void funcUint8(uint8_t x) {}
void funcUint16(uint16_t x) {}
void funcUint32(uint x) {}
void funcUint64(uint64_t x) {}
void funcHalf(half x) {}

int8_t returnOverflow() { return 256; }
//CHECK:                         ^^^ integer constant overflow in conversion
//CHECK:                         ^^^ integer value '256' does not fit in type 'int8_t'

struct GenericInt8<let N : int8_t> { int8_t val() { return N; } };

[numthreads(1, 1, 1)]
void computeMain()
{
    // Generic value argument overflow
    GenericInt8<256> g;
//CHECK:        ^^^ integer constant overflow in conversion
//CHECK:        ^^^ integer value '256' does not fit in type 'int8_t'

    // Function arguments that overflow - should warn
    // (only the overflow warning fires; the general UnrecommendedImplicitConversion
    // is suppressed when overflow is detected)
    funcInt8(256);
//CHECK:     ^^^ integer constant overflow in conversion
//CHECK:     ^^^ integer value '256' does not fit in type 'int8_t'
    funcUint8(256);
//CHECK:      ^^^ integer constant overflow in conversion
//CHECK:      ^^^ integer value '256' does not fit in type 'uint8_t'

    // Variable initializer overflow
    int8_t x = 256;
//CHECK:       ^^^ integer constant overflow in conversion
//CHECK:       ^^^ integer value '256' does not fit in type 'int8_t'

    // Negative values that exceed signed range of the target bit-width
    funcInt8(-129);
//CHECK:      ^^^ integer constant overflow in conversion
//CHECK:      ^^^ integer value '-129' does not fit in type 'int8_t'
    funcInt8(-200);
//CHECK:      ^^^ integer constant overflow in conversion
//CHECK:      ^^^ integer value '-200' does not fit in type 'int8_t'
    funcInt8(-256);
//CHECK:      ^^^ integer constant overflow in conversion
//CHECK:      ^^^ integer value '-256' does not fit in type 'int8_t'

    // Negative overflow into unsigned type
    funcUint8(-129);
//CHECK:       ^^^ integer constant overflow in conversion
//CHECK:       ^^^ integer value '-129' does not fit in type 'uint8_t'

    // Constant expressions that overflow
    funcInt8(100 + 200);
//CHECK:         ^ integer constant overflow in conversion
//CHECK:         ^ integer value '300' does not fit in type 'int8_t'

    // 65536 needs 17 bits, exceeding 16-bit types
    funcInt16(65536);
//CHECK:      ^^^^^ integer constant overflow in conversion
//CHECK:      ^^^^^ integer value '65536' does not fit in type 'int16_t'
    funcUint16(65536);
//CHECK:       ^^^^^ integer constant overflow in conversion
//CHECK:       ^^^^^ integer value '65536' does not fit in type 'uint16_t'

    // 4294967296 (2^32) needs 33 bits, exceeding 32-bit types
    funcInt32(4294967296);
//CHECK:      ^^^^^^^^^^ integer constant overflow in conversion
//CHECK:      ^^^^^^^^^^ integer value '4294967296' does not fit in type 'int'
    funcUint32(4294967296);
//CHECK:       ^^^^^^^^^^ integer constant overflow in conversion
//CHECK:       ^^^^^^^^^^ integer value '4294967296' does not fit in type 'uint'

    // 64-bit overflow: 2^63 exceeds INT64_MAX, triggering the lexer-level
    // "too large for signed" warning (not our overflow check). The literal is
    // reinterpreted as unsigned, so no integer-constant-overflow fires.
    funcInt64(9223372036854775808);
//CHECK:      ^^^^^^^^^^^^^^^^^^^ integer literal is too large to be represented in a signed integer type, interpreting as unsigned
//CHECK:      ^^^^^^^^^^^^^^^^^^^ integer literal is too large to be represented in a signed integer type, interpreting as unsigned
    funcUint64(9223372036854775808);
//CHECK:       ^^^^^^^^^^^^^^^^^^^ integer literal is too large to be represented in a signed integer type, interpreting as unsigned
//CHECK:       ^^^^^^^^^^^^^^^^^^^ integer literal is too large to be represented in a signed integer type, interpreting as unsigned

    // Function arguments that fit - should NOT warn
    funcInt8(127);
    funcInt8(-128);
    funcUint8(255);
    funcInt16(32767);
    funcInt16(-32768);
    funcUint16(65535);
    funcInt32(42);
    funcUint32(42u);
    funcInt64(9223372036854775807);
    funcUint64(9223372036854775807);
    funcHalf(2048);

    // Half is excluded from the integer overflow check (getMaximumTypeBitSize
    // returns 0 for half). Values beyond 16-bit integers still don't trigger
    // the integer overflow warning, but do trigger the general implicit
    // conversion warning since they exceed half's exact integer range.
    funcHalf(131072);
//CHECK:     ^^^^^^ implicit conversion not recommended
//CHECK:     ^^^^^^ implicit conversion from 'int' to 'half' is not recommended

    // Bit-width boundary: max values that fit exactly in the type's bit width
    funcInt8(255);
    funcUint32(4294967295u);

    // Positive values outside the signed range but within the type's bit width
    // do NOT trigger the overflow warning. The check is conservative (bit-width
    // only, not range-aware) to avoid false positives on common shader patterns
    // where hex constants serve as bit patterns
    // (e.g. packed bytes: int x = 0xFF030206).
    funcInt32(0xFFFFFFFFu);
    funcInt8(128);
    funcInt16(40000);

    // -2 to unsigned: bit-width fits and getImplicitConversionCostWithKnownArg
    // reduces cost below kConversionCost_Default, so no warning from either check.
    funcUint32(-2);

    // -1 to unsigned is allowed (common "all ones" pattern)
    funcUint32(-1);
    funcUint8(-1);

    // Explicit casts should NOT warn (handled as ExplicitCoercion)
    int8_t y = int8_t(256);
}
