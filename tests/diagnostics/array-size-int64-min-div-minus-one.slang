//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,exit-code=0):-target spirv -entry computeMain -stage compute
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-cpu -output-using-type -Xslang -warnings-disable -Xslang 30005

// Signed 64-bit `INT64_MIN / -1` and `INT64_MIN % -1` overflow, and a host division instruction
// such as x86-64 `idiv` traps on them. The compiler must not perform that division while
// folding constants.
//
// The front end reports the result as target-defined and does not fold it. In an array size
// written with the infix operator, the size then stays a symbolic value that is lowered to IR,
// where SCCP folds it. SCCP folds the quotient to the wrapped value `INT64_MIN` and the
// remainder to 0, the same values it produces for narrower types. The other cases also reach
// SCCP, from different scopes: the `static const` is lowered to a module-scope global constant
// whose initializer SCCP folds, while the operations on `dividend` and `divisor` are lowered
// inside `computeMain` and reach SCCP after SSA construction.

//TEST_INPUT:ubuffer(stride=8, count=5):out,name=outBuf
RWStructuredBuffer<int64_t> outBuf;

[numthreads(1, 1, 1)]
void computeMain()
{
    int quotientArray[(int64_t(0x8000000000000000ull) / int64_t(-1) == int64_t(0)) ? 1 : 2];
//CHECK:                                              ^ target-defined value
//CHECK:                                              ^ the value of expression -9223372036854775808LL / -1LL is target-defined.
    int remainderArray[(int64_t(0x8000000000000000ull) % int64_t(-1) == int64_t(0)) ? 1 : 2];
//CHECK:                                               ^ target-defined value
//CHECK:                                               ^ the value of expression -9223372036854775808LL % -1LL is target-defined.

    static const int64_t quotientConst = int64_t(0x8000000000000000ull) / int64_t(-1);
//CHECK:                                                                ^ target-defined value
//CHECK:                                                                ^ the value of expression -9223372036854775808LL / -1LL is target-defined.

    int64_t dividend = int64_t(0x8000000000000000ull);
    int64_t divisor = int64_t(-1);

    outBuf[0] = quotientArray.getCount();
    outBuf[1] = remainderArray.getCount();
    outBuf[2] = quotientConst;
    outBuf[3] = dividend / divisor;
    outBuf[4] = dividend % divisor;
}

// The CPU test checks the values that SCCP produces. The quotient is not 0, so the quotient
// array has 2 elements, and the remainder is 0, so the remainder array has 1 element.
//BUF: type: int64_t
//BUF-NEXT: 2
//BUF-NEXT: 1
//BUF-NEXT: -9223372036854775808
//BUF-NEXT: -9223372036854775808
//BUF-NEXT: 0
