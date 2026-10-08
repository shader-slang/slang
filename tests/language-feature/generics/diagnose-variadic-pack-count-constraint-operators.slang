//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void badNotEqual<let N : int, each T>() where countof(T) != N
//CHECK:                                                 ^^ `countof(...)` pack-count constraint requires '=='
//CHECK:                                                 ^^ use '==' for a `countof(...)` pack-count constraint
{
}

void badGreaterEqualLeft<let N : int, each T>() where countof(T) >= N
//CHECK:                                                         ^^ `countof(...)` pack-count constraint requires '=='
//CHECK:                                                         ^^ use '==' for a `countof(...)` pack-count constraint
{
}

void badRightHandCountOf<let N : int, each T>() where N == countof(T)
//CHECK:                                                   ^^^^^^^ `countof(...)` pack-count constraint must be written with `countof(...)` on the left side
//CHECK:                                                   ^^^^^^^ write this constraint as `countof(Pack) == IntExpr`
{
}

void badGreaterEqualRight<let N : int, each T>() where N >= countof(T)
//CHECK:                                                    ^^^^^^^ `countof(...)` pack-count constraint must be written with `countof(...)` on the left side
//CHECK:                                                    ^^^^^^^ write this constraint as `countof(Pack) == IntExpr`
{
}

void badRightHandCountOfWithTrailingExpr<let N : int, each T>()
    where N == countof(T) + 1
//CHECK:       ^^^^^^^ `countof(...)` pack-count constraint must be written with `countof(...)` on the left side
//CHECK:       ^^^^^^^ write this constraint as `countof(Pack) == IntExpr`
    where countof(T) == N
{
}
