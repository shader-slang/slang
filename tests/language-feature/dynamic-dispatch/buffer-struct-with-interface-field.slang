// RWStructuredBuffer of a struct that contains an interface-typed field.
// This pattern models a material descriptor or scene object where
// the struct has both concrete fields and a dynamically-dispatched
// behavior field.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type -cpu
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type -emit-spirv-directly
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-llvm -compute -shaderobj -output-using-type

[anyValueSize(8)]
interface IMaterial
{
    float evaluate(float input);
}

struct Lambertian : IMaterial
{
    float albedo;
    float evaluate(float input) { return albedo * input; }
}

struct Mirror : IMaterial
{
    float reflectance;
    float evaluate(float input) { return reflectance * input * input; }
}

//TEST_INPUT: type_conformance Lambertian:IMaterial = 1
//TEST_INPUT: type_conformance Mirror:IMaterial = 2

struct SceneObject
{
    float weight;
    IMaterial material;
}

//TEST_INPUT: set objectBuffer = ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4)
RWStructuredBuffer<SceneObject> objectBuffer;

//TEST_INPUT: ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // Write a SceneObject with Lambertian material
    SceneObject obj0;
    obj0.weight = 0.5;
    Lambertian lam;
    lam.albedo = 0.8;
    obj0.material = lam;
    objectBuffer[0] = obj0;

    // Write a SceneObject with Mirror material
    SceneObject obj1;
    obj1.weight = 1.0;
    Mirror mir;
    mir.reflectance = 0.9;
    obj1.material = mir;
    objectBuffer[1] = obj1;

    // Read back and verify both concrete and interface fields
    SceneObject loaded0 = objectBuffer[0];
    SceneObject loaded1 = objectBuffer[1];

    // weight * material.evaluate(2.0)
    // Lambertian: 0.5 * (0.8 * 2.0) = 0.5 * 1.6 = 0.8
    outputBuffer[0] = loaded0.weight * loaded0.material.evaluate(2.0);

    // Mirror: 1.0 * (0.9 * 2.0 * 2.0) = 1.0 * 3.6 = 3.6
    outputBuffer[1] = loaded1.weight * loaded1.material.evaluate(2.0);

    // Verify the concrete weight field independently
    outputBuffer[2] = loaded0.weight;  // 0.5
    outputBuffer[3] = loaded1.weight;  // 1.0

    // CHECK: 0.8
    // CHECK: 3.6
    // CHECK: 0.5
    // CHECK: 1.0
}
