//DIAGNOSTIC_TEST:SIMPLE(diag=diag):

// Test that a generic type parameter shadowing an outer generic parameter
// produces a warning diagnostic.

struct Optional<T>
{
    T value;
    bool hasValue;
}

public extension<T> Optional<T>
//diag:          ^ outer generic parameter 'T' declared here
{
    public T or<T>(Optional<T> other)
//diag:         ^ generic parameter 'T' shadows a generic parameter from an enclosing scope
//diag:         ^ generic parameter 'T' shadows an outer generic parameter with the same name
//diag:      ^^ references less visible type
//diag:      ^^ 'or' references less visible type 'Optional<T>'.
    {
        if (hasValue)
            return value;
//diag:            ^^^^^ type mismatch in expression
//diag:            ^^^^^ expected an expression of type 'T', got 'T'
        return other.value;
    }
}
