//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -std legacy -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -std 2025 -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -std 2026 -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -std 202c -no-codegen

// Box formation is supported outside differentiation. Registration in a differentiable
// function must diagnose the unsupported pointer-box case without aborting compilation.
dyn IDifferentiablePtrType makePointerBox();
void ordinary(dyn IDifferentiablePtrType value) {}

[Differentiable]
void parameter(dyn IDifferentiablePtrType value)
//CHECK:       ^^^ differentiation of existential box type 'dyn IDifferentiablePtrType' through IDifferentiablePtrType is not supported
{}

// A return type must be checked even when no parameter has the unsupported type.
[Differentiable]
dyn IDifferentiablePtrType result()
/*CHECK:
^^^ differentiation of existential box type 'dyn IDifferentiablePtrType' through IDifferentiablePtrType is not supported
*/
{
    return makePointerBox();
}

// Neither the parameters nor the return type expose the box in this case.
[Differentiable]
void body()
{
    makePointerBox();
//CHECK:          ^ differentiation of existential box type 'dyn IDifferentiablePtrType' through IDifferentiablePtrType is not supported
}

interface IPtr : IDifferentiablePtrType {}
struct Holder
{
    dyn IPtr value;
}

// Recursive registration diagnoses the nested box at the enclosing parameter's type.
[Differentiable]
void nested(Holder value)
//CHECK:    ^^^^^^ differentiation of existential box type 'dyn IPtr' through IDifferentiablePtrType is not supported
{}
