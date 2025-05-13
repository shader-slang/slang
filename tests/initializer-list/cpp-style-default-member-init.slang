//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUFFER):-shaderobj -vk -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUFFER):-shaderobj

struct NonCStyle
{
  int x = 3;
  float y = 4.0f;
  int z[2] = {5, 6};
  float2 w = {7.0f, 8.0f};

  __init(int x)
  {
    this.x = x;
  }

  __init(int x, float y)
  {
    this.x = x;
    this.y = y;
  }

  __init(int arr[2])
  {
    this.z[0] = arr[0];
    this.z[1] = arr[1];
  }

  __init(float2 vec)
  {
    this.w= vec;
  }
}

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0   0 0 0 0 0 0   0 0 0 0 0 0   0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;
void test()
{
  int idx = 0;
  NonCStyle s1 = NonCStyle(1);
  outputBuffer[idx++] = s1.x;
  // BUFFER: 1
  outputBuffer[idx++] = s1.y;
  // BUFFER: 4.0
  outputBuffer[idx++] = s1.z[0];
  // BUFFER: 5
  outputBuffer[idx++] = s1.z[1];
  // BUFFER: 6
  outputBuffer[idx++] = s1.w.x;
  // BUFFER: 7
  outputBuffer[idx++] = s1.w.y;
  // BUFFER: 8

  NonCStyle s2 = NonCStyle(1, 12.0);
  outputBuffer[idx++] = s2.x;
  // BUFFER: 1
  outputBuffer[idx++] = s2.y;
  // BUFFER: 12.0
  outputBuffer[idx++] = s2.z[0];
  // BUFFER: 5
  outputBuffer[idx++] = s2.z[1];
  // BUFFER: 6
  outputBuffer[idx++] = s2.w.x;
  // BUFFER: 7
  outputBuffer[idx++] = s2.w.y;
  // BUFFER: 8

  NonCStyle s3 = NonCStyle(Array<int, 2>(21u, 22u));
  outputBuffer[idx++] = s3.x;
  // BUFFER: 3
  outputBuffer[idx++] = s3.y;
  // BUFFER: 4.0
  outputBuffer[idx++] = s3.z[0];
  // BUFFER: 21
  outputBuffer[idx++] = s3.z[1];
  // BUFFER: 22
  outputBuffer[idx++] = s3.w.x;
  // BUFFER: 7
  outputBuffer[idx++] = s3.w.y;
  // BUFFER: 8

  NonCStyle s4 = NonCStyle(float2(31, 32));
  outputBuffer[idx++] = s4.x;
  // BUFFER: 3
  outputBuffer[idx++] = s4.y;
  // BUFFER: 4.0
  outputBuffer[idx++] = s4.z[0];
  // BUFFER: 5
  outputBuffer[idx++] = s4.z[1];
  // BUFFER: 6
  outputBuffer[idx++] = s4.w.x;
  // BUFFER: 31
  outputBuffer[idx++] = s4.w.y;
  // BUFFER: 32
}

[shader("compute")]
void computeMain()
{
    test();
}
