//TEST:SIMPLE(filecheck=CPP): -O0 -target cpp -entry computeMain -stage compute -validate-ir
//DIAGNOSTIC_TEST:SIMPLE(diag=SPIRV): -target spirv-asm -entry computeMain -stage compute -validate-ir

//SPIRV: opaque type in local variable is not supported for Khronos targets
//SPIRV: a resource or other opaque-typed value ('Texture2D') cannot be placed in a function-local variable for Khronos targets (SPIR-V/GLSL) or WGSL

// The C++ run checks that the entry point materializes both initializer elements, passes the
// complete array to the helper, copies the helper's update back, and reads both elements. The
// SPIR-V run separately checks the target restriction on that remaining array storage.

//CPP: static void [[INITIALIZE:_S[0-9]+]](
//CPP-SAME: FixedArray<{{.*}}, 2> * [[INITIALIZED:_S[0-9]+]])
//CPP: Texture2D<{{.*}}> [[SECOND:_S[0-9]+]] = {{.*}}textures_{{[0-9]+}}[int(1)];
//CPP: (*[[INITIALIZED]])[int(0)] = {{.*}}textures_{{[0-9]+}}[int(0)];
//CPP: (*[[INITIALIZED]])[int(1)] = [[SECOND]];
//CPP-LABEL: static void updateCachedTexture_
//CPP: FixedArray<{{.*}}, 2> [[UPDATED:cachedTextures_[0-9]+]] = *[[PARAM:cachedTextures_[0-9]+]];
//CPP: [[UPDATED]][int(1)] = {{.*}}textures_{{[0-9]+}}[int(2)];
//CPP: *[[PARAM]] = [[UPDATED]];
//CPP-LABEL: void _computeMain(
//CPP: FixedArray<{{.*}}, 2> [[INITIAL_VALUE:_S[0-9]+]];
//CPP: [[INITIALIZE]]({{.*}}, &[[INITIAL_VALUE]]);
//CPP: FixedArray<{{.*}}, 2> [[CACHED:cachedTextures_[0-9]+]] = [[INITIAL_VALUE]];
//CPP: updateCachedTexture_{{[0-9]+}}(&[[CACHED]],
//CPP: [[CACHED]][int(0)].Load
//CPP: [[CACHED]][int(1)].Load

Texture2D<float4> textures[3];
RWStructuredBuffer<float4> output;
static Texture2D<float4> cachedTextures[2] = {textures[0], textures[1]};

[noinline]
void updateCachedTexture()
{
    cachedTextures[1] = textures[2];
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    updateCachedTexture();
    output[0] = cachedTextures[0].Load(int3(0, 0, 0)) +
                cachedTextures[1].Load(int3(0, 0, 0));
}
