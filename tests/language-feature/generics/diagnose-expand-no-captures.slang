//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// `expand` without any `each` referencing a type pack should produce
// error 30105 because expand needs at least one captured pack.

int noCaptures()
{
    int x = 0;
    expand (x + 1);
/*CHECK:
    ^^^^^^ 'expand' captures no type packs
    ^^^^^^ 'expand' term captures no type packs. At least one type pack must be referenced via an 'each' term inside an 'expand' term.
*/
    return x;
}
