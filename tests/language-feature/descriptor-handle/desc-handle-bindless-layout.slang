//TEST:REFLECTION(filecheck=REFLECT-CUDA):-stage compute -entry computeMain -target cuda -no-codegen
//TEST:REFLECTION(filecheck=REFLECT-CPU):-stage compute -entry computeMain -target cpp -no-codegen

// Test to verify that DescriptorHandle<T> on bindless targets is laid out as a pointer,
// not as the resource type T itself.

struct TestStruct
{
    DescriptorHandle<StructuredBuffer<float>> handle;
    uint padding; // This should appear at offset sizeof(void*), not sizeof(StructuredBuffer<float>)
}

//TEST_INPUT:cbuffer(data=[0], stride=4):name=testData
ConstantBuffer<TestStruct> testData;

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    // Just use the test data to ensure it's in the reflection output
    outputBuffer[0] = testData.padding;
}

// Reflection should show the handle field is pointer-sized (8 bytes on 64-bit targets)
// and the padding field starts at offset 8

// REFLECT-CUDA: "name": "testData"
// REFLECT-CUDA: "type": {
// REFLECT-CUDA: "kind": "constantBuffer"
// REFLECT-CUDA: "elementType": {
// REFLECT-CUDA: "kind": "struct"
// REFLECT-CUDA: "fields": [
// REFLECT-CUDA: {
// REFLECT-CUDA: "name": "handle"
// REFLECT-CUDA: "binding": {"kind": "uniform", "offset": 0, "size": 8
// REFLECT-CUDA: {
// REFLECT-CUDA: "name": "padding"
// REFLECT-CUDA: "binding": {"kind": "uniform", "offset": 8, "size": 4

// REFLECT-CPU: "name": "testData"
// REFLECT-CPU: "type": {
// REFLECT-CPU: "kind": "constantBuffer"
// REFLECT-CPU: "elementType": {
// REFLECT-CPU: "kind": "struct"
// REFLECT-CPU: "fields": [
// REFLECT-CPU: {
// REFLECT-CPU: "name": "handle"
// REFLECT-CPU: "binding": {"kind": "uniform", "offset": 0, "size": 8
// REFLECT-CPU: {
// REFLECT-CPU: "name": "padding"
// REFLECT-CPU: "binding": {"kind": "uniform", "offset": 8, "size": 4
