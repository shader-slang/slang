//TEST:INTERPRET(filecheck=CHECK):

interface IShape
{
    int area();
}

struct Square : IShape
{
    int side;
    int area() { return side * side; }
}

void main()
{
    // Coerce Optional<Square> with a value to Optional<IShape>
    Square sq = { 3 };
    Optional<Square> optSq = sq;
    Optional<IShape> sh = optSq; // implicit coercion: Optional<Square> -> Optional<IShape>

    // CHECK: 9
    if (sh.hasValue)
        printf("%d\n", sh.value.area());

    // Coerce Optional<Square>(none) to Optional<IShape> — must preserve hasValue=false
    Optional<Square> noSq = none;
    Optional<IShape> noSh = noSq; // implicit coercion: Optional<Square>(none) -> Optional<IShape>

    // CHECK: none
    if (!noSh.hasValue)
        printf("none\n");
}
