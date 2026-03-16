//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Test that using a generic type without providing type arguments
// produces error 30400.
// Duplicate CHECK lines: the compiler emits each diagnostic twice
// (once during initial checking and once during re-checking).

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
