// This example demonstrates lambda expressions in Slang.

// See language feature propsoal at:
// https://github.com/shader-slang/spec/blob/main/proposals/025-lambda-1.md

import playground;

public struct MyData
{
    int data[4];
    MyData map(IFunc<int, int> f)
    {
        MyData result;
        for (int i = 0; i < data.getCount(); i++)
            result.data[i] = f(data[i]);
        return result;
    }
}

public void printMain()
{
    let p = MyData({ 1, 2, 3, 4 });

    int env = 1;
    let q = p.map((int x) => x * 2 + env);

    // Alternative syntax:
    // let q = p.map((int x) => { return x * 2 + env; });
    
    printf("q = (%d, %d, %d, %d)\n", q.data[0], q.data[1], q.data[2], q.data[3]);
}