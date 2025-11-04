#!/usr/bin/python3

import sys


# TEST OP
#
# sampler ops, level 0
#   1  - sample with 1-dimensional coord
#   2  - sample with 2-dimensional coord
#   3  - sample with 3-dimensional coord
#   4  - sample with 4-dimensional coord
#
# load ops without sampler state
#  11  - load with 1-dimensional coord
#  12  - load with 2-dimensional coord
#  13  - load with 3-dimensional coord
#  14  - load with 4-dimensional coord
#  15  - load with 2-dimensional coord + sample index
#  16  - load with 3-dimensional coord + sample index
#
# store ops
#  21  - store with 1-dimensional coord
#  22  - store with 2-dimensional coord
#  23  - store with 3-dimensional coord
#
# feedback sampler ops, level 0
#  32  - write sampler feedback with 2-dimensional coord
#  33  - write sampler feedback with 3-dimensional coord
#
# depth/shadow sampler ops, level 0
#  41  - sample with 1-dimensional coord
#  42  - sample with 2-dimensional coord
#  43  - sample with 3-dimensional coord
#  44  - sample with 4-dimensional coord

# Test info for Target/Texture type combo
class TestInfo:

    # Constructor
    #
    # minVersion: Minimum backend version required for the test type. Version 'None'
    #             signifies that the texture type is not supported at all on the target.
    #
    # testOp:     Test operation code (see the list above)
    def __init__(self, minVersion, testOp):
        self.minVersion = minVersion
        self.testOp = testOp
        self.disableForIssue = None
        self.disableComputeForIssue = None
        self.disableNegativeTestForIssue = None

    # Mark the test completely skipped due to a bug
    def bug(self, issue):
        self.disableForIssue = issue
        return self

    # Mark the functional compute test skipped due to a bug
    def disableComputeTest(self, issue):
        self.disableComputeForIssue = issue
        return self

    # Mark the negative test skipped due to a bug
    def disableNegativeTest(self, issue):
        self.disableNegativeTestForIssue = issue
        return self

def getWgslTests():

    backend = {
        "name" : "wgsl",
        "versions" : [
            ("1.0", "wgsl", "-wgpu")
        ]
    }

    tests = {
        "Texture1D<float4>"        : TestInfo("1.0", 12).disableComputeTest(8786),
        "Texture1DArray<float4>"   : TestInfo(None,  13),
        "Texture2D<float4>"        : TestInfo("1.0", 13),
        "Texture2DArray<float4>"   : TestInfo("1.0", 14),
        "Texture2DMS<float4>"      : TestInfo("1.0", 13).disableComputeTest(8786),
        "Texture2DMSArray<float4>" : TestInfo(None,  14),
        "Texture3D<float4>"        : TestInfo("1.0", 14),
        "TextureCube<float4>"      : TestInfo("1.0", 3),
        "TextureCubeArray<float4>" : TestInfo("1.0", 4),

        "DepthTexture1D"           : TestInfo(None,  41),
        "DepthTexture1DArray"      : TestInfo(None,  42),
        "DepthTexture2D"           : TestInfo("1.0", 42).disableComputeTest(8786),
        "DepthTexture2DArray"      : TestInfo("1.0", 43).disableComputeTest(8786),
        "DepthTexture2DMS"         : TestInfo("1.0", 15).disableComputeTest(8786),
        "DepthTexture2DMSArray"    : TestInfo(None,  16),
        "DepthTexture3D"           : TestInfo(None,  43),
        "DepthTextureCube"         : TestInfo("1.0", 43).disableComputeTest(8786),
        "DepthTextureCubeArray"    : TestInfo("1.0", 44).disableComputeTest(8786),

        "RWTexture1D<float4>"        : TestInfo("1.0", 11).disableComputeTest(8786),
        "RWTexture1DArray<float4>"   : TestInfo(None,  12),
        "RWTexture2D<float4>"        : TestInfo("1.0", 12).disableComputeTest(8786),
        "RWTexture2DArray<float4>"   : TestInfo("1.0", 13).disableComputeTest(8786),
        "RWTexture2DMS<float4>"      : TestInfo(None,  15),
        "RWTexture2DMSArray<float4>" : TestInfo(None,  16),
        "RWTexture3D<float4>"        : TestInfo("1.0", 13).disableComputeTest(8786),

        "WTexture1D<float4>"        : TestInfo("1.0", 21).disableComputeTest(8786),
        "WTexture1DArray<float4>"   : TestInfo(None,  22),
        "WTexture2D<float4>"        : TestInfo("1.0", 22).disableComputeTest(8786),
        "WTexture2DArray<float4>"   : TestInfo("1.0", 23).disableComputeTest(8786),
        "WTexture3D<float4>"        : TestInfo("1.0", 23).disableComputeTest(8786),

        "FeedbackTexture2D<SAMPLER_FEEDBACK_MIN_MIP>"      : TestInfo(None,  32),
        "FeedbackTexture2DArray<SAMPLER_FEEDBACK_MIN_MIP>" : TestInfo(None,  33),

        "RasterizerOrderedTexture1D<float4>"        : TestInfo(None,  21),
        "RasterizerOrderedTexture1DArray<float4>"   : TestInfo(None,  22),
        "RasterizerOrderedTexture2D<float4>"        : TestInfo(None,  22),
        "RasterizerOrderedTexture2DArray<float4>"   : TestInfo(None,  23),
        "RasterizerOrderedTexture3D<float4>"        : TestInfo(None,  23),
    }

    return (backend, tests)


