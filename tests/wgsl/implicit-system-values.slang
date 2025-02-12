//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-wgpu -compute -entry computeMain

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

interface IInterface
{
    uint getLaneIndex();
    uint getLaneIndex(uint base);
};

struct Impl1 : IInterface
{
    uint getLaneIndex()
    {
        return WaveGetLaneIndex();
    }

    uint getLaneIndex(uint base) 
    {
        return base + WaveGetLaneIndex();
    }
};

struct Impl2 : IInterface
{
    uint getLaneIndex()
    {
        return 100;
    }

    uint getLaneIndex(uint base) 
    {
        return base - 1;
    }
};

struct Impl3 : IInterface
{
    uint getLaneIndex(uint base = 1)
    {
        return base + WaveGetLaneIndex() + 1;
    }
};

struct Impl4 : IInterface
{
    uint getLaneIndex()
    {
        return WaveGetLaneIndex() + 2;
    }

    uint getLaneIndex(uint base) 
    {
        return base * 2;
    }
};



uint getLaneIndexGeneric<T>(T interface, uint base = 3) where T : IInterface
{
    return interface.getLaneIndex(base);
}

struct MyStruct<T> where T : IInterface
{
    T interface;
    uint getLaneIndex(uint base = 4)
    {
        return interface.getLaneIndex(base);
    }
}

[numthreads(1,1,1)]
void computeMain()
{
    Impl1 impl1;
    Impl2 impl2;
    Impl3 impl3;
    Impl4 impl4;

    MyStruct<Impl1> s1;
    MyStruct<Impl2> s2;
    MyStruct<Impl3> s3;
    MyStruct<Impl4> s4;

    // BUF: 1
    outputBuffer[0] = uint(
        (0 == WaveGetLaneIndex())
        
        // Interface implementations.
        && (0 == impl1.getLaneIndex())
        && (100 == impl2.getLaneIndex())
        && (2 == impl3.getLaneIndex())
        && (2 == impl4.getLaneIndex())
        && (2 == impl1.getLaneIndex(2))
        && (1 == impl2.getLaneIndex(2))
        && (3 == impl3.getLaneIndex(2))
        && (12 == impl4.getLaneIndex(6))

        // Interface as function generic parameter.
        && (3 == getLaneIndexGeneric(impl1))
        && (2 == getLaneIndexGeneric(impl2))
        && (4 == getLaneIndexGeneric(impl3))
        && (6 == getLaneIndexGeneric(impl4))
        && (4 == getLaneIndexGeneric(impl1, 4))
        && (3 == getLaneIndexGeneric(impl2, 4))
        && (5 == getLaneIndexGeneric(impl3, 4))
        && (8 == getLaneIndexGeneric(impl4, 4))

        // Interface as struct generic member.
        && (5 == s1.getLaneIndex(5))
        && (4 == s2.getLaneIndex(5))
        && (6 == s3.getLaneIndex(5))
        && (10 == s4.getLaneIndex(5))
        && (4 == s1.getLaneIndex())
        && (3 == s2.getLaneIndex())
        && (5 == s3.getLaneIndex())
        && (8 == s4.getLaneIndex())
    );
}
