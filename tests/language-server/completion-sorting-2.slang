//TEST:LANG_SERVER(filecheck=CHECK):

// Test that completion candidates are sorted according to context.

enum Fruit { Orange, Apple, Banana }
enum Color { Red, Green, Blue }
void expect(Fruit e) {}
void expect(Color e) {}


void test()
{
//COMPLETE:14,12
    expect(); // should list `Fruit` and `Color` first.

}

// CHECK: Fruit:{{.*}}sort(0:Fruit)
// CHECK: Color:{{.*}}sort(0:Color)