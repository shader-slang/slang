//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

// Test that [DerivativeMember] can only be used on members of a type (error 31132).

struct DiffTarget : IDifferentiable
{
    typealias Differential = DiffTarget;
    float y;
}

[DerivativeMember(DiffTarget.y)]
/*CHECK:
 ^^^^^^^^^^^^^^^^ [DerivativeMember] is allowed on members only.
*/
float globalVar;
