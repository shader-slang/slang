//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void funcInt8(int8_t x) {}
void funcInt16(int16_t x) {}
void funcUint16(uint16_t x) {}
void funcInt(int x) {}
void funcUint(uint x) {}
void funcUint8(uint8_t x) {}
void funcHalf(half x) {}

struct GenericInt8<let N : int8_t> { int8_t val() { return N; } };

[numthreads(1, 1, 1)]
void computeMain()
{
    // Generic value argument overflow
    GenericInt8<256> g;
//CHECK:        ^^^ integer constant overflow in conversion
//CHECK:        ^^^ integer value '256' does not fit in type 'int8_t'

    // Function arguments that overflow - should warn
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
    funcInt(4294967296);
//CHECK:    ^^^^^^^^^^ integer constant overflow in conversion
//CHECK:    ^^^^^^^^^^ integer value '4294967296' does not fit in type 'int'
    funcUint(4294967296);
//CHECK:     ^^^^^^^^^^ integer constant overflow in conversion
//CHECK:     ^^^^^^^^^^ integer value '4294967296' does not fit in type 'uint'

    // Function arguments that fit - should NOT warn
    funcInt8(127);
    funcInt8(-128);
    funcUint8(255);
    funcInt16(32767);
    funcInt16(-32768);
    funcUint16(65535);
    funcInt(42);
    funcUint(42u);
    funcHalf(2048);

    // Bit-width boundary: max values that fit exactly in the type's bit width
    funcInt8(255);
    funcUint(4294967295u);

    // Values outside the signed range but within the type's bit width do
    // NOT trigger the overflow warning. The check is conservative (bit-width
    // only, not range-aware) to avoid false positives on common shader
    // patterns where hex constants serve as bit patterns
    // (e.g. packed bytes: int x = 0xFF030206).
    funcInt(0xFFFFFFFFu);
    funcInt8(128);
    funcInt8(-129);
    funcInt8(-200);
    funcInt16(40000);

    // -2 to unsigned: bit-width fits and getImplicitConversionCostWithKnownArg
    // reduces cost below kConversionCost_Default, so no warning from either check.
    funcUint(-2);

    // -1 to unsigned is allowed (common "all ones" pattern)
    funcUint(-1);
    funcUint8(-1);

    // Explicit casts should NOT warn (handled as ExplicitCoercion)
    int8_t y = int8_t(256);
}
