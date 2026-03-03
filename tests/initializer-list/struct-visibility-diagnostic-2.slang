//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

public struct Visibility
{
  internal int x;
//CHECK:       ^ default initializer will not initialize field
//CHECK:       ^ default initializer for 'Visibility' will not initialize field 'x'
  public int y = 5;
  // will synthesize a constructor
  // __init(int y = 0)
}

void test()
{
    Visibility t1 = {};
}


