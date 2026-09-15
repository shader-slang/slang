//DIAGNOSTIC_TEST:SIMPLE(diag=DIAG):

// `[NoDiscard]` (the result must not be discarded) and `[DiscardableResult]` (the result may be
// discarded) are contradictory, so applying both to one function is rejected at the declaration.

[NoDiscard]
[DiscardableResult]
int both() { return 1; }
/*DIAG:
    ^^^^ '[NoDiscard]' and '[DiscardableResult]' cannot both be applied to a function
    ^^^^ a function cannot be both '[NoDiscard]' (its result must not be discarded) and '[DiscardableResult]' (its result may be discarded).
*/

[numthreads(1, 1, 1)]
void main()
{
    let x = both();
}
