//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Empty declarations introduce no conformance requirements.
interface IEmpty { ; }
struct Empty : IEmpty {}

interface IProperty
{
    property float3 X { get; };
}

struct Property : IProperty
{
    property float3 X { get; }
}

interface IDerived : IProperty { ;; }
struct Derived : IDerived
{
    property float3 X { get; }
}

interface IGeneric<T>
{
    ;
    property T value { get; };
}

struct Generic : IGeneric<int>
{
    property int value { get; }
}
