//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):

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
    //CHECK: error 39999: not enough arguments to call (got 0, expected 1)
    Visibility t1 = {};
}


