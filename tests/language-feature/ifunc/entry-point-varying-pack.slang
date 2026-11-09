// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

//TEST:SIMPLE(filecheck=SPIRV): -target spirv -entry main -specialize Fragment
//TEST:SIMPLE(filecheck=DXIL): -target dxil -entry main -specialize Fragment
// SPIRV: OpEntryPoint Fragment
// DXIL: define void @main

struct Varyings
{
    float4 position : SV_Position;
    float4 color : COLOR;
}

struct Fragment : IFunc<float4, Varyings>
{
    float4 operator()(Varyings input) { return input.color; }
}

// Before generic-call specialization, invoking IFunc builds a MakeValuePack from the varying
// parameter. Borrow-in rewriting must update the pack through canonical operand replacement.
[shader("fragment")]
float4 main<F : IFunc<float4, Varyings>>(Varyings input, uniform F function) : SV_Target0
{
    return function(input);
}
