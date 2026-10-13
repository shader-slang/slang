//TEST:SIMPLE_EX: tests/language-feature/effective-this-param-static-overload-lambda.slang -no-codegen

// Static and instance overload candidates initially share the written value qualifier inside each
// lambda. Selecting the static overload must discard that unevaluated qualifier; receiver
// validation and capture attachment apply only to the selected callable path.

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
    // Overload resolution selects the static overload based on the integer argument. Its value
    // qualifier is not evaluated, so the lambda does not capture `outer` and remains convertible
    // to a direct function type.
    return apply((int value) => outer.choose(value));
}

int invokeSelectedStaticGenericOverloadThroughLambda(StaticOverload outer)
{
    // Explicit generic application leaves both declarations available until overload resolution
    // selects the static overload based on the integer argument. Keeping the instance declaration
    // in the candidate set must not capture `outer`.
    return apply((int value) => outer.chooseGeneric<int>(value));
}
