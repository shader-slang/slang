//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv

// Mutually recursive function-like macros whose bodies end in the other macro's
// complete invocation used to loop forever in the preprocessor, because the
// exhausted expansion stream of `A` was not counted as busy while expanding `B`.
// The inner `A` must be left unexpanded, as with the object-like case, and then
// be diagnosed as an undefined identifier.

#define A() B()
#define B() A()
//CHECK:    ^ undefined identifier
//CHECK:    ^ undefined identifier 'A'.

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    A();
}
