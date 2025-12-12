// texture-capabilities-full.slang
// Complete texture capability test generator for Metal backend
//
// This is a FIDDLE replacement for the Metal tests in:
// extras/generate-tests-capabilities-for-texture-types.py
//
// Generates to: tests/fiddle/texture-capabilities/

#if 0 // FIDDLE TEMPLATE:
%-- Texture types lookup
%local textureInputs = {
%    ["Texture1D<float4>"] = "Texture1D(size=4, content = zero)",
%    ["Texture1DArray<float4>"] = "Texture1D(size=4, content = zero, arrayLength=2)",
%    ["Texture2D<float4>"] = "Texture2D(size=4, content = zero)",
%    ["Texture2DArray<float4>"] = "Texture2D(size=4, content = zero, arrayLength=2)",
%    ["Texture2DMS<float4>"] = "Texture2D(size=4, content = zero, sampleCount = two)",
%    ["Texture3D<float4>"] = "Texture3D(size=4, content = zero)",
%    ["TextureCube<float4>"] = "TextureCube(size=4, content = zero)",
%    ["TextureCubeArray<float4>"] = "TextureCube(size=4, content = zero, arrayLength=2)",
%    ["DepthTexture2D"] = "Texture2D(format=D32Float, size=4, content = zero)",
%    ["DepthTexture2DArray"] = "Texture2D(format=D32Float, size=4, content = zero, arrayLength=2)",
%    ["DepthTexture2DMS"] = "Texture2D(format=D32Float, size=4, content = zero, sampleCount = two)",
%    ["DepthTexture2DMSArray"] = "Texture2D(format=D32Float, size=4, content = zero, sampleCount = two, arrayLength=2)",
%    ["DepthTextureCube"] = "TextureCube(format=D32Float, size=4, content = zero)",
%    ["DepthTextureCubeArray"] = "TextureCube(format=D32Float, size=4, content = zero, arrayLength=2)",
%    ["RWTexture1D<float4>"] = "RWTexture1D(size=4, content = zero)",
%    ["RWTexture1DArray<float4>"] = "RWTexture1D(size=4, content = zero, arrayLength=2)",
%    ["RWTexture2D<float4>"] = "RWTexture2D(size=4, content = zero)",
%    ["RWTexture2DArray<float4>"] = "RWTexture2D(size=4, content = zero, arrayLength=2)",
%    ["RWTexture3D<float4>"] = "RWTexture3D(size=4, content = zero)",
%    ["WTexture1D<float4>"] = "RWTexture1D(size=4, content = zero)",
%    ["WTexture1DArray<float4>"] = "RWTexture1D(size=4, content = zero, arrayLength=2)",
%    ["WTexture2D<float4>"] = "RWTexture2D(size=4, content = zero)",
%    ["WTexture2DArray<float4>"] = "RWTexture2D(size=4, content = zero, arrayLength=2)",
%    ["WTexture3D<float4>"] = "RWTexture3D(size=4, content = zero)",
%    ["RasterizerOrderedTexture1D<float4>"] = "Texture1D(size=4, content = zero)",
%    ["RasterizerOrderedTexture1DArray<float4>"] = "Texture1D(size=4, content = zero, arrayLength=2)",
%    ["RasterizerOrderedTexture2D<float4>"] = "Texture2D(size=4, content = zero)",
%    ["RasterizerOrderedTexture2DArray<float4>"] = "Texture2D(size=4, content = zero, arrayLength=2)",
%    ["RasterizerOrderedTexture3D<float4>"] = "Texture3D(size=4, content = zero)",
%}
%
%-- Test operation code generator
%local function getTestOp(testOp)
%    if testOp == 3 then
%        return 'ret = int((texHandle.SampleLevel(samplerState, float3(0, 0, 0), float(0))).x);'
%    elseif testOp == 4 then
%        return 'ret = int((texHandle.SampleLevel(samplerState, float4(0, 0, 0, 0), float(0))).x);'
%    elseif testOp == 11 then
%        return 'ret = int((texHandle.Load(int(0))).x);'
%    elseif testOp == 12 then
%        return 'ret = int((texHandle.Load(int2(0, 0))).x);'
%    elseif testOp == 13 then
%        return 'ret = int((texHandle.Load(int3(0, 0, 0))).x);'
%    elseif testOp == 14 then
%        return 'ret = int((texHandle.Load(int4(0, 0, 0, 0))).x);'
%    elseif testOp == 15 then
%        return 'ret = int((texHandle.Load(int2(0, 0), 0)).x);'
%    elseif testOp == 16 then
%        return 'ret = int((texHandle.Load(int3(0, 0, 0), 0)).x);'
%    elseif testOp == 21 then
%        return 'texHandle.Store(int(0), float4(0, 0, 0, 0));\n    ret = 0;'
%    elseif testOp == 22 then
%        return 'texHandle.Store(int2(0, 0), float4(0, 0, 0, 0));\n    ret = 0;'
%    elseif testOp == 23 then
%        return 'texHandle.Store(int3(0, 0, 0), float4(0, 0, 0, 0));\n    ret = 0;'
%    elseif testOp == 42 then
%        return 'ret = int((texHandle.SampleCmpLevelZero(samplerComparisonState, float2(0, 0), float(0))).x);'
%    elseif testOp == 43 then
%        return 'ret = int((texHandle.SampleCmpLevelZero(samplerComparisonState, float3(0, 0, 0), float(0))).x);'
%    elseif testOp == 44 then
%        return 'ret = int((texHandle.SampleCmpLevelZero(samplerComparisonState, float4(0, 0, 0, 0), float(0))).x);'
%    else
%        return 'ret = 0;'
%    end
%end
%
%-- Metal tests (matching Python script lines 193-236)
%local metalTests = {
%    {type="Texture1D<float4>", minVer="1.0", testOp=12},
%    {type="Texture1DArray<float4>", minVer="1.0", testOp=13},
%    {type="Texture2D<float4>", minVer="1.0", testOp=13},
%    {type="Texture2DArray<float4>", minVer="1.0", testOp=14},
%    {type="Texture2DMS<float4>", minVer="1.0", testOp=13, bug=8457},
%    {type="Texture3D<float4>", minVer="1.0", testOp=14},
%    {type="TextureCube<float4>", minVer="1.0", testOp=3},
%    {type="TextureCubeArray<float4>", minVer="1.0", testOp=4},
%    {type="DepthTexture2D", minVer="1.0", testOp=42},
%    {type="DepthTexture2DArray", minVer="1.0", testOp=43},
%    {type="DepthTexture2DMS", minVer="1.0", testOp=15, disableCompute=8457},
%    {type="DepthTexture2DMSArray", minVer="2.0", testOp=16, disableCompute=8457},
%    {type="DepthTextureCube", minVer="1.0", testOp=43},
%    {type="DepthTextureCubeArray", minVer="1.0", testOp=44},
%    {type="RWTexture1D<float4>", minVer="1.0", testOp=11},
%    {type="RWTexture1DArray<float4>", minVer="1.0", testOp=12},
%    {type="RWTexture2D<float4>", minVer="1.0", testOp=12},
%    {type="RWTexture2DArray<float4>", minVer="1.0", testOp=13},
%    {type="RWTexture3D<float4>", minVer="1.0", testOp=13},
%    {type="WTexture1D<float4>", minVer="1.0", testOp=21},
%    {type="WTexture1DArray<float4>", minVer="1.0", testOp=22},
%    {type="WTexture2D<float4>", minVer="1.0", testOp=22},
%    {type="WTexture2DArray<float4>", minVer="1.0", testOp=23},
%    {type="WTexture3D<float4>", minVer="1.0", testOp=23},
%    {type="RasterizerOrderedTexture1D<float4>", minVer="2.0", testOp=21, disableCompute=8787},
%    {type="RasterizerOrderedTexture1DArray<float4>", minVer="2.0", testOp=22, disableCompute=8787},
%    {type="RasterizerOrderedTexture2D<float4>", minVer="2.0", testOp=22, disableCompute=8787},
%    {type="RasterizerOrderedTexture2DArray<float4>", minVer="2.0", testOp=23, disableCompute=8787},
%    {type="RasterizerOrderedTexture3D<float4>", minVer="2.0", testOp=23, disableCompute=8787},
%}
%
%-- Generate tests
%for _, test in ipairs(metalTests) do
%    local textureType = test.type
%    local testInput = textureInputs[textureType]
%    local testOp = test.testOp
%    local testOpCode = getTestOp(testOp)
%    local cleanType = string.lower(string.gsub(textureType, "[<> ]", ""))
%    local minVersion = test.minVer or "None"
%    local testTarget = "metal -profile metallib_2_3"
%    local testRuntime = "-mtl"
%    local disableSimple = test.bug and "DISABLE_TEST:" or "TEST:"
%    local disableCompute = (test.bug or test.disableCompute) and "DISABLE_TEST" or "TEST"
%    local needsSampler = (testOp >= 1 and testOp <= 9)
%    local needsComparisonSampler = (testOp >= 40 and testOp <= 49)
%    local bugIssue = test.bug or test.disableCompute
// THIS IS A GENERATED FILE. DO NOT EDIT!
// Instead, edit generate-types-tests.py and then regenerate this test
// by running: ./generate-types-tests.py
//
// Texture types capability test: metal / $textureType
// - Type supported since target version:  $minVersion
// - Target version for positive test:     2.3
// - Target version for negative test:     None
//
%    if test.bug then
// Test disabled, see https://github.com/shader-slang/slang/issues/$bugIssue
%    elseif test.disableCompute then
// Compute test disabled, see https://github.com/shader-slang/slang/issues/$bugIssue
%    end

