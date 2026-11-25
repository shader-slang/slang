//TEST:SIMPLE(filecheck=CHECK):-dump-ir

// For-loop trip-count inference at the limit of `int`. The inferred count becomes the
// `loopMaxIters` decoration of the loop, whose value is an `int`, so a count up to INT32_MAX is
// inferred and a larger count is not. Without a target, the dump contains only the LOWER-TO-IR
// section, so the checks see the loops as lowering creates them.
//
// The diagnostic `[MaxIters]` comparison that `tests/diagnostics/for-loop-trip-count-types.slang`
// uses cannot show a count of INT32_MAX, because `[MaxIters]` itself is at most INT32_MAX.

// CHECK: ### LOWER-TO-IR:

// F1: INT32_MAX - 1 iterations.
// CHECK-LABEL: func %belowLimit
// CHECK: [loopMaxIters(2147483646 : Int)]
void belowLimit()
{
    for (uint i = 0; i < 0x7FFFFFFEu; i++) {}
}

// F3: INT32_MAX iterations, counting the bound with `<=`.
// CHECK-LABEL: func %atLimitInclusive
// CHECK: [loopMaxIters(2147483647 : Int)]
void atLimitInclusive()
{
    for (uint i = 0; i <= 0x7FFFFFFEu; i++) {}
}

// F4: INT32_MAX iterations, counting the bound with `<`.
// CHECK-LABEL: func %atLimitExclusive
// CHECK: [loopMaxIters(2147483647 : Int)]
void atLimitExclusive()
{
    for (uint i = 0; i < 0x7FFFFFFFu; i++) {}
}

// F2: 2^31 iterations do not fit in `int`, so nothing is inferred.
// CHECK-LABEL: func %overLimitExclusive
// CHECK-NOT: loopMaxIters
void overLimitExclusive()
{
    for (uint i = 0; i < 0x80000000u; i++) {}
}

// F5: 2^31 iterations, counting the bound with `<=`.
// CHECK-LABEL: func %overLimitInclusive
// CHECK-NOT: loopMaxIters
void overLimitInclusive()
{
    for (uint i = 0; i <= 0x7FFFFFFFu; i++) {}
}
