//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

public struct NonCStyle
{}
public struct Visibility
{
  internal int x;
  public int[] y; // define a unsized array here on purpose so that the ctor will not associate it with a '{}' initializer
  // will synthesize a constructor
  // __init(int[] y)
}

void test()
{
    Visibility t1 = {};
//CHECK:            ^ not enough arguments to call
}


