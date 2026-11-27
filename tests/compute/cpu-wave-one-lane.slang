// On the CPU targets every thread is a wave of one lane. With one thread per group, every target
// sees a single active lane, so the expected values hold for the GPU lanes slang-test synthesizes
// from this test too.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-cpu -compute -shaderobj -output-using-type
//TEST:SIMPLE(filecheck=CPP):-target cpp -stage compute -entry computeMain

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

// The C++ targets define a wave as exactly one lane; a GPU wave has at least one.
bool hasExpectedLaneCount()
{
    __target_switch
    {
    case cpp:
        return WaveGetLaneCount() == 1;
    default:
        return WaveGetLaneCount() >= 1;
    }
}

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    int x = int(tid.x) + 5;
    float f = float(x) * 0.5f;
    int3 v = int3(x, x + 1, x + 2);

    outputBuffer[0] = hasExpectedLaneCount() ? 1 : 0;
    outputBuffer[1] = int(WaveGetLaneIndex());
    outputBuffer[2] = WaveIsFirstLane() ? 1 : 0;
    outputBuffer[3] = WaveActiveSum(x);
    outputBuffer[4] = WaveActiveProduct(x);
    outputBuffer[5] = WaveActiveMin(x);
    outputBuffer[6] = WaveActiveMax(x);
    outputBuffer[7] = WaveActiveBitOr(x);
    outputBuffer[8] = int(WaveActiveSum(f) * 2.0f);
    outputBuffer[9] = WaveActiveSum(v).z;
    outputBuffer[10] = WavePrefixSum(x);
    outputBuffer[11] = WavePrefixProduct(x);
    outputBuffer[12] = int(WavePrefixCountBits(true));
    outputBuffer[13] = int(WaveActiveCountBits(true));
    outputBuffer[14] = int(WaveActiveBallot(true).x);
    outputBuffer[15] = WaveActiveAllTrue(x == 5) ? 1 : 0;
    outputBuffer[16] = WaveActiveAnyTrue(x == 6) ? 1 : 0;
    outputBuffer[17] = WaveActiveAllEqual(v) ? 1 : 0;
    outputBuffer[18] = WaveReadLaneFirst(x);
    outputBuffer[19] = WaveReadLaneAt(v, 0).y;
    outputBuffer[20] = WavePrefixSum(v).x;
    outputBuffer[21] = WaveActiveBitAnd(x) + WaveActiveBitXor(x);
    outputBuffer[22] = WaveActiveMin(v).y + WaveActiveMax(v).z;
}

// BUF: 1
// BUF-NEXT: 0
// BUF-NEXT: 1
// BUF-NEXT: 5
// BUF-NEXT: 5
// BUF-NEXT: 5
// BUF-NEXT: 5
// BUF-NEXT: 5
// BUF-NEXT: 5
// BUF-NEXT: 7
// BUF-NEXT: 0
// BUF-NEXT: 1
// BUF-NEXT: 0
// BUF-NEXT: 1
// BUF-NEXT: 1
// BUF-NEXT: 1
// BUF-NEXT: 0
// BUF-NEXT: 1
// BUF-NEXT: 5
// BUF-NEXT: 6
// BUF-NEXT: 0
// BUF-NEXT: 10
// BUF-NEXT: 13

// CPP: computeMain
