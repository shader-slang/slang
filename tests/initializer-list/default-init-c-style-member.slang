//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUFFER):-shaderobj -vk
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUFFER):-shaderobj

struct NonCStyle
{
  int x;
  __init(int x)
  {
    this.x = x;
  }
}

struct CLike
{
  NonCStyle nonCStyleMember;
  int x;
  int y;
  // compiler synthesizes:
  // __init(NonCStyle nonCStyleMember, int x = {}, int y = {});
}

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;
void test()
{
  // case 1: initialized with synthesized ctor call, but CLike::x and CLike::y are default initialized
  // and `c1` is now `{1, 0, 0}`.
  CLike c1 = {NonCStyle(1)};

  // case 2: initialized with synthesized ctor call, CLike::y are default initialized
  CLike c2 = {NonCStyle(1), 2};

  // case 3: initialized with synthesized ctor call
  CLike c3 = {NonCStyle(1), 2, 3};

  outputBuffer[0] = c1.nonCStyleMember.x;
  // BUFFER: 1
  outputBuffer[1] = c1.x;
  // BUFFER-NEXT: 0
  outputBuffer[2] = c1.y;
  // BUFFER-NEXT: 0

  outputBuffer[3] = c2.nonCStyleMember.x;
  // BUFFER-NEXT: 1
  outputBuffer[4] = c2.x;
  // BUFFER-NEXT: 2
  outputBuffer[5] = c2.y;
  // BUFFER-NEXT: 0

  outputBuffer[6] = c3.nonCStyleMember.x;
  // BUFFER-NEXT: 1
  outputBuffer[7] = c3.x;
  // BUFFER-NEXT: 2
  outputBuffer[8] = c3.y;
  // BUFFER-NEXT: 3
}

[shader("compute")]
void computeMain()
{
    test();
}
