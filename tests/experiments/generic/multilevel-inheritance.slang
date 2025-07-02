//TEST:SIMPLE(filecheck=CHECK): -target spirv -gnone
//TEST:SIMPLE(filecheck=CHECK_DEBUG_INFO): -target spirv -gminimal

// CHECK: support for inheritance is unstable
// CHECK: struct Child : Parent {};
// CHECK-NOT: Unhandled global inst in spirv-emit
// CHECK-NOT: let  %1 : %2    = lookupWitness(%base, %3)

// CHECK_DEBUG_INFO: support for inheritance is unstable
// CHECK_DEBUG_INFO: struct Child : Parent {};
// CHECK_DEBUG_INFO: Unhandled global inst in spirv-emit
// CHECK_DEBUG_INFO: let  %1 : %2    = lookupWitness(%base, %3)

interface IFoo {};
struct Parent : IFoo {};
struct Child : Parent {};

T getObject<T>() where T : IFoo
{
    return T();
}

[shader("compute")]
void main()
{
    getObject<Child>();
}
