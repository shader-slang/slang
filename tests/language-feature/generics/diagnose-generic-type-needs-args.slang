//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Test that using a generic type without providing type arguments
// produces error 30400.

struct MyGeneric<T>
{
    T value;
}

// Using the generic type without arguments should error.
MyGeneric x;
/*CHECK:
^^^^^^^^^ generic type 'MyGeneric' used without argument
^^^^^^^^^ generic type 'MyGeneric' used without argument
*/
