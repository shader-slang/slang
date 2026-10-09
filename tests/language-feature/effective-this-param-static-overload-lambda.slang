//TEST:SIMPLE_EX: tests/language-feature/effective-this-param-static-overload-lambda.slang -no-codegen

int apply(functype(int) -> int functionValue)
{
    return functionValue(1);
}

struct StaticOverload
{
    static int choose(int value) { return value; }

    [nonmutating]
    int choose(float value) { return int(value); }

    static int chooseGeneric<T>(int value) { return value; }

    [nonmutating]
    int chooseGeneric<T>(float value) { return int(value); }
};

int invokeSelectedStaticOverloadThroughLambda(StaticOverload outer)
{
    // The integer argument selects the static overload. Its value qualifier is not evaluated, so
    // the lambda does not capture `outer` and remains convertible to a direct function type.
    return apply((int value) => outer.choose(value));
}

int invokeSelectedStaticGenericOverloadThroughLambda(StaticOverload outer)
{
    // Explicit generic application retains both declarations until the integer argument selects
    // the static overload. Retaining the instance candidate must not capture `outer`.
    return apply((int value) => outer.chooseGeneric<int>(value));
}
