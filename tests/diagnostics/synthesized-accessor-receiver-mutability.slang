//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// A synthesized nonmutating accessor must check its forwarding expression with an immutable
// receiver. An ordinary field or mutating subscript therefore cannot satisfy either setter below.
// The differing subscript index types force the second conformance through wrapper synthesis.

interface INonmutatingProperty
{
    property int value
//CHECK:         ^^^^^ see interface requirement declaration of 'value'
    {
        get;
        [nonmutating] set;
    }
}

struct InvalidProperty : INonmutatingProperty
{
    int value;
//CHECK:^^^^^ interface requirement mismatch
//CHECK:^^^^^ member 'value' does not match interface requirement.
}

interface INonmutatingSubscript
{
    __subscript(int index) -> int
//CHECK:       ^ see interface requirement declaration of 'operator[]'
    {
        get;
        [nonmutating] set;
    }
}

struct InvalidSubscript : INonmutatingSubscript
{
    int data[1];

    __subscript(uint index) -> int
//CHECK:       ^ interface requirement mismatch
//CHECK:       ^ member 'operator[]' does not match interface requirement.
    {
        get { return data[index]; }
        set { data[index] = newValue; }
    }
}

// A static accessor has no effective `this` parameter. An exact static witness can be installed
// directly, but adapting a mismatched signature would require a type-based forwarding expression;
// it must fail as an ordinary conformance mismatch instead of synthesizing an instance wrapper.
interface IStaticSubscript
{
    static __subscript(int index) -> int
//CHECK:              ^ see interface requirement declaration of 'operator[]'
    {
        get;
    }
}

struct InvalidStaticSubscript : IStaticSubscript
{
    static __subscript(uint index) -> int
//CHECK:              ^ interface requirement mismatch
//CHECK:              ^ member 'operator[]' does not match interface requirement.
    {
        get { return 0; }
    }
}
