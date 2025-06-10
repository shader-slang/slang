//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):

public struct Visibility
{
  internal int x;
  public int y = 5;
  // will synthesize a constructor
  // __init(int y = 0)
}

void test()
{
    // CHECK: warning 41021: default initializer for 'Visibility' will not initialize field 'x'
    Visibility t1 = {};
}


