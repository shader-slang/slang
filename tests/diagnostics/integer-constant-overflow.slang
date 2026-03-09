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

    // 4294967296 (2^32) needs 33 bits, exceeding int's 32
    funcInt(4294967296);
//CHECK:    ^^^^^^^^^^ integer constant overflow in conversion
//CHECK:    ^^^^^^^^^^ integer value '4294967296' does not fit in type 'int'

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

    // Same-width sign reinterpretation is NOT overflow. These values are
    // outside the signed range but fit within the type's bit width, so
    // no warning is emitted from either the bit-width overflow check or
    // UnrecommendedImplicitConversion (whose cost is reduced by
    // getImplicitConversionCostWithKnownArg). This is a deliberate choice
    // for shader code where hex constants are commonly used as bit patterns
    // (e.g. packed bytes: int x = 0xFF030206).
    funcInt(0xFFFFFFFFu);
    funcInt8(128);
    funcInt8(-129);
    funcInt8(-200);
    funcInt16(40000);
    funcUint(-2);

    // -1 to unsigned is allowed (common "all ones" pattern)
    funcUint(-1);
    funcUint8(-1);

    // Explicit casts should NOT warn (handled as ExplicitCoercion)
    int8_t y = int8_t(256);
}
