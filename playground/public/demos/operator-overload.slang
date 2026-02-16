// This example demonstrates how to overload various operators in Slang.

// Import the playground module so we can use the `printf` utility function.
import playground;

// Define a type that conforms to the `IMyHashable` interface.
struct MyPoint
{
    float x;
    float y;
    __init(float _x, float _y) { this.x = _x; this.y = _y; }

    // Overload `[]` operator to access the x and y components.
    __subscript(int i) -> float
    {
        get
        {
            if (i == 0) return x;
            if (i == 1) return y;
            return 0.0;
        }
        set
        {
            if (i == 0) x = newValue;
            if (i == 1) y = newValue;
        }
    }
}

// Overload operator `+`.
MyPoint operator+(MyPoint a, MyPoint b)
{
    return MyPoint(a.x + b.x, a.y + b.y);
}

void printMain()
{
    // `let` keyword is used to declare an immutable variable.
    let p0 = MyPoint(1.0, 2.0);
    let p1 = MyPoint(3.0, 4.0);

    // `var` keyword is used to declare a mutable variable.
    var p2 = p0 + p1;
    printf("P2 = (%f, %f)\n", p2[0], p2[1]);

    p2[1] = 10.0f;
    printf("P2 = (%f, %f)\n", p2[0], p2[1]);
}

