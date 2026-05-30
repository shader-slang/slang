//TEST:INTERPRET(filecheck=CHECK):

// Exercises lowering (not just diagnostics) of an `enum` with a `bool` tag type.
// `__EnumType` constrains its `__Tag` associated type with
// `__Tag : __BuiltinIntegerType`; `bool` is a permitted tag type that does not
// conform to `__BuiltinIntegerType`, so the enum's synthesized `__EnumType`
// conformance supplies a `NoneWitness` for that constraint. A regression there
// would break compilation/lowering of any `enum : bool`. Paired with an
// `enum : int` to also cover the real-subtype-witness branch.

enum Flag : bool
{
    No = false,
    Yes = true,
}

enum Level : int
{
    Low = 3,
    High = 9,
}

int pick(Flag f, Level l)
{
    if (f == Flag.Yes)
        return (int)l;
    return 0;
}

void main()
{
    // CHECK: 9
    printf("%d\n", pick(Flag.Yes, Level.High));

    // CHECK: 0
    printf("%d\n", pick(Flag.No, Level.High));
}
