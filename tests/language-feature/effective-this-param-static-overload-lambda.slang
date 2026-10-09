//TEST:SIMPLE_EX: -lang hlsl tests/language-feature/effective-this-param-static-overload-lambda.slang -no-codegen

int apply(functype(int) -> int functionValue)
{
    return functionValue(1);
}

struct StaticOverload
{
    static int choose(int value) { return value; }
    static int choose(float value) { return int(value); }
    static T choose<T>(T value) { return value; }
    float choose<T>(float value) { return value; }

    int invokeThroughLambda()
    {
        // Selecting the static member function means the lambda does not use or capture `this`, so
        // it can convert to a direct function type.
        return apply((int value) => choose(value));
    }

    int invokeGenericThroughLambda()
    {
        // The same remains true after explicit generic arguments leave both candidates for the
        // enclosing call to resolve.
        return apply((int value) => choose<int>(value));
    }
};
