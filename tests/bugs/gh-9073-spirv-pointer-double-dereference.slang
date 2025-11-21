//TEST:SIMPLE(filecheck=POSITIVE): -entry main -stage closesthit -target spirv

// This is a minimal repro for issue 9073

struct Payload
{
    uint volumeStackIndex;
    int volumeStack[2];

    int GetVolume()
    {
        return volumeStack[volumeStackIndex];
    }
}

RWStructuredBuffer<int> outputBuffer;

[shader("closesthit")]
void main(inout Payload payload)
{
// POSITIVE-NOT: {{(error|warning).*}}:
// POSITIVE: result code = 0
// POSITIVE-NOT: {{(error|warning).*}}:

    outputBuffer[0] = payload.GetVolume();
}
