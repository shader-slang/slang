//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -stage compute -entry main -capability spvDescriptorHeapEXT -O0 -validate-ir
//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -stage compute -entry main -capability spvDescriptorHeapEXT -validate-ir

// Check that a function returning a selected heap buffer is legalized to select an index,
// and that the caller can write through the resulting buffer pointer.

// CHECK: %[[VAR:[^ ]+]] = OpVariable %_ptr_Function_uint Function
// CHECK: OpStore %[[VAR]] %uint_2
// CHECK: OpStore %[[VAR]] %uint_3
// CHECK: %[[INDEX:[^ ]+]] = OpLoad %uint %[[VAR]]
// CHECK: %[[DESC:[^ ]+]] = OpUntypedAccessChainKHR {{.*}} %slang_resourceHeap %[[INDEX]]
// CHECK: %[[BUFFER:[^ ]+]] = OpBufferPointerEXT {{.*}} %[[DESC]]
// CHECK: %[[ELEMENT:[^ ]+]] = OpAccessChain {{.*}} %[[BUFFER]] %int_0
// CHECK: OpStore %[[ELEMENT]] %uint_42

cbuffer Ubo { uint x; };

[noinline]
RWStructuredBuffer<uint> choose(bool c)
{
    return c ? (RWStructuredBuffer<uint>)ResourceDescriptorHeap[2]
             : (RWStructuredBuffer<uint>)ResourceDescriptorHeap[3];
}

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    RWStructuredBuffer<uint> output = choose(x == 0);
    output[tid.x] = 42;
}
