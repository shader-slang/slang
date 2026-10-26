//TEST:SIMPLE(filecheck=CHECK): -std legacy -no-codegen
//TEST:SIMPLE(filecheck=CHECK): -std 2025 -no-codegen
//TEST:SIMPLE(filecheck=CHECK): -std 2026 -no-codegen
//TEST:SIMPLE(filecheck=CHECK): -std 202c -no-codegen

interface IFoo
{
    static int required();
    static int withDefault()
    {
        return 1;
    }
}

int missingConformance()
{
    return IFoo.required();
}
int missingConcreteThis()
{
    return IFoo.withDefault();
}

// CHECK-NOT: internal error
// CHECK: error[E30323]
// CHECK-NOT: internal error
// CHECK: error[E30323]
// CHECK-NOT: internal error
