//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):

public struct Visibility
{
  internal int x;
  public int y = 0;
  // will synthesize a constructor
  // __init(int y = 0)
}

void test()
{
    // CHECK: error 39999: too many arguments to call (got 2, expected 1)
    Visibility t1 = {1, 2}; // error, no matching ctor
}


