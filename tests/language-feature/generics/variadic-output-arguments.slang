// TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
// TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type
// TEST:SIMPLE:-target dxil -entry computeMain -stage compute -g3 -O0

void writePack<each T>(Tuple<expand each T> source, out expand each T values)
{
    expand((each values) = (each source));
}

void forwardWrite<each T>(Tuple<expand each T> source, out expand each T values)
{
    writePack(source, expand each values);
}

void exchangePack<each T>(inout Tuple<expand each T> source, inout expand each T values)
{
    let previous = makeTuple(values);
    expand((each values) = (each source));
    source = previous;
}

void forwardExchange<each T>(inout Tuple<expand each T> source, inout expand each T values)
{
    exchangePack(source, expand each values);
}

void writeWholePack<each T>(Tuple<expand each T> source, __ref expand each T values)
{
    values = expand each source;
}

void forwardRef<each T>(Tuple<expand each T> source, __ref expand each T values)
{
    writeWholePack(source, expand each values);
}

struct PropertyStorage
{
    float data;
    property float value
    {
        get
        {
            return data;
        }
        set
        {
            data = newValue;
        }
    }
}

// TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float>
    outputBuffer;

[numthreads(1, 1, 1)] void computeMain()
{
    Tuple<uint, float> values;
    forwardWrite(makeTuple(uint(7), 3.0), expand each values);
    outputBuffer[0] = float(values._0); // CHECK: 7.0
    outputBuffer[1] = values._1;        // CHECK: 3.0

    var replacement = makeTuple(uint(11), 5.0);
    forwardExchange(replacement, expand each values);
    outputBuffer[2] = float(values._0);      // CHECK: 11.0
    outputBuffer[3] = values._1;             // CHECK: 5.0
    outputBuffer[4] = float(replacement._0); // CHECK: 7.0
    outputBuffer[5] = replacement._1;        // CHECK: 3.0

    // Concrete argument packs retain the usual scalar property/swizzle copy-back path.
    PropertyStorage propertyStorage = {0.0};
    float4 vector = float4(0.0);
    writePack(makeTuple(13.0, float2(17.0, 19.0)), propertyStorage.value, vector.xz);
    outputBuffer[6] = propertyStorage.data; // CHECK: 13.0
    outputBuffer[7] = vector.z;             // CHECK: 19.0

    forwardRef(makeTuple(uint(23), 29.0), expand each values);
    outputBuffer[8] = float(values._0); // CHECK: 23.0
    outputBuffer[9] = values._1;        // CHECK: 29.0

    Tuple<> empty;
    forwardWrite(makeTuple(), expand each empty);
    forwardExchange(empty, expand each empty);
}
