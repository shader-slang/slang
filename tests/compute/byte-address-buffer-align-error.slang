// byte-address-buffer-align-error.slang

//TEST:SIMPLE(filecheck=CHECK):-target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK):-target spirv -emit-spirv-directly -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK):-target cuda -entry computeMain -stage compute

// Confirm compilation of `(RW)ByteAddressBuffer` with aligned load / stores to wider data types.

[vk::binding(2, 3)] RWByteAddressBuffer buffer;
struct Block {
    float4 val[2];
};
[shader("compute")]
[numthreads(1,1,1)]
void computeMain(uint3 threadId : SV_DispatchThreadID)
{
    uint address = threadId.x;

    // CHECK: error[E41300]: invalid byte address buffer alignment
    // CHECK: error[E41300]: invalid byte address buffer alignment
    buffer.Store<Block>(address, buffer.LoadAligned<Block>(address, 5));
    buffer.Store<Block>(address, buffer.Load<Block>(address), 3);

}

