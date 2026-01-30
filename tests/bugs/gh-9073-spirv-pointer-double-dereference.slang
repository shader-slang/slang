//TEST:SIMPLE(filecheck=POSITIVE): -entry main -stage closesthit -target spirv

// This is a minimal repro for issue 9073

struct Payload
{
    uint volumeStackIndex;
    int volumeStack[2];

    int GetVolume()
    {
        // This function exercises the following transformation in
        // transformParamsToConstRef():
        //
        // IRGetElement(
        //   IRFieldExtract(...),
        //   IRFieldExtract(...))
        //
        //      =>
        //
        // IRGetElementPtr(
        //   IRFieldAddress(...),
        //   IRLoad(IRFieldAddress(...)))
        //
        // That is, that IRLoad() is correctly injected to
        // IRGetElementPtr() index when IRFieldExtract() is
        // transformed to IRFieldAddress()

        return volumeStack[volumeStackIndex];
    }

    int GetVolume2()
    {
        // This function exercises the following transformation in
        // transformParamsToConstRef():
        //
        // IRGetElement(
        //   IRFieldExtract(...),
        //   IRGetElement(...))
        //
        //      =>
        //
        // IRGetElementPtr(
        //   IRFieldAddress(...),
        //   IRLoad(IRGetElementPtr(...)))
        //
        // That is, that IRLoad() is correctly injected to
        // IRGetElementPtr() index when IRGetElement() is
        // transformed to IRGetElementPtr()

        return volumeStack[volumeStack[volumeStackIndex]];
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

    outputBuffer[1] = payload.GetVolume2();
}
