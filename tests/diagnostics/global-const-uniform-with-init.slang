//DIAGNOSTIC_TEST:SIMPLE(diag=diag): -target hlsl -entry main

// Test for diagnostic 31224: global const/uniform variables with initializers must be static
// Exception: specialization constants are allowed to have initializers without static
// Test for diagnostic 31219: specialization constants cannot be declared static

// These should trigger error 31224
const float globalConstWithInit = 1.0f;
/*diag:
            ^^^^^^^^^^^^^^^^^^^ global const requires static
            ^^^^^^^^^^^^^^^^^^^ global const variable with initializer must be declared static: 'globalConstWithInit'
*/

uniform float uniformWithInit = 2.0f;
/*diag:
              ^^^^^^^^^^^^^^^ global const requires static
              ^^^^^^^^^^^^^^^ global const variable with initializer must be declared static: 'uniformWithInit'
*/

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
[SpecializationConstant]
static const int staticSpecConstInt = 100;
/*diag:
                 ^^^^^^^^^^^^^^^^^^ push/specialization constant storage class error
                 ^^^^^^^^^^^^^^^^^^ push or specialization constants cannot be 'static'.
*/

[vk::constant_id(7)]
static const float staticSpecConstFloat = 2.71f;
/*diag:
                   ^^^^^^^^^^^^^^^^^^^^ push/specialization constant storage class error
                   ^^^^^^^^^^^^^^^^^^^^ push or specialization constants cannot be 'static'.
*/

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
