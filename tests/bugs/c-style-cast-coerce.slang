//DIAGNOSTIC_TEST:SIMPLE(diag=CHK2026): -std 2026 -DSLANG2026
//DIAGNOSTIC_TEST:SIMPLE(diag=CHK202C): -std 202c -DSLANG202C

// Slang <=2026:
//
// It used to be the case that coercions of already-coerced initializer lists
// didn't take into account the more specific type. One way of triggering this
// was to create an already-coerced initializer list via the HLSL `(S)0`
// syntax which creates the equivalent of `{} : S`. In the example below this
// then proceeds to be coerced to type T.

// Slang >=202c:
//
// The cast from 0 special case has been removed from the language, so
// this is now a regular conversion from value 0. Since `S` does not
// have an initializer from an integer value, an additional error
// about a missing initializer is triggered.

struct S {};
struct T {};

void main()
{
#ifdef SLANG2026
    // Should generate a type error.
    T t = (S)0;
//CHK2026:^ casting literal 0 to a struct type changes semantics in Slang 202c
//CHK2026:^ casting literal 0 to a struct type becomes a conversion in Slang 202c. To keep the current semantics, switch to a constructor with no parameters.
//CHK2026:^ type mismatch in expression
//CHK2026:^ expected an expression of type 'T', got 'S'
#endif

#ifdef SLANG202C
    // In Slang 202c, cast from 0 no longer maps to default
    // initializer, so we'll get an additional error
    T t = (S)0;
//CHK202C:^ too many initializers in initializer list
//CHK202C:^ too many initializers (expected 0, got 1)
//CHK202C:^ type mismatch in expression
//CHK202C:^ expected an expression of type 'T', got 'S'

#endif
}
