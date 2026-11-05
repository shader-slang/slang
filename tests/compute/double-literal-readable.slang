// SPDX-FileCopyrightText: The Khronos Group, Inc.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

//TEST:SIMPLE(filecheck=HLSL): -target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=GLSL): -target glsl -entry computeMain -stage compute

RWStructuredBuffer<double> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = 0.5l;
    outputBuffer[1] = 0.25l;
    outputBuffer[2] = 0.125l;
    outputBuffer[3] = 0.0625l;
    outputBuffer[4] = 0.00390625l;
    outputBuffer[5] = 1.0l;
    outputBuffer[6] = 10000.0l;
    outputBuffer[7] = 0.0l;
    outputBuffer[8] = -0.0l;
    outputBuffer[9] = 0.0001l;
}

// Simple fractions should stay readable. Whole numbers must keep a decimal point,
// and trimming fractional zeros must not remove significant integer zeros.
// HLSL: = 0.5;
// HLSL: = 0.25;
// HLSL: = 0.125;
// HLSL: = 0.0625;
// HLSL: = 0.00390625;
// HLSL: = 1.0;
// HLSL: = 10000.0;
// HLSL: = 0.0;
// HLSL: = -0.0;
// HLSL: = 0.0001;
// GLSL: = 0.5LF;
// GLSL: = 0.25LF;
// GLSL: = 0.125LF;
// GLSL: = 0.0625LF;
// GLSL: = 0.00390625LF;
// GLSL: = 1.0LF;
// GLSL: = 10000.0LF;
// GLSL: = 0.0LF;
// GLSL: = -0.0LF;
// GLSL: = 0.0001LF;
