//TEST:SIMPLE: -std legacy -no-codegen
//TEST:SIMPLE: -std 2025 -no-codegen
//TEST:SIMPLE: -std 2026 -no-codegen
//TEST:SIMPLE: -std 202c -no-codegen

// Qualified names and generic applications belong to the operand of `dyn`.
namespace Types
{
    interface IFoo<T>
    {
    }
    interface IBar
    {
    }
}

struct Foo : Types::IFoo<int>
{
}

typealias Box = dyn Types::IFoo<int>;
typealias DotQualifiedBox = dyn Types.IFoo<int>;
typealias GroupedBox = dyn (Types::IFoo<int>);
typealias ParenthesizedBox = (dyn Types::IFoo<int>);
typealias Conjunction = (Types::IFoo<int> & Types::IBar);

// A registered syntax alias follows the same type-operator path as its original keyword.
syntax boxed = dyn;
typealias RegisteredBox = boxed Types::IFoo<int>;

// Pointer and array suffixes apply to the box, both in aliases and in declarations.
typealias BoxPointer = dyn Types::IFoo<int>*;
typealias BoxArray = dyn Types::IFoo<int>[2];

BoxPointer pointerFromDeclaration(dyn Types::IFoo<int> *value)
{
    return value;
}

dyn Types::IFoo<int> *pointerFromAlias(BoxPointer value)
{
    return value;
}

BoxArray arrayFromDeclaration(dyn Types::IFoo<int> value[2])
{
    return value;
}

dyn Types::IFoo<int>[2] arrayFromAlias(BoxArray value)
{
    return value;
}

struct Holder<T>
{
    T value;
}

// Generic arguments use expression parsing, so registered type syntax must work there too.
Holder<Box> genericArgument(Holder<dyn Types::IFoo<int>> value)
{
    return value;
}

Holder<BoxPointer> genericPointerArgument(Holder<dyn Types::IFoo<int>*> value)
{
    return value;
}

Holder<Box> groupedGenericArgument(Holder<(dyn Types::IFoo<int>)> value)
{
    return value;
}

Holder<Box> nestedGenericGrouping(Holder<((dyn Types::IFoo<int>))> value)
{
    return value;
}

Holder<BoxArray> groupedGenericArray(Holder<(dyn Types::IFoo<int>)[2]> value)
{
    return value;
}

Box qualifiedAndGroupedOperands(DotQualifiedBox first, GroupedBox second, ParenthesizedBox third)
{
    first = second;
    return third;
}

Box registeredOperator(RegisteredBox value)
{
    return value;
}

Box castToBox(Foo value)
{
    return (dyn Types::IFoo<int>)value;
}

Box castDereferencedPointer(Foo* value)
{
    return (dyn Types::IFoo<int>)*value;
}

// Grouping in a known type context must not change expression grouping or cast disambiguation.
typealias GroupedInteger = ((int));

GroupedInteger expressionAndCast(int value, float other)
{
    return (value) + (int)other + ((value * 2));
}
