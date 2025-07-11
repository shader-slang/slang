//TEST:LANG_SERVER(filecheck=CHECK):

// Test that completion candidates are sorted according to context.

enum Fruit { Orange, Apple, Banana }
enum Color { Red, Green, Blue }
void expect(Fruit e) {}
void expect(Color e) {}


void test()
{
//COMPLETE:14,15
    Fruit t =  // should list `Fruit` first.

}

// CHECK: Fruit:{{.*}}sort(0:Fruit)