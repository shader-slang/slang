//DIAGNOSTIC_TEST:SIMPLE:

void foo()
{
    // Ok.
    uint64_t ok1 = 18446744073709551615U; // UINT64_MAX
    uint64_t ok2 = 0xffffffffffffffffU; // UINT64_MAX

    // Error, integer literal is too large to be represented in any integer type.
    uint64_t tooLargeForAnyType1 = 18446744073709551616; // UINT64_MAX + 1
    uint64_t tooLargeForAnyType2 = 184467440737095516150; // UINT64_MAX * 10
    uint64_t tooLargeForAnyType3 = 0x10000000000000000; // UINT64_MAX + 1
    uint64_t tooLargeForAnyType4 = 0xffffffffffffffff0; // UINT64_MAX * 16
}
