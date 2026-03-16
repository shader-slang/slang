//TEST:INTERPRET(filecheck=CHECK):

// Test that generic structs with multiple defaulted parameters work correctly
// when some are provided and some are defaulted.

struct Grid<T = float, let Rows : int = 2, let Cols : int = 3>
{
    int getTotalCells() { return Rows * Cols; }
}

void main()
{
    // All defaults: T=float, Rows=2, Cols=3.
    Grid<> g1;
    printf("%d\n", g1.getTotalCells());
    // CHECK: 6

    // Override only T, keep Rows=2 and Cols=3.
    Grid<int> g2;
    printf("%d\n", g2.getTotalCells());
    // CHECK: 6

    // Override T and Rows, keep Cols=3.
    Grid<int, 4> g3;
    printf("%d\n", g3.getTotalCells());
    // CHECK: 12

    // Override all three parameters.
    Grid<int, 5, 5> g4;
    printf("%d\n", g4.getTotalCells());
    // CHECK: 25
}
