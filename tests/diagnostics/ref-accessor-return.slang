//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature

struct InvalidReferences
{
    int integerValue;
    let readonlyValue = 1.0f;
    float mutableValue;

    property float literal
    {
        ref { return 1.0f; }
//CHECK:             ^^^^ ref accessor must return mutable storage
//CHECK:             ^^^^ a 'ref' accessor must return an l-value naming mutable storage.
    }
    property float converted
    {
        ref { return integerValue; }
//CHECK:             ^^^^^^^^^^^^ type mismatch in expression
//CHECK:             ^^^^^^^^^^^^ expected an expression of type 'float', got 'int'
    }
    property float readonlyField
    {
        ref { return readonlyValue; }
//CHECK:             ^^^^^^^^^^^^^ ref accessor must return mutable storage
//CHECK:             ^^^^^^^^^^^^^ a 'ref' accessor must return an l-value naming mutable storage.
    }
    property float expression
    {
        ref { return mutableValue + 1.0f; }
//CHECK:                          ^ ref accessor must return mutable storage
//CHECK:                          ^ a 'ref' accessor must return an l-value naming mutable storage.
    }
}
