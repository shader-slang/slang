//TEST:SIMPLE(filecheck=CHK): -target hlsl -entry main

// Test for diagnostic 31224: global const/uniform variables with initializers must be static
// Exception: specialization constants are allowed to have initializers without static

// These should trigger error 31224
//CHK-DAG: ([[# @LINE + 1]]): error 31224: global const variable with initializer must be declared static: 'globalConstWithInit'
const float globalConstWithInit = 1.0f;

//CHK-DAG: ([[# @LINE + 1]]): error 31224: global const variable with initializer must be declared static: 'uniformWithInit'
uniform float uniformWithInit = 2.0f;

// These are valid and should not produce errors
static const float staticConstWithInit = 3.0f;        // OK - static const with init
const float globalConstNoInit;                        // OK - const without init
uniform float uniformNoInit;                          // OK - uniform without init

// Specialization constants with initializers are allowed without static
[SpecializationConstant]
const int specConstInt = 42;                          // OK - specialization constant with init

[vk::constant_id(5)]
const float specConstFloat = 3.14f;                   // OK - vk constant id with init

void functionScope()
{
    const float localConstWithInit = 4.0f;           // OK - local const with init
}

struct MyStruct 
{
    static const float memberStaticConst = 5.0f;    // OK - member static const
};

[shader("vertex")]
float4 main() : SV_Position
{
    return float4(staticConstWithInit, 0, 0, 1);
}