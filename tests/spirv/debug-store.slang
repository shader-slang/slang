//TEST:SIMPLE(filecheck=CHECK): -target spirv -g2 -O0

// Test that writes to stand-in proxy variables for an `in` parameter are
// properly updating the corresponding debug variable for the parameter when
// debug info is enabled.

//CHECK-COUNT-4: OpStore %_dbgvar_func_value

cbuffer CBuffer
{
	float x;
} cb;

float Func(float func_value)
{
	func_value = func_value > 10.0f ? func_value + 1.0f : func_value - 1.0f;
	func_value = sqrt(func_value);
	func_value = sin(func_value);

	return func_value;
}

RWBuffer<float> outbuf;

[numthreads(1, 1, 1)]
void main()
{
	float value = cb.x;

	value = Func(cb.x);

	outbuf[0] = value;
}
