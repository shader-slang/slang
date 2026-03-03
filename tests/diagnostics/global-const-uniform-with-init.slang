//TEST:SIMPLE(filecheck=CHK): -target hlsl -entry main

// Test for diagnostic 31224: global const/uniform variables with initializers must be static
// Exception: specialization constants are allowed to have initializers without static
// Test for diagnostic 31219: specialization constants cannot be declared static

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

// These should trigger error 31219: static specialization constants are not allowed
//CHK-DAG: ([[# @LINE + 2]]): error 31219: push or specialization constants cannot be 'static'.
[SpecializationConstant]
static const int staticSpecConstInt = 100;

//CHK-DAG: ([[# @LINE + 2]]): error 31219: push or specialization constants cannot be 'static'.
[vk::constant_id(7)]
static const float staticSpecConstFloat = 2.71f;

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