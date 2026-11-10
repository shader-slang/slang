//TEST:SIMPLE: -std legacy -no-codegen
//TEST:SIMPLE: -std 2025 -no-codegen
//TEST:SIMPLE: -std 2026 -no-codegen
//TEST:SIMPLE: -std 202c -no-codegen

// We record a current parser implementation detail, not a language guarantee about declaration
// order. Initializers, parameter defaults, and type aliases are parsed as declarations are read.
// `parseOptBody` instead saves a function body as an `UnparsedStmt`; semantic checking parses the
// saved body after module declarations have populated its scope. We should revisit these tests
// if how or when syntax declarations are looked up changes, or if such names become reserved words.
namespace FunctionBinding
{
    interface IFoo
    {
    }

    struct Foo : IFoo
    {
    }

    // The later function named `dyn` is absent while these type expressions are parsed, so the
    // parser uses the syntax declaration for the alias, initializer, and default argument.
    typealias Box = dyn IFoo;
    static Box initializedBox = (dyn IFoo)Foo();

    Box defaultBox(Box value = (dyn IFoo)Foo())
    {
        return value;
    }

    // This body is parsed after the function named `dyn` has been added to `FunctionBinding`.
    int callFromEarlierBody()
    {
        return dyn(1);
    }

    int dyn(int value)
    {
        return value + 1;
    }

    // After the ordinary function is declared, immediately parsed expressions can call it.
    static int initializedValue = dyn(2);

    int defaultValue(int value = dyn(3))
    {
        return value;
    }
}

namespace TypeBinding
{
    interface IFoo
    {
    }

    // The alias is parsed before the ordinary type named `dyn` is added to this scope.
    typealias Box = dyn IFoo;

    // Parsing this body is deferred, so `dyn` denotes the ordinary type declared below.
    int constructFromEarlierBody()
    {
        dyn value = {4};
        return value.value;
    }

    struct dyn
    {
        int value;
    }

    typealias OrdinaryType = dyn;
}
