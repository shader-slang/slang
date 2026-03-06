//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void funcInt8(int8_t x) {}
void funcInt16(int16_t x) {}
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
    funcInt8(-200);
//CHECK:      ^^^ integer constant overflow in conversion
//CHECK:      ^^^ integer value '-200' does not fit in type 'int8_t'
    funcUint8(256);
//CHECK:      ^^^ integer constant overflow in conversion
//CHECK:      ^^^ integer value '256' does not fit in type 'uint8_t'
    funcInt16(40000);
//CHECK:      ^^^^^ integer constant overflow in conversion
//CHECK:      ^^^^^ integer value '40000' does not fit in type 'int16_t'

    // Unsigned-to-signed overflow
    funcInt(0xFFFFFFFFu);
//CHECK:    ^^^^^^^^^^^ integer constant overflow in conversion
//CHECK:    ^^^^^^^^^^^ integer value '4294967295' does not fit in type 'int'

    // Signed-to-unsigned overflow
    funcUint(-2);
//CHECK:      ^ integer constant overflow in conversion
//CHECK:      ^ integer value '-2' does not fit in type 'uint'

    // Half overflow
    funcHalf(3000);
//CHECK:     ^^^^ integer constant overflow in conversion
//CHECK:     ^^^^ integer value '3000' does not fit in type 'half'

    // Variable initializer overflow
    int8_t x = 256;
//CHECK:       ^^^ integer constant overflow in conversion
//CHECK:       ^^^ integer value '256' does not fit in type 'int8_t'

    // Function arguments that fit - should NOT warn
    funcInt8(127);
    funcInt8(-128);
    funcUint8(255);
    funcInt16(32767);
    funcInt(42);
    funcUint(42u);
    funcHalf(2048);

    // -1 to unsigned is allowed (common "all ones" pattern)
    funcUint(-1);
    funcUint8(-1);

    // Explicit casts should NOT warn (handled as ExplicitCoercion)
    int8_t y = int8_t(256);

    // Constant expressions that overflow
    funcInt8(100 + 200);
//CHECK:         ^ integer constant overflow in conversion
//CHECK:         ^ integer value '300' does not fit in type 'int8_t'
}
