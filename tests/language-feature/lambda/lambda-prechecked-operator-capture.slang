//TEST:SIMPLE(filecheck=SPIRV): -target spirv -entry computeMain -validate-ir
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -output-using-type
//SPIRV: OpEntryPoint GLCompute

//TEST_INPUT: set inputBuffer = ubuffer(data=[2.0 4.0], stride=4)
StructuredBuffer<float> inputBuffer;
//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0 0], stride=4)
RWStructuredBuffer<float> outputBuffer;

// Parsing `<` checks whether its left operand is a generic declaration. For a let
// binding this can check the initializer before its lambda's capture context exists.
// The later capture walk must traverse the resulting BuiltinOperatorExpr operands.
// Load captures from a buffer so constant folding cannot hide a missing capture.
[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    float scale = inputBuffer[0];
    let scalar = (float x) =>
    {
        let uv = x / scale;
        let valid = uv >= 0.0 && uv < 1.0;
        return valid ? uv : -1.0;
    };
    outputBuffer[0] = scalar(1.0);
    // CHECK: 0.5
    outputBuffer[1] = scalar(4.0);
    // CHECK: -1

    let unary = (float x) =>
    {
        let negativeScale = -scale;
        return negativeScale < x ? negativeScale : 0.0;
    };
    outputBuffer[2] = unary(0.0);
    // CHECK: -2

    float2 extent = float2(scale, inputBuffer[1]);
    let vector = (float2 position) =>
    {
        let uv = position / extent;
        let inside = all(uv >= 0.0) && all(uv < 1.0);
        return inside ? uv.x + uv.y : -1.0;
    };
    outputBuffer[3] = vector(float2(1.0));
    // CHECK: 0.75

    let nested = (float x) =>
    {
        let inner = (float y) =>
        {
            let difference = y - scale;
            return difference < x ? difference : -1.0;
        };
        return inner(3.0);
    };
    outputBuffer[4] = nested(2.0);
    // CHECK: 1

    // This spelling does not eagerly check uv while parsing generic arguments.
    let reversedComparison = (float x) =>
    {
        let uv = x / scale;
        return 1.0 > uv ? uv : -1.0;
    };
    outputBuffer[5] = reversedComparison(1.0);
    // CHECK: 0.5
}
