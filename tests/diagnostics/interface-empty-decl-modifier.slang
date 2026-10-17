//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Empty declarations still undergo attribute validation.
interface IInvalid
{
    [numthreads(1, 1, 1)] ;
/*CHECK:
     ^^^^^^^^^^ E31002
     ^^^^^^^^^^ attribute 'numthreads' is not valid here
*/
}
struct Invalid : IInvalid {}
