//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

public struct Visibility
{
  internal int x = 1;
  public int y = 5;
  // compiler synthesizes:
  // public __init(int y = 0);
}

void test()
{
    Visibility t1 = {1, 2};
//CHECK:            ^ too many arguments to call
}

