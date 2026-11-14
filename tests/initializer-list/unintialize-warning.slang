//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

struct CLike
{
  int x;
  int y;
  // compiler synthesizes:
  // __init(int x, int y);
}

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

struct DefaultMember {
  int x = 0;
  int y = 1;
  // compiler synthesizes:
  // __init(int x = 0, int y = 1);
}

struct PartialInit1 {
  int x;
  int y = 1;
  // compiler synthesizes:
  // __init(int x, int y = 1);
}

struct PartialInit2 {
  int x = 1;
  int y; // warning: not all members are initialized.
  // compiler synthesizes:
  // __init(int x, int y);
}

void func1(CLike c)
{
}

void func2(ExplicitCtor e)
{
}

void func3(DefaultMember d)
{
}

void func4(PartialInit1 p)
{
}

void func5(PartialInit2 p)
{
}

void test()
{
    CLike c; // `c` is uninitialized.
    func1(c);
//CHECK: ^ use of uninitialized variable
//CHECK: ^ use of uninitialized variable 'c'

    ExplicitCtor e; // `e` is uninitialized.
    func2(e);
//CHECK: ^ use of uninitialized variable
//CHECK: ^ use of uninitialized variable 'e'

    DefaultMember d; // `d` is uninitialized.
    func3(d);
//CHECK: ^ use of uninitialized variable
//CHECK: ^ use of uninitialized variable 'd'

    PartialInit1 p1; // `p` is uninitialized.
    func4(p1);
//CHECK: ^ use of uninitialized variable
//CHECK: ^ use of uninitialized variable 'p1'

    PartialInit2 p2; // `p` is uninitialized.
    func5(p2);
//CHECK: ^ use of uninitialized variable
//CHECK: ^ use of uninitialized variable 'p2'
}
