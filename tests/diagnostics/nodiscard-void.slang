//DIAGNOSTIC_TEST:SIMPLE(diag=DIAG):

// `[NoDiscard]` on a function that returns `void` has no result to discard,
// so it is rejected at the declaration with an error.

struct MyType
{
    int val;

    [NoDiscard]
    void touch() {}
/*DIAG:
         ^^^^^ '[NoDiscard]' applied to a function returning 'void'
         ^^^^^ '[NoDiscard]' is not allowed on a function that returns 'void'; there is no result to discard.
*/
}

[NoDiscard]
void freeVoid() {}
/*DIAG:
     ^^^^^^^^ '[NoDiscard]' applied to a function returning 'void'
     ^^^^^^^^ '[NoDiscard]' is not allowed on a function that returns 'void'; there is no result to discard.
*/

[numthreads(1, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    MyType t;
    t.val = 0;

    // Calling a `[NoDiscard]` void function adds no discard error at the call site: only the
    // declaration error above fires. The call-site guard in `maybeDiagnoseDiscardedNoDiscardResult`
    // returns early for a `void` result, so these lines are intentionally un-annotated.
    t.touch();
    freeVoid();
}
