#!/usr/bin/python3

import sys


# Table of test input types with the appropriate Slang texture types
#
# Tests specifications for all these types must be specified by
# backends. See getMetalTests() for a simple example.
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
#
# Function getTestOp() returns the respective code to access a texture with the specified op.

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

# Test specifications for the WGSL backend
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

# Test specifications for Metal
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


# Determines the backend versions for the positive and the negative tests:
#
# Positive test uses the lowest version that supports the texture type.
#
# Negative test uses the highest version that does not supports the texture type.
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


def getTestOp(testOp):
    ret = ""
    if testOp == 1:
        ret += ("ret = int((texHandle.SampleLevel(samplerState, float(0), float(0))).x);")
    elif testOp == 2:
        ret += ("ret = int((texHandle.SampleLevel(samplerState, float2(0, 0), float(0))).x);")
    elif testOp == 3:
        ret += ("ret = int((texHandle.SampleLevel(samplerState, float3(0, 0, 0), float(0))).x);")
    elif testOp == 4:
        ret += ("ret = int((texHandle.SampleLevel(samplerState, float4(0, 0, 0, 0), float(0))).x);")
    elif testOp == 11:
        ret += ("ret = int((texHandle.Load(int(0))).x);")
    elif testOp == 12:
        ret += ("ret = int((texHandle.Load(int2(0, 0))).x);")
    elif testOp == 13:
        ret += ("ret = int((texHandle.Load(int3(0, 0, 0))).x);")
    elif testOp == 14:
        ret += ("ret = int((texHandle.Load(int4(0, 0, 0, 0))).x);")
    elif testOp == 15:
        ret += ("ret = int((texHandle.Load(int2(0, 0), 0)).x);")
    elif testOp == 16:
        ret += ("ret = int((texHandle.Load(int3(0, 0, 0), 0)).x);")
    elif testOp == 21:
        ret += ("texHandle.Store(int(0), float4(0, 0, 0, 0));\n")
        ret += ("    ret = 0;")
    elif testOp == 22:
        ret += ("texHandle.Store(int2(0, 0), float4(0, 0, 0, 0));\n")
        ret += ("    ret = 0;")
    elif testOp == 23:
        ret += ("texHandle.Store(int3(0, 0, 0), float4(0, 0, 0, 0));\n")
        ret += ("    ret = 0;")
    elif testOp == 32:
        ret += ("texHandle.WriteSamplerFeedbackLevel(feedbackSamplerInput2D, samplerState, float2(0, 0), float(0));\n")
        ret += ("    ret = 0;")
    elif testOp == 33:
        ret += ("texHandle.WriteSamplerFeedbackLevel(feedbackSamplerInput2DArray, samplerState, float3(0, 0, 0), float(0));\n")
        ret += ("    ret = 0;")
    elif testOp == 41:
        ret += ("ret = int((texHandle.SampleCmpLevelZero(samplerComparisonState, float(0), float(0))).x);")
    elif testOp == 42:
        ret += ("ret = int((texHandle.SampleCmpLevelZero(samplerComparisonState, float2(0, 0), float(0))).x);")
    elif testOp == 43:
        ret += ("ret = int((texHandle.SampleCmpLevelZero(samplerComparisonState, float3(0, 0, 0), float(0))).x);")
    elif testOp == 44:
        ret += ("ret = int((texHandle.SampleCmpLevelZero(samplerComparisonState, float4(0, 0, 0, 0), float(0))).x);")
    else:
        print(f"Undefined test op code: {testOp}")
        sys.exit(1)

    return ret


