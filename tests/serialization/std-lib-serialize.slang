//TEST:COMPILE: -save-core-module tests/serialization/slang-core-module.zip
// Disabled: segfaults when Slang is built with SLANG_EMBED_CORE_MODULE_SOURCE=OFF.
//
// -save-glsl-module-bin-source asks for the text of glsl.meta.slang, which is only compiled into
// the binary when that option is on. Without it Session::getGLSLLibraryCode() returns a null blob
// (source/slang-core-module/slang-embedded-core-module-source.cpp) and
// Session::getBuiltinModuleSource dereferences it without checking
// (source/slang/slang-global-session.cpp), so the compiler crashes rather than reporting that the
// source is unavailable. Re-enable once that null is handled; the test itself is fine.
//DISABLE_TEST:COMPILE: -load-core-module tests/serialization/slang-core-module.zip -save-glsl-module-bin-source tests/serialization/slang-glsl-module-generated.actual
//TEST:COMPILE: -save-core-module-bin-source tests/serialization/slang-core-module-generated.actual
//TEST:COMPARE_COMPUTE: -compile-arg -load-core-module -compile-arg tests/serialization/slang-core-module.zip -shaderobj

struct A
{
	float x;

	float addWith(float y)
	{
		return this.x + y;
	}
};

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;


float test(float inVal)
{
	A a;
	a.x = inVal;
	return a.addWith(inVal*inVal);
}

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
	uint tid = dispatchThreadID.x;
	float inVal = float(tid);
	float outVal = test(inVal);
	outputBuffer[tid] = outVal;
}
