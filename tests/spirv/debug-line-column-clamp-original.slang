RWStructuredBuffer<float> outputBuffer;

float helper(float value)
{
	{
		return value;
	}
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
	outputBuffer[0] = helper(1.0);
}
