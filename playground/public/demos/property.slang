// This example demonstrates how to use properties in Slang.

import playground;

public struct MyPoint
{
    // Use a 32bit integer to store x and y in normalized [0, 1] range.
    private uint data;

    public property float x
    {
        get { return (data & 0xFFFF) / 65535.0; }
        set { data = (data & 0xFFFF0000) | (uint)(newValue * 65535); }
    }

    public property float y
    {
        get { return (data >> 16) / 65535.0; }
        set { data = (data & 0xFFFF) | ((uint)(newValue * 65535) << 16); }
    }
}

public void printMain()
{
    MyPoint p;
    p.x = 0.5;
    p.y = 0.25;
    printf("P = (%f, %f)\n", p.x, p.y);
}