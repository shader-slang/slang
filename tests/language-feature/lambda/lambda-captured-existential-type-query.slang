//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -output-using-type

// `sizeof` consumes the complete checked type and therefore needs the captured existential value
// from which `value.Value` was opened. Fixed-array `countof` instead consumes only the checked
// array element count.

interface IValueShape
{
    associatedtype Value;
}

struct IntValueShape : IValueShape
{
    typealias Value = int;
}

[__NonCopyableType]
interface INonCopyableValueShape
{
    associatedtype Value;
}

void checkUnevaluatedFixedArrayQueries(INonCopyableValueShape value)
{
    // The query does not evaluate `value`, so the lambda does not capture the non-copyable
    // interface value.
    let getCount = () => countof(value.Value[2]);
    getCount();
}

int getExistentialTypeQueryResults(IValueShape value)
{
    let getSize = () => sizeof(value.Value[1]);
    let getCount = () => countof(value.Value[2]);
    return getSize() + getCount();
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    IValueShape value = IntValueShape();
    outputBuffer[0] = getExistentialTypeQueryResults(value);
}

// CHECK: type: int32_t
// CHECK-NEXT: 6
