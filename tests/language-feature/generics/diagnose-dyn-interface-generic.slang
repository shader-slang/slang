// Test that the compiler rejects dyn interfaces that are generic,
// dyn interfaces with generic methods, and generic types that
// try to conform to dyn interfaces.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -lang slang -std 2026

// dyn interfaces cannot be generic.
dyn interface IGenericDyn<T>
//CHECK:      ^^^^^^^^^^^ dyn interfaces cannot be generic
{
    int getValue();
}

// dyn interfaces cannot have generic methods.
dyn interface IMethodDyn
{
    T transform<T>(T val);
/*CHECK:
      ^^^^^^^^^ dyn interfaces cannot have generic methods
*/
}

// generic type cannot conform to dyn interface.
dyn interface ISimpleDyn
{
    int getValue();
}

struct GenericImpl<T> : ISimpleDyn
//CHECK:                ^^^^^^^^^^ generic type cannot conform to dyn interface
{
    int getValue() { return 0; }
}
