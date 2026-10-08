//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

void load<let N : int, each TIndex>(TIndex indices) where countof(TIndex) == N
{
}

void mismatch()
{
    load<2>(1, 2, 3);
//CHECK:   ^ pack count does not satisfy `countof(...)` constraint
//CHECK:   ^ expected 2 elements, but pack argument has 3
}

void noCountInference()
{
    load(1, 2, 3);
//CHECK:^ could not specialize generic for arguments of type (int, int, int)
}

void bar<let N : int, each I>() where countof(I) == N
{
}

void badFoo<let N : int, let M : int, each I>() where countof(I) == N
{
    bar<M, I>();
/*CHECK:
    ^^^ `countof(...)` pack-count constraint is not satisfied
    ^^^ cannot prove the required `countof(...)` pack-count constraint for this specialization
*/
}

struct NoSymmetricCountWitness<each U>
{
    void badInner<each T>(T args) where countof(T) == countof(U)
    {
        bar<countof(T), U>();
/*CHECK:
        ^^^ `countof(...)` pack-count constraint is not satisfied
        ^^^ cannot prove the required `countof(...)` pack-count constraint for this specialization
*/
    }
}

void badTarget<let N : int>() where countof(int) == N
//CHECK:                                    ^^^ `countof(...)` pack-count constraint requires a generic type pack or value pack parameter
//CHECK:                                    ^^^ expected a direct reference to a generic pack parameter here
{
}

int runtimeCount()
{
    return 1;
}

void badCount<each T>() where countof(T) == runtimeCount()
//CHECK:                                                ^ `countof(...)` pack-count constraint requires a compile-time integer count
//CHECK:                                                ^ expected a compile-time integer expression here
{
}

void optionalBad<each T>() where optional countof(T) == 1
//CHECK:                                          ^ `optional countof(...)` pack-count constraint is not meaningful
//CHECK:                                          ^ remove `optional` from this `countof(...)` constraint
{
}

struct Outer<each T>
{
    void badInner<each U>() where countof(T) == 1
//CHECK:                                  ^ `countof(...)` pack-count constraint target must be declared in the current generic
//CHECK:                                  ^ this pack parameter is declared outside the current generic
    {
    }
};

struct OuterCountOfBothSides<let each N : int>
{
    void badInner<each TIndex>(TIndex indices)
        where TIndex == int
        where countof(N) == countof(TIndex)
//CHECK:              ^ `countof(...)` pack-count constraint target must be declared in the current generic
//CHECK:              ^ this pack parameter is declared outside the current generic
    {
    }
};
