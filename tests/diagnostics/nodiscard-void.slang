//DIAGNOSTIC_TEST:SIMPLE(diag=diag):

// `[nodiscard]` on a function that returns `void` has no result to discard,
// so it is rejected at the declaration with an error.

struct MyType
{
    int val;

    [nodiscard]
    void touch() {}
/*diag:
         ^^^^^ '[nodiscard]' applied to a function returning 'void'
         ^^^^^ '[nodiscard]' has no effect on a function that returns 'void'.
*/
}

[nodiscard]
void freeVoid() {}
/*diag:
     ^^^^^^^^ '[nodiscard]' applied to a function returning 'void'
     ^^^^^^^^ '[nodiscard]' has no effect on a function that returns 'void'.
*/

[numthreads(1, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    MyType t;
    t.val = 0;
}
