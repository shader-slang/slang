//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

public struct Visibility
{
  internal int x;
  public int y = 0;
  // will synthesize a constructor
  // __init(int y = 0)
}

void test()
{
    Visibility t1 = {1, 2}; // error, no matching ctor
//CHECK:            ^ too many arguments to call
}