def generateSingleTest(filepath, backend, testType, testInfo):

    global testInputForTestType;

    # Determine the first supporting version and highest non-supporting version
    (positiveTestTarget, negativeTestTarget) = \
        getPositiveNegativeBackendTargets(backend, testInfo.minVersion)

    positiveTestTargetVersion = None
    if positiveTestTarget is not None:
        positiveTestTargetVersion = positiveTestTarget[0]

    negativeTestTargetVersion = None
    if negativeTestTarget is not None:
        negativeTestTargetVersion = negativeTestTarget[0]

    # Determine test targets
    positiveSimpleTestTarget = "None"
    positiveComputeTestTarget = "None"
    negativeSimpleTestTarget = "None"
    disablePositiveSimpleTestPrefix = ""
    disableComputeTestPrefix = ""
    disableNegativeSimpleTestPrefix = ""

    if positiveTestTarget is not None:
        positiveSimpleTestTarget = positiveTestTarget[1]
        positiveComputeTestTarget = positiveTestTarget[2]
    else:
        disablePositiveSimpleTestPrefix = "DISABLE_"
        disableComputeTestPrefix = "DISABLE_"

    if negativeTestTarget is not None:
        negativeSimpleTestTarget = negativeTestTarget[1]
    else:
        disableNegativeSimpleTestPrefix = "DISABLE_"

    additionalTestInput = ""
    # ops 0..9 and 30..39 require sampler state
    if (testInfo.testOp in range(0, 9)) or (testInfo.testOp in range(30, 39)):
        additionalTestInput += '''
//TEST_INPUT: Sampler(filteringMode=point):name samplerState
SamplerState samplerState;
'''

    # ops 40..49 require sampler comparison state (shadow/depth sampling)
    if testInfo.testOp in range(40, 49):
        additionalTestInput += '''
//TEST_INPUT: Sampler(depthCompare):name samplerComparisonState
SamplerComparisonState samplerComparisonState;
'''

    # ops 32 and 33 require also input texture for feedback sampling
    if testInfo.testOp == 32:
        additionalTestInput += '''
Texture2D<float4> feedbackSamplerInput2D;
'''

    if testInfo.testOp == 33:
        additionalTestInput += '''
Texture2DArray<float4> feedbackSamplerInput2DArray;
'''

    labelName = "fragMain"
    if "positive-label" in backend:
        labelName = backend["positive-label"]

    # determine if tests are disabled
    testDisabledComment = ""
    computeTestDisabledComment = ""
    negativeTestDisabledComment = ""

    if testInfo.disableForIssue is not None:
        # test completely disabled
        disablePositiveSimpleTestPrefix = "DISABLE_"
        disableNegativeSimpleTestPrefix = "DISABLE_"
        disableComputeTestPrefix = "DISABLE_"
        testDisabledComment = f"\n// Test disabled, see https://github.com/shader-slang/slang/issues/{testInfo.disableForIssue}"
    else:
        if testInfo.disableComputeForIssue is not None:
            # the compute test instance disabled
            disableComputeTestPrefix = "DISABLE_"
            testDisabledComment += f"\n// Compute test disabled, see https://github.com/shader-slang/slang/issues/{testInfo.disableComputeForIssue}"

        if disableNegativeSimpleTestPrefix == "" and testInfo.disableNegativeTestForIssue is not None:
            # the negative test instance disabled
            disableNegativeSimpleTestPrefix = "DISABLE_"
            testDisabledComment += f"\n// Negative test disabled, see https://github.com/shader-slang/slang/issues/{testInfo.disableNegativeTestForIssue}"

    testStr = f"""// THIS IS A GENERATED FILE. DO NOT EDIT!
// Instead, edit generate-types-tests.py and then regenerate this test
// by running: ./generate-types-tests.py
//
// Texture types capability test: {backend['name']} / {testType}
// - Type supported since target version:  {testInfo.minVersion}
// - Target version for positive test:     {positiveTestTargetVersion}
// - Target version for negative test:     {negativeTestTargetVersion}
//
{testDisabledComment}
//{disablePositiveSimpleTestPrefix}TEST:SIMPLE(filecheck=POSITIVE): -entry fragMain -stage fragment -target {positiveSimpleTestTarget}
//{disableComputeTestPrefix}TEST(compute):COMPARE_COMPUTE(filecheck-buffer=POSITIVE_RESULT): {positiveComputeTestTarget}
//{disableNegativeSimpleTestPrefix}TEST:SIMPLE(filecheck=NEGATIVE): -entry fragMain -stage fragment -target {negativeSimpleTestTarget}

//TEST_INPUT: ubuffer(data=[0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

//TEST_INPUT: {testInputForTestType[testType]}:name texHandle
{testType} texHandle;
{additionalTestInput}
[shader(\"fragment\")]
void fragMain()
{{
// POSITIVE-NOT: {{{{(error|warning).*}}}}:
// POSITIVE: result code = 0
// POSITIVE-NOT: {{{{(error|warning).*}}}}:
// POSITIVE-LABEL: {labelName}
// POSITIVE-NOT: {{{{(error|warning).*}}}}:

// Expect either a compilation error or warning 41012 for upgraded profile to support functionality
// NEGATIVE: {{{{error [[:digit:]]+|warning 41012}}}}:

    int ret = 0;
    {getTestOp(testInfo.testOp)}
    outputBuffer[0] = 0x12345 + ret;
}}

[numthreads(4, 1, 1)]
void computeMain()
{{
    int ret = 0;
    {getTestOp(testInfo.testOp)}
    outputBuffer[0] = 0x12345 + ret;
}}
// POSITIVE_RESULT: 12345
"""

    # write the generated test to file
    with open(filepath, "w") as f:
        f.write(testStr)


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

