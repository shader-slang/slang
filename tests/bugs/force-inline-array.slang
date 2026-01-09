//TEST:COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -output-using-type -vk -emit-spirv-directly -xslang -g0

struct Coord
{
    uint coords[8];
    __subscript(int index) -> uint
    {
        get { return coords[index]; }
        set { coords[index] = newValue; }
    }

    __init(int c0, int c1)
    {
        coords[0] = c0;
        coords[1] = c1;
    }
}

struct BufferSink
{
    int* data;
    int rank;
    int strides[8];

    void store(Coord coord, int value)
    {
        uint offset = 0;
        [ForceUnroll]
        for (int i = 0; i < 3; i++)
        {
            if (i >= rank)
                break;
            offset += coord[i] * strides[i];
        }
        data[offset] = value;
    }
};

//TEST_INPUT: set output = out ubuffer(data=[0 0 0 0], stride=4)
uniform int* output;

[numthreads(1,1,1)]
void computeMain()
{
    BufferSink sink;
    sink.data = output;
    sink.rank= 2;
    sink.strides[0] = 2;
    sink.strides[1] = 1;
    sink.store(Coord(0, 0), 1);
    sink.store(Coord(0, 1), 2);
    sink.store(Coord(1, 0), 3);
    sink.store(Coord(1, 1), 4);
}
// CHECK: 1
// CHECK: 2
// CHECK: 3
// CHECK: 4