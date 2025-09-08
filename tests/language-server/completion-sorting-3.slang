//TEST:LANG_SERVER(filecheck=CHECK):

// Test that completion candidates are sorted according to context.

enum Fruit { Orange, Apple, Banana }
struct MyType<Fruit v> {}


void test()
{
//COMPLETE:12,12
    MyType< // should list `Fruit` first.

}

// CHECK: Fruit:{{.*}}sort(0:Fruit)