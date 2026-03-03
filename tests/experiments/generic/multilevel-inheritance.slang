//TEST:SIMPLE(filecheck=CHECK): -target spirv -gnone -std 2026
//TEST:SIMPLE(filecheck=CHECK): -target spirv -gminimal -std 2026
//TEST:SIMPLE(filecheck=CHECK_LEGACY): -target spirv -gnone -std 2025
//TEST:SIMPLE(filecheck=CHECK_LEGACY): -target spirv -gminimal -std 2025
//TEST:SIMPLE(filecheck=CHECK_LEGACY): -target spirv -gnone -std legacy
//TEST:SIMPLE(filecheck=CHECK_LEGACY): -target spirv -gminimal -std legacy

// CHECK: error 30811
// CHECK: struct Child : Parent {};
// CHECK: error 38029
// CHECK: getObject<Child>();

// CHECK_LEGACY: warning 30816
// CHECK_LEGACY: struct Child : Parent {};
// CHECK_LEGACY: error 38029
// CHECK_LEGACY: getObject<Child>();

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
