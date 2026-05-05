//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

public struct Visibility
{
  internal int x;
  public int y = 0;
  // Mixed visibility: internal x != public struct.
  // A synthesized public constructor is created for the public member only: __init(int y = 0).
}

void test()
{
    Visibility t1 = {1, 2}; // error: synthesized ctor takes 1 arg (y), not 2
//CHECK:            ^ too many arguments to call
}
