//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -no-codegen

// A `ref` accessor returns the address of the storage named by its return expression. An r-value
// has no address for the accessor to preserve, so semantic checking must reject it before lowering.

struct Storage
{
    property int value
    {
        ref
        {
            return 1;
//CHECK:           ^ a `ref` accessor must return an l-value
//CHECK:           ^ this expression does not identify storage that the accessor can return by reference
        }
    }
}