//$(disableSimple)SIMPLE(filecheck=POSITIVE): -entry fragMain -stage fragment -target $testTarget
//$(disableCompute)(compute):COMPARE_COMPUTE(filecheck-buffer=POSITIVE_RESULT): $testRuntime
//DISABLE_TEST:SIMPLE(filecheck=NEGATIVE): -entry fragMain -stage fragment -target None

//TEST_INPUT: ubuffer(data=[0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

%    if needsSampler then
//TEST_INPUT: Sampler(filteringMode=point):name samplerState
%    end
%    if needsComparisonSampler then
//TEST_INPUT: Sampler(depthCompare):name samplerComparisonState
%    end
//TEST_INPUT: $testInput:name texHandle
$textureType texHandle;
%    if needsSampler then
SamplerState samplerState;
%    end
%    if needsComparisonSampler then
SamplerComparisonState samplerComparisonState;
%    end

[shader("fragment")]
void fragMain()
{
// POSITIVE-NOT: {{(error|warning).*}}:
// POSITIVE: result code = 0
// POSITIVE-NOT: {{(error|warning).*}}:
// POSITIVE-LABEL: fragMain
// POSITIVE-NOT: {{(error|warning).*}}:

// Expect either a compilation error or warning 41012 for upgraded profile to support functionality
// NEGATIVE: {{error [[:digit:]]+|warning 41012}}:

    int ret = 0;
    $testOpCode
    outputBuffer[0] = 0x12345 + ret;
}

[numthreads(4, 1, 1)]
void computeMain()
{
    int ret = 0;
    $testOpCode
    outputBuffer[0] = 0x12345 + ret;
}
// POSITIVE_RESULT: 12345
%    emit_test_file("texture-capabilities/gen-types-metal-" .. cleanType .. ".slang")
%end
#else // FIDDLE OUTPUT:
#endif // FIDDLE END
