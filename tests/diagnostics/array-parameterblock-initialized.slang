//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -no-codegen

// Header checking infers array bounds and types from initializers. It must still validate
// array elements when `SemanticsDeclBodyVisitor` will not run for those declarations.
struct Item
{
    ParameterBlock<uint> block;
}

[numthreads(1, 1, 1)]
void main(uniform Item value)
{
    Item explicitValues[] = {value};
//CHECK: ^^^^^^^^^^^^^^ arrays of non-addressable type not allowed
//CHECK: ^^^^^^^^^^^^^^ Arrays of non-addressable type 'Item' are not allowed

    var inferredValues = explicitValues;
//CHECK:^^^^^^^^^^^^^^ arrays of non-addressable type not allowed
//CHECK:^^^^^^^^^^^^^^ Arrays of non-addressable type 'Item' are not allowed
}
