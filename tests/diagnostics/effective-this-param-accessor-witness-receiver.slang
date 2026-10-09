//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -no-codegen

interface INonmutatingProperty
{
    property int value
//CHECK:         ^^^^^ see interface requirement declaration of 'value'
    {
        [nonmutating] get;
    }
}

struct MutatingPropertyImpl : INonmutatingProperty
{
    int storedValue;

    property int value
//CHECK:         ^^^^^ interface requirement mismatch
//CHECK:         ^^^^^ member 'value' does not match interface requirement.
    {
        [mutating] get
        {
            storedValue++;
            return storedValue;
        }
    }
}

interface INonmutatingSubscript
{
    __subscript(int index) -> int
//CHECK:       ^ see interface requirement declaration of 'operator[]'
    {
        [nonmutating] get;
    }
}

struct MutatingSubscriptImpl : INonmutatingSubscript
{
    int storedValue;

    __subscript(int index) -> int
//CHECK:       ^ interface requirement mismatch
//CHECK:       ^ member 'operator[]' does not match interface requirement.
    {
        [mutating] get
        {
            storedValue += index;
            return storedValue;
        }
    }
}