def getMetalTests():

    backend = {
        "name" : "metal",
        "versions" : [
            ("1.0", None, None), # unsupported by Slang
            ("2.0", None, None), # unsupported by Slang
            ("2.3", "metal -profile metallib_2_3", "-mtl"),
            ("2.4", "metal -profile metallib_2_4", "-mtl"),
        ]
    }

    tests = {
        "Texture1D<float4>"        : TestInfo("1.0", 12),
        "Texture1DArray<float4>"   : TestInfo("1.0", 13),
        "Texture2D<float4>"        : TestInfo("1.0", 13),
        "Texture2DArray<float4>"   : TestInfo("1.0", 14),
        "Texture2DMS<float4>"      : TestInfo("1.0", 13).bug(8457),
        "Texture2DMSArray<float4>" : TestInfo(None,  14),
        "Texture3D<float4>"        : TestInfo("1.0", 14),
        "TextureCube<float4>"      : TestInfo("1.0", 3),
        "TextureCubeArray<float4>" : TestInfo("1.0", 4),

        "DepthTexture1D"           : TestInfo(None,  41).bug(8721),
        "DepthTexture1DArray"      : TestInfo(None,  42).bug(8721),
        "DepthTexture2D"           : TestInfo("1.0", 42),
        "DepthTexture2DArray"      : TestInfo("1.0", 43),
        "DepthTexture2DMS"         : TestInfo("1.0", 15).disableComputeTest(8457),
        "DepthTexture2DMSArray"    : TestInfo("2.0", 16).disableComputeTest(8457),
        "DepthTexture3D"           : TestInfo(None,  43).bug(8721),
        "DepthTextureCube"         : TestInfo("1.0", 43),
        "DepthTextureCubeArray"    : TestInfo("1.0", 44),

        "RWTexture1D<float4>"        : TestInfo("1.0", 11),
        "RWTexture1DArray<float4>"   : TestInfo("1.0", 12),
        "RWTexture2D<float4>"        : TestInfo("1.0", 12),
        "RWTexture2DArray<float4>"   : TestInfo("1.0", 13),
        "RWTexture2DMS<float4>"      : TestInfo(None,  15).bug(8721),
        "RWTexture2DMSArray<float4>" : TestInfo(None,  16).bug(8721),
        "RWTexture3D<float4>"        : TestInfo("1.0", 13),

        "WTexture1D<float4>"        : TestInfo("1.0", 21),
        "WTexture1DArray<float4>"   : TestInfo("1.0", 22),
        "WTexture2D<float4>"        : TestInfo("1.0", 22),
        "WTexture2DArray<float4>"   : TestInfo("1.0", 23),
        "WTexture3D<float4>"        : TestInfo("1.0", 23),

        "FeedbackTexture2D<SAMPLER_FEEDBACK_MIN_MIP>"      : TestInfo(None,  32),
        "FeedbackTexture2DArray<SAMPLER_FEEDBACK_MIN_MIP>" : TestInfo(None,  33),

        "RasterizerOrderedTexture1D<float4>"        : TestInfo("2.0", 21).disableComputeTest(8787),
        "RasterizerOrderedTexture1DArray<float4>"   : TestInfo("2.0", 22).disableComputeTest(8787),
        "RasterizerOrderedTexture2D<float4>"        : TestInfo("2.0", 22).disableComputeTest(8787),
        "RasterizerOrderedTexture2DArray<float4>"   : TestInfo("2.0", 23).disableComputeTest(8787),
        "RasterizerOrderedTexture3D<float4>"        : TestInfo("2.0", 23).disableComputeTest(8787),
    }

    return (backend, tests)


