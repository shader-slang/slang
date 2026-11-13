//TEST:SIMPLE: -std legacy -no-codegen
//TEST:SIMPLE: -std 2025 -no-codegen
//TEST:SIMPLE: -std 2026 -no-codegen
//TEST:SIMPLE: -std 202c -no-codegen

// We recognize `dyn` as type syntax only while its registered syntax declaration is visible.
// Ordinary bindings in a closer scope hide that declaration, including an overload set.
namespace UnshadowedSyntax
{
    interface IFoo
    {
    }

    typealias Box = dyn IFoo;
    typealias GroupedBox = dyn (IFoo);

    Box sameBoxType(GroupedBox value)
    {
        return value;
    }
}

namespace ValueBindings
{
    int parameterValue(int dyn)
    {
        return dyn + 1;
    }

    int localValue()
    {
        int dyn = 3;
        return dyn + 1;
    }
}

namespace GlobalValueBinding
{
    static int dyn = 5;
    static int initializedValue = dyn;

    int globalValue()
    {
        return dyn + initializedValue;
    }
}

namespace TypeBinding
{
    struct dyn
    {
        int value;
    }

    dyn sameType(dyn value)
    {
        return value;
    }
}

namespace FunctionBindings
{
    int dyn(int value)
    {
        return value + 1;
    }

    float dyn(float value)
    {
        return value + 2.0;
    }

    int integerCall()
    {
        return dyn(3);
    }

    float floatingPointCall()
    {
        return dyn(4.0);
    }
}
