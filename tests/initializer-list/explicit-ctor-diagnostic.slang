//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

struct ExplicitCtor
{
   int x;
   int y;
   __init(int x)
   {
      this.x = x;
      this.y = 0;
   }
  // compiler does not synthesize any ctors.
}

void test()
{
   ExplicitCtor e = {1, 2}; // error, no ctor matches initializer list.
//CHECK:            ^ too many arguments to call
//CHECK:            ^ too many arguments to call (got 2, expected 1)

   ExplicitCtor e1 = {};
//CHECK:             ^ not enough arguments to call
//CHECK:             ^ not enough arguments to call (got 0, expected 1)
}
