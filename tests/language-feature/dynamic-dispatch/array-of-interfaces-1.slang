//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type -Xslang -Wno-41020

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

//
// Test struct fields with array of interface types, with a single implementation for the 
// interface.
// 

public interface Term 
{
  float evaluate(float4 u);
}

public struct Sum 
{
  Term terms[16];
  uint count;

  public __init() 
  { 
    count = 0; 
    for (uint i = 0; i < 16; i++) 
      terms[i] = Multiply();
  }

  [mutating]
  public void add(Term term, float weight) 
  {
    terms[count] = term;
    count += 1;
  }

  public float evaluate(float4 u) { return terms[0].evaluate(u); }
}

public struct Multiply : Term 
{
  public float evaluate(float4 u) { return u.x * u.y; }
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 threadId: SV_DispatchThreadID) {

  let pixel = uint2(threadId.x, threadId.y);
  Sum terms;
  terms.add(Multiply(), 1.0);
  outputBuffer[0] = terms.evaluate(float4(0.5)); // CHECK: 0.25
}