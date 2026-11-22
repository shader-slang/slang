//TEST:SIMPLE_EX(filecheck=CHECK): -lang glsl tests/glsl/flat-in.slang -target spirv -stage fragment -entry main -emit-spirv-directly
//TEST:SIMPLE_EX(filecheck=CHECK): -lang glsl tests/glsl/flat-in.slang -target spirv -stage fragment -entry main

#version 310 es
precision highp float;
precision highp int;

layout(location = 0) out mediump vec4 dEQP_FragColor;

bool isOk (uint a, uint b)   { return (a == b); }

layout(location = 0) flat in uint out0;
layout(binding = 0, std140) uniform Reference
{
	uint out0;
} ref;

void main()
{
	bool RES = isOk(out0, ref.out0);
    dEQP_FragColor = vec4(RES, RES, RES, 1.0);
    // CHECK: OpEntryPoint
}

