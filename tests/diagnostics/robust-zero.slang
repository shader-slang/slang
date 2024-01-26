//TEST:SIMPLE(filecheck=CHECK): -target hlsl -stage compute -entry main
RWStructuredBuffer<float> output;

void main()
{
    // 1.7e-37 means 1.7 * (1/10)^37.   1234567890123456789012345678901234567
    // JKWAKCHECK: output{{.*}}[0{{.*}}] = 0.00000000000000000000000000000000000017;
    // CHECK: output{{.*}}[0{{.*}}] = 1.7e-37
    output[0] = 1.7e-37;
}
