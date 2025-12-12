//TEST(compute):SIMPLE(filecheck=CHECK):-dump-ir -stage compute -entry main -g2

void main()
{
    float x = 1.0;
    float y = 2.0;
    float result = x + y;
}

// CHECK: module
// CHECK: func main
// CHECK: DebugBuildIdentifier("{{.*}}", 0 : UInt)
