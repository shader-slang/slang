//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry computeMain -stage compute

// Regression test for shader-slang/slang#11075: on targets with a native vector `min`/`max`, a
// generic `IFloat`/`IInteger` `min`/`max` on a vector lowers to that vector operation, as the
// concrete vector overload does, not to a loop over scalars.

T minFloat<T : IFloat>(T a, T b)
{
    return min(a, b);
}
T maxFloat<T : IFloat>(T a, T b)
{
    return max(a, b);
}
T minInteger<T : IInteger>(T a, T b)
{
    return min(a, b);
}
T maxInteger<T : IInteger>(T a, T b)
{
    return max(a, b);
}

RWStructuredBuffer<float3> floatBuffer;
RWStructuredBuffer<int3> intBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    floatBuffer[2] = minFloat(floatBuffer[0], floatBuffer[1]);
    floatBuffer[3] = maxFloat(floatBuffer[0], floatBuffer[1]);
    intBuffer[2] = minInteger(intBuffer[0], intBuffer[1]);
    intBuffer[3] = maxInteger(intBuffer[0], intBuffer[1]);
}

// Each specialized function returns the target's `min`/`max` applied to its vector parameters.
// Floating-point GLSL adds boolean selections around that operation to select numeric operands
// when the other component is NaN. A scalar loop would index the parameters instead.

// HLSL: int3 maxInteger_{{[0-9]+}}(int3 a_{{[0-9]+}}, int3 b_{{[0-9]+}})
// HLSL-NOT: }
// HLSL: return max(a_{{[0-9]+}}, b_{{[0-9]+}});
// HLSL: int3 minInteger_{{[0-9]+}}(int3 a_{{[0-9]+}}, int3 b_{{[0-9]+}})
// HLSL-NOT: }
// HLSL: return min(a_{{[0-9]+}}, b_{{[0-9]+}});
// HLSL: float3 maxFloat_{{[0-9]+}}(float3 a_{{[0-9]+}}, float3 b_{{[0-9]+}})
// HLSL-NOT: }
// HLSL: return max(a_{{[0-9]+}}, b_{{[0-9]+}});
// HLSL: float3 minFloat_{{[0-9]+}}(float3 a_{{[0-9]+}}, float3 b_{{[0-9]+}})
// HLSL-NOT: }
// HLSL: return min(a_{{[0-9]+}}, b_{{[0-9]+}});

// GLSL: ivec3 maxInteger_{{[0-9]+}}(ivec3 a_{{[0-9]+}}, ivec3 b_{{[0-9]+}})
// GLSL-NOT: }
// GLSL: return max(a_{{[0-9]+}}, b_{{[0-9]+}});
// GLSL: ivec3 minInteger_{{[0-9]+}}(ivec3 a_{{[0-9]+}}, ivec3 b_{{[0-9]+}})
// GLSL-NOT: }
// GLSL: return min(a_{{[0-9]+}}, b_{{[0-9]+}});
// GLSL: vec3 maxFloat_{{[0-9]+}}(vec3 a_{{[0-9]+}}, vec3 b_{{[0-9]+}})
// GLSL-NOT: }
// GLSL: return (mix(mix(max((a_{{[0-9]+}}), (b_{{[0-9]+}})), (a_{{[0-9]+}}), isnan((b_{{[0-9]+}}))), (b_{{[0-9]+}}), isnan((a_{{[0-9]+}}))));
// GLSL: vec3 minFloat_{{[0-9]+}}(vec3 a_{{[0-9]+}}, vec3 b_{{[0-9]+}})
// GLSL-NOT: }
// GLSL: return (mix(mix(min((a_{{[0-9]+}}), (b_{{[0-9]+}})), (a_{{[0-9]+}}), isnan((b_{{[0-9]+}}))), (b_{{[0-9]+}}), isnan((a_{{[0-9]+}}))));

// METAL: int3 maxInteger_{{[0-9]+}}(int3 a_{{[0-9]+}}, int3 b_{{[0-9]+}})
// METAL-NOT: }
// METAL: return max(a_{{[0-9]+}}, b_{{[0-9]+}});
// METAL: int3 minInteger_{{[0-9]+}}(int3 a_{{[0-9]+}}, int3 b_{{[0-9]+}})
// METAL-NOT: }
// METAL: return min(a_{{[0-9]+}}, b_{{[0-9]+}});
// METAL: float3 maxFloat_{{[0-9]+}}(float3 a_{{[0-9]+}}, float3 b_{{[0-9]+}})
// METAL-NOT: }
// METAL: return max(a_{{[0-9]+}}, b_{{[0-9]+}});
// METAL: float3 minFloat_{{[0-9]+}}(float3 a_{{[0-9]+}}, float3 b_{{[0-9]+}})
// METAL-NOT: }
// METAL: return min(a_{{[0-9]+}}, b_{{[0-9]+}});

// WGSL: fn maxInteger_{{[0-9]+}}( a_{{[0-9]+}} : vec3<i32>, b_{{[0-9]+}} : vec3<i32>) -> vec3<i32>
// WGSL-NOT: }
// WGSL: return max(a_{{[0-9]+}}, b_{{[0-9]+}});
// WGSL: fn minInteger_{{[0-9]+}}( a_{{[0-9]+}} : vec3<i32>, b_{{[0-9]+}} : vec3<i32>) -> vec3<i32>
// WGSL-NOT: }
// WGSL: return min(a_{{[0-9]+}}, b_{{[0-9]+}});
// WGSL: fn maxFloat_{{[0-9]+}}( a_{{[0-9]+}} : vec3<f32>, b_{{[0-9]+}} : vec3<f32>) -> vec3<f32>
// WGSL-NOT: }
// WGSL: return max(a_{{[0-9]+}}, b_{{[0-9]+}});
// WGSL: fn minFloat_{{[0-9]+}}( a_{{[0-9]+}} : vec3<f32>, b_{{[0-9]+}} : vec3<f32>) -> vec3<f32>
// WGSL-NOT: }
// WGSL: return min(a_{{[0-9]+}}, b_{{[0-9]+}});

// In SPIR-V the extended instruction's result type is the vector type.
// SPIRV-DAG: OpExtInst %v3float %{{[0-9a-zA-Z_]+}} NMin
// SPIRV-DAG: OpExtInst %v3float %{{[0-9a-zA-Z_]+}} NMax
// SPIRV-DAG: OpExtInst %v3int %{{[0-9a-zA-Z_]+}} SMin
// SPIRV-DAG: OpExtInst %v3int %{{[0-9a-zA-Z_]+}} SMax
