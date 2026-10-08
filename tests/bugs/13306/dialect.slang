//TEST:SIMPLE_EX(filecheck=CHECK): -lang hlsl tests/bugs/13306/dialect.slang -target hlsl -entry main
//TEST:SIMPLE_EX(filecheck=CHECK): -lang slang tests/bugs/13306/dialect.slang -Gec -target hlsl -entry main

// The option changes only HLSL inputs, and HLSL inputs remain immutable without the option.
uniform uint x;
cbuffer CB { uint y; };

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    x = 1;
    // CHECK: error[E30011]
    y = 2;
    // CHECK: error[E30011]
}
