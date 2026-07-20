// byte-address-buffer-align-error.slang

//TEST:SIMPLE(filecheck=CHECK):-target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK):-target spirv -emit-spirv-directly -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK):-target cuda -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=LOC,non-exhaustive):-target spirv -entry computeMain -stage compute

// Confirm that a non-power-of-two `alignment` for `(RW)ByteAddressBuffer` aligned load / stores is
// rejected. (Per issue #11545 a non-power-of-two alignment is reported as error 41301; an alignment
// that is a power of two but smaller than the scalar component is the separate error 41300.)

[vk::binding(2, 3)] RWByteAddressBuffer buffer;
struct Block {
    float4 val[2];
};
[shader("compute")]
[numthreads(1,1,1)]
void computeMain(uint3 threadId : SV_DispatchThreadID)
{
    // CHECK: error[E41301]: byte address buffer alignment must be a power of two
    // CHECK: error[E41301]: byte address buffer alignment must be a power of two
    buffer.Store<Block>(threadId.x, buffer.LoadAligned<Block>(threadId.x, 5));
//LOC:                                                                 ^ must be a power of two
    buffer.Store<Block>(threadId.x, buffer.Load<Block>(threadId.x), 3);
//LOC:                           ^ must be a power of two

}
