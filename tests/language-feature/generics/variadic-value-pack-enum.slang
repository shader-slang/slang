//TEST:INTERPRET(filecheck=CHECK):

enum Color
{
    Red = 0,
    Green = 1,
    Blue = 2,
}

struct ColorSet<let each C : Color>
{
    int getCount() { return countof(C); }

    void print()
    {
        expand printf("%d\n", (int)(each C));
    }
}

void main()
{
    ColorSet<Color.Red, Color.Green, Color.Blue> rgb;
    printf("%d\n", rgb.getCount());
    // CHECK: 3

    rgb.print();
    // CHECK: 0
    // CHECK: 1
    // CHECK: 2
}
