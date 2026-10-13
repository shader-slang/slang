//DIAGNOSTIC_TEST:SIMPLE:

#lang 2026

struct ComparisonLeft
{}

struct ComparisonValue
{}

struct Holder
{
    ComparisonValue field;
}

Holder makeHolder()
{
    return {};
}

ComparisonLeft operator<(ComparisonValue left, int right)
{
    return {};
}

bool operator>(ComparisonLeft left, Tuple<> right)
{
    return true;
}

bool checkEmptyTupleComparison(int middle)
{
    // In Slang 2026, `()` is an empty-tuple expression. The spaces preserve this expression as
    // two comparisons rather than forcing `field<middle>()` to be a generic call.
    return makeHolder().field < middle > ();
}

struct GenericMethodFactory
{
    int constant<let N : int>() { return N; }
}

GenericMethodFactory makeGenericMethodFactory()
{
    return {};
}

int checkEmptyGenericCall()
{
    // The unspaced `<` remains unambiguously a generic application in Slang 2026.
    return makeGenericMethodFactory().constant<4>();
}
