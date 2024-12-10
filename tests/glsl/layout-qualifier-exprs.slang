//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry computeMain -allow-glsl

#version 450

layout(local_size_x = 4, local_size_y = 1, local_size_z = 1) in;

static const uint BINDING_BASE = 3;
static const uint BINDING_STRIDE = 2;

static const uint SET_BASE = 1;
static const uint SET_STRIDE = 4;

//TEST_INPUT:ubuffer(data=[2 2 2 2], stride=4):name=a
layout(set = SET_BASE, binding = BINDING_BASE * BINDING_STRIDE) buffer InputA {
    float a[];
};

//TEST_INPUT:ubuffer(data=[2 2 2 2], stride=4):name=b
layout(set = SET_BASE + SET_STRIDE, binding = BINDING_BASE * BINDING_STRIDE * 2) buffer InputB {
    float b[];
};

layout(set = SET_BASE + SET_STRIDE * 2, binding = BINDING_BASE * BINDING_STRIDE * 3) buffer Output {
    float result[];
};

void computeMain() {
    uint index = gl_GlobalInvocationID.x;

    result[index] = a[index] * b[index];

    // CHECK: OpEntryPoint
}