def getGlslTests(nameSuffix, simpleTarget, vkTargetAdditionalFlags):

    backend = {
        "name" : f"glsl{nameSuffix}",
        "positive-label" : "main",
        "versions" : [
            ("110", None, None), # unsupported by Slang
            ("120", None, None), # unsupported by Slang
            ("130", None, None), # unsupported by Slang
            ("140", None, None), # unsupported by Slang
            ("150", f"{simpleTarget} -profile glsl_150", f"-vk {vkTargetAdditionalFlags}"),
            ("330", f"{simpleTarget} -profile glsl_330", f"-vk {vkTargetAdditionalFlags}"),
            ("400", f"{simpleTarget} -profile glsl_400", f"-vk {vkTargetAdditionalFlags}"),
            ("410", f"{simpleTarget} -profile glsl_410", f"-vk {vkTargetAdditionalFlags}"),
            ("420", f"{simpleTarget} -profile glsl_420", f"-vk {vkTargetAdditionalFlags}"),
            ("430", f"{simpleTarget} -profile glsl_430", f"-vk {vkTargetAdditionalFlags}"),
            ("440", f"{simpleTarget} -profile glsl_440", f"-vk {vkTargetAdditionalFlags}"),
            ("450", f"{simpleTarget} -profile glsl_450", f"-vk {vkTargetAdditionalFlags}"),
            ("460", f"{simpleTarget} -profile glsl_460", f"-vk {vkTargetAdditionalFlags}"),
        ]
    }

    tests = {
        "Texture1D<float4>"        : TestInfo("110", 1),
        "Texture1DArray<float4>"   : TestInfo("130", 2),
        "Texture2D<float4>"        : TestInfo("110", 2),
        "Texture2DArray<float4>"   : TestInfo("130", 3),
        "Texture2DMS<float4>"      : TestInfo(None,  13), # accessibe only via combined texture+sampler
        "Texture2DMSArray<float4>" : TestInfo(None,  14), # accessibe only via combined texture+sampler
        "Texture3D<float4>"        : TestInfo("110", 3),
        "TextureCube<float4>"      : TestInfo("110", 3),
        "TextureCubeArray<float4>" : TestInfo("400", 4).disableNegativeTest(8912),

        "DepthTexture1D"           : TestInfo("110", 41).bug(8802),
        "DepthTexture1DArray"      : TestInfo("130", 42).bug(8802),
        "DepthTexture2D"           : TestInfo("110", 42).bug(8802),
        "DepthTexture2DArray"      : TestInfo("130", 43).bug(8802),
        "DepthTexture2DMS"         : TestInfo(None,  15),
        "DepthTexture2DMSArray"    : TestInfo(None,  16),
        "DepthTexture3D"           : TestInfo(None,  43),
        "DepthTextureCube"         : TestInfo("110", 43).bug(8802),
        "DepthTextureCubeArray"    : TestInfo("400", 44).bug(8802),

        "RWTexture1D<float4>"        : TestInfo("420", 11).disableNegativeTest(8908),
        "RWTexture1DArray<float4>"   : TestInfo("420", 12).disableNegativeTest(8908),
        "RWTexture2D<float4>"        : TestInfo("420", 12).disableNegativeTest(8908),
        "RWTexture2DArray<float4>"   : TestInfo("420", 13).disableNegativeTest(8908),
        "RWTexture2DMS<float4>"      : TestInfo("420", 15).disableNegativeTest(8908),
        "RWTexture2DMSArray<float4>" : TestInfo("420", 16).disableNegativeTest(8908),
        "RWTexture3D<float4>"        : TestInfo("420", 13).disableNegativeTest(8908),

        "WTexture1D<float4>"        : TestInfo("420", 21).disableNegativeTest(8908),
        "WTexture1DArray<float4>"   : TestInfo("420", 22).disableNegativeTest(8908),
        "WTexture2D<float4>"        : TestInfo("420", 22).disableNegativeTest(8908),
        "WTexture2DArray<float4>"   : TestInfo("420", 23).disableNegativeTest(8908),
        "WTexture3D<float4>"        : TestInfo("420", 23).disableNegativeTest(8908),

        "FeedbackTexture2D<SAMPLER_FEEDBACK_MIN_MIP>"      : TestInfo(None,  32),
        "FeedbackTexture2DArray<SAMPLER_FEEDBACK_MIN_MIP>" : TestInfo(None,  33),

        "RasterizerOrderedTexture1D<float4>"        : TestInfo("450", 21).disableComputeTest(8787).disableNegativeTest(8908), # note: this feature requires ARB_fragment_shader_interlock
        "RasterizerOrderedTexture1DArray<float4>"   : TestInfo("450", 22).disableComputeTest(8787).disableNegativeTest(8908), # note: this feature requires ARB_fragment_shader_interlock
        "RasterizerOrderedTexture2D<float4>"        : TestInfo("450", 22).disableComputeTest(8787).disableNegativeTest(8908), # note: this feature requires ARB_fragment_shader_interlock
        "RasterizerOrderedTexture2DArray<float4>"   : TestInfo("450", 23).disableComputeTest(8787).disableNegativeTest(8908), # note: this feature requires ARB_fragment_shader_interlock
        "RasterizerOrderedTexture3D<float4>"        : TestInfo("450", 23).disableComputeTest(8787).disableNegativeTest(8908), # note: this feature requires ARB_fragment_shader_interlock
    }

    return (backend, tests)


