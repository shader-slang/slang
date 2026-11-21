//TEST:SIMPLE(filecheck=KILL): -target spirv-asm -profile spirv_1_5 -entry fragmentMain
//TEST:SIMPLE(filecheck=KILL): -target spirv-asm -profile spirv_1_5 -entry fragmentMain -minimum-slang-optimization
//TEST:SIMPLE(filecheck=DEMOTE): -target spirv-asm -profile spirv_1_6 -entry fragmentMain
//TEST:SIMPLE(filecheck=DEMOTE): -target spirv-asm -profile spirv_1_6 -entry fragmentMain -minimum-slang-optimization

// A terminating discard must remove the IR branch that would otherwise supply
// an incoming value to the result phi. With SSA values preserved, discard can
// be followed immediately by that branch, without any intervening store.
// A demoting discard continues execution and must keep the incoming value.

uniform uint mode;

[shader("fragment")]
float fragmentMain() : SV_Target
{
    float result;
    switch (mode & 3)
    {
    case 0:
        result = 1.0;
        break;
    case 1:
        discard;
        result = 2.0;
        break;
    default:
        result = 3.0;
        break;
    }
    return result;
}

// KILL: OpSwitch {{.*}} [[DEFAULT:%[a-zA-Z0-9_]+]] 0 [[CASE0:%[a-zA-Z0-9_]+]] 1 [[DISCARD:%[a-zA-Z0-9_]+]]
// KILL: [[DISCARD]] = OpLabel
// KILL-NEXT: OpKill
// KILL: OpPhi %float %float_3 [[DEFAULT]] %float_1 [[CASE0]]{{$}}

// DEMOTE: OpSwitch {{.*}} [[DEFAULT:%[a-zA-Z0-9_]+]] 0 [[CASE0:%[a-zA-Z0-9_]+]] 1 [[DISCARD:%[a-zA-Z0-9_]+]]
// DEMOTE: [[DISCARD]] = OpLabel
// DEMOTE-NEXT: OpDemoteToHelperInvocation
// DEMOTE: OpPhi %float %float_3 [[DEFAULT]] %float_2 [[DISCARD]] %float_1 [[CASE0]]{{$}}
