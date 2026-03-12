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

Lambertian createLambertian(float albedo) { Lambertian l; l.albedo = albedo; return l; }
Mirror createMirror(float reflectance) { Mirror m; m.reflectance = reflectance; return m; }

SceneObject createSceneObject(float weight, IMaterial mat)
{
    SceneObject obj;
    obj.weight = weight;
    obj.material = mat;
    return obj;
}

//TEST_INPUT: ubuffer(data=[2 3 4 5], stride=4):name=inputParams
StructuredBuffer<uint> inputParams;

[numthreads(1, 1, 1)]
void computeMain()
{
    objectBuffer[0] = createSceneObject(float(inputParams[0]), createLambertian(float(inputParams[1])));
    objectBuffer[1] = createSceneObject(float(inputParams[2]), createMirror(float(inputParams[3])));

    // Read back and verify both concrete and interface fields
    SceneObject loaded0 = objectBuffer[0];
    SceneObject loaded1 = objectBuffer[1];

    // weight * material.evaluate(2.0)
    // Lambertian: 2.0 * (3.0 * 2.0) = 2.0 * 6.0 = 12.0
    outputBuffer[0] = loaded0.weight * loaded0.material.evaluate(2.0);

    // Mirror: 4.0 * (5.0 * 2.0 * 2.0) = 4.0 * 20.0 = 80.0
    outputBuffer[1] = loaded1.weight * loaded1.material.evaluate(2.0);

    // Verify the concrete weight field independently
    outputBuffer[2] = loaded0.weight;  // 2.0
    outputBuffer[3] = loaded1.weight;  // 4.0

    // CHECK: 12.0
    // CHECK: 80.0
    // CHECK: 2.0
    // CHECK: 4.0
}