def getPositiveNegativeBackendTargets(backend, minSupportedVersion):

    negativeTestTarget = None
    minimumVersionMet = False

    for versionTuple in backend["versions"]:
        if versionTuple[0] == minSupportedVersion:
            minimumVersionMet = True

        # is this version unsupported by Slang
        if versionTuple[1] is None:
            continue

        # supported, so update negative or positive test target accordingly
        if minimumVersionMet:
            return (versionTuple, negativeTestTarget)
        else:
            negativeTestTarget = versionTuple

    # if we get here, we didn't find a working target version for the positive test
    if (minSupportedVersion is None) or minimumVersionMet:
        return (None, negativeTestTarget)

    # a test refers to undefined target version, bail out
    print(f"Undefined version: {minSupportedVersion}")
    sys.exit(1)

def emitTestOp(f, testOp):
    if testOp == 1:
        f.write("    ret = int((texHandle.SampleLevel(samplerState, float(0), float(0))).x);\n")
    elif testOp == 2:
        f.write("    ret = int((texHandle.SampleLevel(samplerState, float2(0, 0), float(0))).x);\n")
    elif testOp == 3:
        f.write("    ret = int((texHandle.SampleLevel(samplerState, float3(0, 0, 0), float(0))).x);\n")
    elif testOp == 4:
        f.write("    ret = int((texHandle.SampleLevel(samplerState, float4(0, 0, 0, 0), float(0))).x);\n")
    elif testOp == 11:
        f.write("    ret = int((texHandle.Load(int(0))).x);\n")
    elif testOp == 12:
        f.write("    ret = int((texHandle.Load(int2(0, 0))).x);\n")
    elif testOp == 13:
        f.write("    ret = int((texHandle.Load(int3(0, 0, 0))).x);\n")
    elif testOp == 14:
        f.write("    ret = int((texHandle.Load(int4(0, 0, 0, 0))).x);\n")
    elif testOp == 15:
        f.write("    ret = int((texHandle.Load(int2(0, 0), 0)).x);\n")
    elif testOp == 16:
        f.write("    ret = int((texHandle.Load(int3(0, 0, 0), 0)).x);\n")
    elif testOp == 21:
        f.write("    texHandle.Store(int(0), float4(0, 0, 0, 0));\n")
        f.write("    ret = 0;\n")
    elif testOp == 22:
        f.write("    texHandle.Store(int2(0, 0), float4(0, 0, 0, 0));\n")
        f.write("    ret = 0;\n")
    elif testOp == 23:
        f.write("    texHandle.Store(int3(0, 0, 0), float4(0, 0, 0, 0));\n")
        f.write("    ret = 0;\n")
    elif testOp == 32:
        f.write("    texHandle.WriteSamplerFeedbackLevel(feedbackSamplerInput2D, samplerState, float2(0, 0), float(0));\n")
        f.write("    ret = 0;\n")
    elif testOp == 33:
        f.write("    texHandle.WriteSamplerFeedbackLevel(feedbackSamplerInput2DArray, samplerState, float3(0, 0, 0), float(0));\n")
        f.write("    ret = 0;\n")
    elif testOp == 41:
        f.write("    ret = int((texHandle.SampleCmpLevelZero(samplerComparisonState, float(0), float(0))).x);\n")
    elif testOp == 42:
        f.write("    ret = int((texHandle.SampleCmpLevelZero(samplerComparisonState, float2(0, 0), float(0))).x);\n")
    elif testOp == 43:
        f.write("    ret = int((texHandle.SampleCmpLevelZero(samplerComparisonState, float3(0, 0, 0), float(0))).x);\n")
    elif testOp == 44:
        f.write("    ret = int((texHandle.SampleCmpLevelZero(samplerComparisonState, float4(0, 0, 0, 0), float(0))).x);\n")
    else:
        print(f"Undefined test op code: {testOp}")
        sys.exit(1)

