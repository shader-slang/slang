//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK, non-exhaustive):

int4 apply(int4 value)
{
    return value.map((int element) => element + 1);
//CHECK:         ^^^ 'map' is not a member of 'vector<int,4>'.
}
