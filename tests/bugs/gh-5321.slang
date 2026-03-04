//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv

uniform uint FillType_Solid = 0u;
//CHECK:     ^^^^^^^^^^^^^^ global const requires static
//CHECK:     ^^^^^^^^^^^^^^ global const variable with initializer must be declared static: 'FillType_Solid'

[shader("compute")]
float cs(uniform uint x)
    : SV_Target
{
    switch (x)
    {
    case FillType_Solid:
//CHECK: ^^^^^^^^^^^^^^ expression does not evaluate to a compile-time constant
//CHECK: ^^^^^^^^^^^^^^ expression does not evaluate to a compile-time constant
        return 0.0;
    }
    return 0.0;
}