"""Versioned optimization-sensitive kernels and independently lowered references.

Each case fixes program complexity. Element count only controls parallel work.
References share the mathematical specification, never Slang-generated source.
"""

import re

VERSION = 1
CASES = {f"sensitive_{family}_{n}": (family, n)
         for family, sizes in (("registers", (16, 48)), ("loops", (8, 32)),
                               ("material", (8, 32))) for n in sizes}


def algorithm(case, slang=False):
    family, n = CASES[case]
    if family == "registers":
        # All values survive initialization and are accessed through runtime indices.
        # This deliberately exercises local-array lowering and spill thresholds.
        return "", f"""
    float values[{n}];
    for (uint j = 0; j < {n}; ++j)
        values[j] = inputData[(i + j * 17) % count] + float(j) * 0.001;
    float x = 0.0;
    for (uint k = 0; k < {n * 2}; ++k)
    {{
        uint a = (i + k * 7) % {n};
        uint b = (a + 3) % {n};
        values[a] = values[a] * 0.91 + values[b] * 0.07;
        x += values[a];
    }}
    outputData[i] = x / float({n * 2});
"""
    if family == "loops":
        return "", f"""
    float x = inputData[i];
    uint steps = 4 + (i % 5);
    for (uint j = 0; j < {n}; ++j)
        for (uint k = 0; k < steps; ++k)
        {{
            float y = inputData[(i + j * 13 + k * 7) % count];
            x = x * 0.97 + y * 0.02 + float(k) * 0.0001;
        }}
    outputData[i] = x;
"""
    declarations = ""
    if slang:
        declarations = """
interface Transform { float apply(float x, float y); }
struct Diffuse : Transform
{
    float apply(float x, float y) { return x * 0.91 + y * 0.07; }
}
struct Glossy : Transform
{
    float apply(float x, float y) { return x * 0.83 + y * y * 0.13; }
}
struct Layer<T : Transform> : Transform
{
    T inner;
    float apply(float x, float y) { return inner.apply(x, y) * 0.98 + 0.001; }
}
float evaluate<T : Transform>(T t, float x, float y) { return t.apply(x, y); }
"""
        select = """
        if (((i + j) & 3) == 0)
            x = evaluate(Layer<Layer<Glossy> >(), x, y);
        else
            x = evaluate(Layer<Layer<Diffuse> >(), x, y);
"""
    else:
        select = """
        if (((i + j) & 3) == 0)
            x = x * 0.83 + y * y * 0.13;
        else
            x = x * 0.91 + y * 0.07;
        x = (x * 0.98 + 0.001) * 0.98 + 0.001;
"""
    return declarations, f"""
    float x = inputData[i];
    for (uint j = 0; j < {n}; ++j)
    {{
        float y = inputData[(i + j * 17) % count];
{select}
    }}
    outputData[i] = x;
"""


def generate(case, language="slang", variant="equivalent"):
    if variant not in ("equivalent", "unroll", "unroll-affine") or (language == "slang" and variant != "equivalent"):
        raise ValueError("unsupported reference variant")
    declarations, body = algorithm(case, slang=language == "slang")
    if variant == "unroll-affine":
        family, n = CASES[case]
        if family != "material":
            raise ValueError("affine tuning applies only to material evaluation")
        # Fold the two affine layers into each material. This reassociates FP32
        # arithmetic, so it must pass the same independent CPU error tolerance.
        body = f"""
    float x = inputData[i];
    for (uint j = 0; j < {n}; ++j)
    {{
        float y = inputData[(i + j * 17) % count];
        x = ((i + j) & 3) == 0
            ? x * 0.797132 + y * y * 0.124852 + 0.00198
            : x * 0.873964 + y * 0.067228 + 0.00198;
    }}
    outputData[i] = x;
"""
    if variant != "equivalent":
        directive = {"hlsl": "[unroll]", "glsl": "[[unroll]]", "cuda": "#pragma unroll"}[language]
        body = re.sub(r"(    )(for \(uint \w+ = 0; \w+ < \d+;)",
                      lambda m: m[1] + directive + "\n" + m[1] + m[2], body)
    if language in ("slang", "hlsl"):
        bindings = """
[[vk::binding(0, 0)]] StructuredBuffer<float> inputData : register(t0);
[[vk::binding(1, 0)]] RWStructuredBuffer<float> outputData : register(u0);
[[vk::binding(2, 0)]] cbuffer Params : register(b0)
{
    uint count;
    uint dispatchWidth;
};
"""
        return bindings + declarations + ('[shader("compute")]\n' if language == "slang" else "") + """
[numthreads(64, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint i = tid.x + tid.y * dispatchWidth;
    if (i >= count) return;
""" + body + "}\n"
    if language == "glsl":
        return "#version 460\n" + ("#extension GL_EXT_control_flow_attributes : require\n" if variant != "equivalent" else "") + """layout(local_size_x=64) in;
layout(set=0, binding=0, std430) readonly buffer Input { float inputData[]; };
layout(set=0, binding=1, std430) writeonly buffer Output { float outputData[]; };
layout(set=0, binding=2, std140) uniform Params { uint count; uint dispatchWidth; };
void main()
{
    uint i = gl_GlobalInvocationID.x + gl_GlobalInvocationID.y * dispatchWidth;
    if (i >= count) return;
""" + body + "}\n"
    if language == "cuda":
        body = re.sub(r"\b(\d+\.\d+)\b", r"\1f", body)
        if variant != "equivalent":
            body = re.sub(r"inputData\[([^\]]+)\]", r"__ldg(inputData + (\1))", body)
        # RHI's CUDA ABI uses a constant global parameter block. The buffer
        # descriptors contain a device pointer and a 64-bit element count.
        return """typedef unsigned int uint;
struct Buffer { float* data; unsigned long long size; };
struct Params { uint count; uint dispatchWidth; };
struct Globals { Buffer inputData; Buffer outputData; Params* params; };
extern "C" __constant__ Globals SLANG_globalParams;
extern "C" __global__ void computeMain()
{
    const float* inputData = SLANG_globalParams.inputData.data;
    float* outputData = SLANG_globalParams.outputData.data;
    uint count = SLANG_globalParams.params->count;
    uint dispatchWidth = SLANG_globalParams.params->dispatchWidth;
    uint i = blockIdx.x * blockDim.x + threadIdx.x + blockIdx.y * dispatchWidth;
    if (i >= count) return;
""" + body + "}\n"
    raise ValueError(language)