def generateSingleTest(filepath, backend, testType, testInfo):

    testInputForTestType = {
        "Texture1D<float4>"        : "Texture1D(size=4, content = zero)",
        "Texture1DArray<float4>"   : "Texture1D(size=4, content = zero, arrayLength=2)",
        "Texture2D<float4>"        : "Texture2D(size=4, content = zero)",
        "Texture2DArray<float4>"   : "Texture2D(size=4, content = zero, arrayLength=2)",
        "Texture2DMS<float4>"      : "Texture2D(size=4, content = zero, sampleCount = two)",
        "Texture2DMSArray<float4>" : "Texture2D(size=4, content = zero, sampleCount = two, arrayLength=2)",
        "Texture3D<float4>"        : "Texture3D(size=4, content = zero)",
        "TextureCube<float4>"      : "TextureCube(size=4, content = zero)",
        "TextureCubeArray<float4>" : "TextureCube(size=4, content = zero, arrayLength=2)",

        "DepthTexture1D"        : "Texture1D(format=D32Float, size=4, content = zero)",
        "DepthTexture1DArray"   : "Texture1D(format=D32Float, size=4, content = zero, arrayLength=2)",
        "DepthTexture2D"        : "Texture2D(format=D32Float, size=4, content = zero)",
        "DepthTexture2DArray"   : "Texture2D(format=D32Float, size=4, content = zero, arrayLength=2)",
        "DepthTexture2DMS"      : "Texture2D(format=D32Float, size=4, content = zero, sampleCount = two)",
        "DepthTexture2DMSArray" : "Texture2D(format=D32Float, size=4, content = zero, sampleCount = two, arrayLength=2)",
        "DepthTexture3D"        : "Texture3D(format=D32Float, size=4, content = zero)",
        "DepthTextureCube"      : "TextureCube(format=D32Float, size=4, content = zero)",
        "DepthTextureCubeArray" : "TextureCube(format=D32Float, size=4, content = zero, arrayLength=2)",

        "RWTexture1D<float4>"        : "RWTexture1D(size=4, content = zero)",
        "RWTexture1DArray<float4>"   : "RWTexture1D(size=4, content = zero, arrayLength=2)",
        "RWTexture2D<float4>"        : "RWTexture2D(size=4, content = zero)",
        "RWTexture2DArray<float4>"   : "RWTexture2D(size=4, content = zero, arrayLength=2)",
        "RWTexture2DMS<float4>"      : "RWTexture2D(size=4, content = zero, sampleCount = two)",
        "RWTexture2DMSArray<float4>" : "RWTexture2D(size=4, content = zero, sampleCount = two, arrayLength=2)",
        "RWTexture3D<float4>"        : "RWTexture3D(size=4, content = zero)",

        "WTexture1D<float4>"        : "RWTexture1D(size=4, content = zero)",
        "WTexture1DArray<float4>"   : "RWTexture1D(size=4, content = zero, arrayLength=2)",
        "WTexture2D<float4>"        : "RWTexture2D(size=4, content = zero)",
        "WTexture2DArray<float4>"   : "RWTexture2D(size=4, content = zero, arrayLength=2)",
        "WTexture3D<float4>"        : "RWTexture3D(size=4, content = zero)",

        "FeedbackTexture2D<SAMPLER_FEEDBACK_MIN_MIP>"      : "Texture2D(size=4, content = zero)",
        "FeedbackTexture2DArray<SAMPLER_FEEDBACK_MIN_MIP>" : "Texture2D(size=4, content = zero, arrayLength=2)",

        "RasterizerOrderedTexture1D<float4>"        : "Texture1D(size=4, content = zero)",
        "RasterizerOrderedTexture1DArray<float4>"   : "Texture1D(size=4, content = zero, arrayLength=2)",
        "RasterizerOrderedTexture2D<float4>"        : "Texture2D(size=4, content = zero)",
        "RasterizerOrderedTexture2DArray<float4>"   : "Texture2D(size=4, content = zero, arrayLength=2)",
        "RasterizerOrderedTexture3D<float4>"        : "Texture3D(size=4, content = zero)",
    }

    # Determine the first supporting version and highest non-supporting version
    (positiveTestTarget, negativeTestTarget) = \
        getPositiveNegativeBackendTargets(backend, testInfo.minVersion)

    positiveTestTargetVersion = None
    if positiveTestTarget is not None:
        positiveTestTargetVersion = positiveTestTarget[0]

    negativeTestTargetVersion = None
    if negativeTestTarget is not None:
        negativeTestTargetVersion = negativeTestTarget[0]

    # Determine which tests to generate
    generateNegativeTest = negativeTestTarget is not None
    generatePositiveTest = positiveTestTarget is not None
    generatePositiveComputeTest = generatePositiveTest

    with open(filepath, "w") as f:
        f.write(f"// THIS IS A GENERATED FILE. DO NOT EDIT!\n")
        f.write( "// Instead, edit generate-types-tests.py and then regenerate this test\n")
        f.write( "// by running: ./generate-types-tests.py\n")
        f.write( "//\n")
        f.write(f"// Texture types capability test: {backend['name']} / {testType}\n")
        f.write(f"// - Type supported since target version:  {testInfo.minVersion}\n")
        f.write(f"// - Target version for positive test:     {positiveTestTargetVersion}\n")
        f.write(f"// - Target version for negative test:     {negativeTestTargetVersion}\n")
        f.write( "//\n")
        f.write( "\n")

        # determine if tests are disabled
        disableTestPrefix = ""
        disableComputeTestPrefix = ""
        disableNegativeTestPrefix = ""
        if testInfo.disableForIssue is not None:
            f.write(f"// Test disabled, see https://github.com/shader-slang/slang/issues/{testInfo.disableForIssue}\n")
            disableTestPrefix = "DISABLE_"
            disableNegativeTestPrefix = "DISABLE_"
            disableComputeTestPrefix = "DISABLE_"

        if disableComputeTestPrefix == "" and testInfo.disableComputeForIssue is not None:
            f.write(f"// Compute test disabled, see https://github.com/shader-slang/slang/issues/{testInfo.disableComputeForIssue}\n")
            disableComputeTestPrefix = "DISABLE_"

        if disableNegativeTestPrefix == "" and testInfo.disableNegativeTestForIssue is not None:
            f.write(f"// Negative test disabled, see https://github.com/shader-slang/slang/issues/{testInfo.disableNegativeTestForIssue}\n")
            disableNegativeTestPrefix = "DISABLE_"

        if generatePositiveTest:
            f.write(f"//{disableTestPrefix}TEST:SIMPLE(filecheck=POSITIVE): -entry fragMain -stage fragment -target {positiveTestTarget[1]}\n")

        if generatePositiveComputeTest:
            f.write(f"//{disableComputeTestPrefix}TEST(compute):COMPARE_COMPUTE(filecheck-buffer=POSITIVE_RESULT): {positiveTestTarget[2]}\n")

        if generateNegativeTest:
            f.write(f"//{disableNegativeTestPrefix}TEST:SIMPLE(filecheck=NEGATIVE): -entry fragMain -stage fragment -target {negativeTestTarget[1]}\n")

        f.write( "\n")
        f.write( "//TEST_INPUT: ubuffer(data=[0], stride=4):out,name outputBuffer\n")
        f.write( "RWStructuredBuffer<int> outputBuffer;\n")
        f.write( "\n")
        f.write(f"//TEST_INPUT: {testInputForTestType[testType]}:name texHandle\n")
        f.write(f"{testType} texHandle;\n");
        f.write( "\n")

        if (testInfo.testOp in range(0, 9)) or (testInfo.testOp in range(30, 39)):
            f.write( "//TEST_INPUT: Sampler(filteringMode=point):name samplerState\n")
            f.write( "SamplerState samplerState;\n")
            f.write( "\n")

        if testInfo.testOp in range(40, 49):
            f.write( "//TEST_INPUT: Sampler(depthCompare):name samplerComparisonState\n")
            f.write( "SamplerComparisonState samplerComparisonState;\n")
            f.write( "\n")

        if testInfo.testOp == 32:
            f.write( "Texture2D<float4> feedbackSamplerInput2D;\n")
            f.write( "\n")

        if testInfo.testOp == 33:
            f.write( "Texture2DArray<float4> feedbackSamplerInput2DArray;\n")
            f.write( "\n")

        f.write( "[shader(\"fragment\")]\n")
        f.write( "void fragMain()\n")
        f.write( "{\n")

        if generatePositiveTest:
            labelName = "fragMain"
            if "positive-label" in backend:
                labelName = backend["positive-label"]

            f.write( "// POSITIVE-NOT: {{(error|warning).*}}:\n")
            f.write( "// POSITIVE: result code = 0\n")
            f.write( "// POSITIVE-NOT: {{(error|warning).*}}:\n")
            f.write(f"// POSITIVE-LABEL: {labelName}\n")
            f.write( "// POSITIVE-NOT: {{(error|warning).*}}:\n")
            f.write( "\n")

        if generateNegativeTest:
            f.write( "// Expect either a compilation error or warning 41012 for upgraded profile to support functionality\n")
            f.write( "// NEGATIVE: {{error [[:digit:]]+|warning 41012}}:\n")
            f.write( "\n")

        f.write( "    int ret = 0;\n")
        emitTestOp(f, testInfo.testOp);
        f.write( "    outputBuffer[0] = 0x12345 + ret;\n")
        f.write( "}\n")
        if generatePositiveComputeTest:
            f.write( "\n")
            f.write( "[numthreads(4, 1, 1)]\n")
            f.write( "void computeMain()\n")
            f.write( "{\n")
            f.write( "    int ret = 0;\n")
            emitTestOp(f, testInfo.testOp);
            f.write( "    outputBuffer[0] = 0x12345 + ret;\n")
            f.write( "}\n")
            f.write( "// POSITIVE_RESULT: 12345\n")

def generateTests(backendTestTuple):
    backend = backendTestTuple[0]
    tests = backendTestTuple[1]

    print(f"Generating tests for backend {backend['name']}")

    for testType, testInfo in tests.items():
        filepath = f"gen-types-{backend['name']}-{testType.split('<')[0].lower()}.slang"
        print(f"- {filepath}")

        generateSingleTest(filepath, backend, testType, testInfo)


def main():
    if len(sys.argv) != 1:
        print("Generates texture types cabilities tests for all backends")
        print("")
        print("The pattern for generated tests is: gen-types-<backend>-<texture-type>.slang")
        sys.exit(1)

    generateTests(getWgslTests())
    generateTests(getMetalTests())
    generateTests(getGlslTests('-vk', 'glsl', '-emit-spirv-via-glsl'))

if __name__ == "__main__":
    main()

