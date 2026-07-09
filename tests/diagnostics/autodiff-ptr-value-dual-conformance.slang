//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

// Test that a type cannot conform to both IDifferentiable and IDifferentiablePtrType (error 30311).

struct BadType : IDifferentiable, IDifferentiablePtrType
/*CHECK:
       ^^^^^^^ type cannot conform to both value and pointer differentiation interfaces
       ^^^^^^^ type 'BadType' should conform only to one of IDifferentiable or IDifferentiablePtrType, not both.
*/
{
    typealias Differential = BadType;
    static Differential dzero() { return BadType(); }
    __init() {}
    static Differential dadd(Differential a, Differential b) { return a; }
}

[Differentiable]
float useBadType(BadType x)
{
    return 0.0;
}
