//DIAGNOSTIC_TEST:SIMPLE:

void foo()
{
    // Ok.
    uint64_t ok1 = 18446744073709551615U;
    uint64_t ok2 = 0xFFFFFFFFFFFFFFFFU;

    // Error, integer literal is too large any integer type.
    uint64_t tooLargeForAnyType1 = 18446744073709551616;
    uint64_t tooLargeForAnyType2 = 184467440737095516150;
    uint64_t tooLargeForAnyType3 = 0x10000000000000000;
    uint64_t tooLargeForAnyType4 = 0xFFFFFFFFFFFFFFFF0;
}
