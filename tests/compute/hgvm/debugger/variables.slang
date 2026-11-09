//TEST:DEBUGGER(mode=CLI, prefix=CLI):

struct Item { float2 position; uint label; }
class Box { uint value; __init() {} }
[noinline] uint index() { return 1; }

[host]
void main()
{
    Item items[2] = { { float2(1, 2), 3 }, { float2(4, 5), 6 } };
    uint i = index();
    items[i].position.x = 9;
    items[0].label = 7;
    i = 0;
    Item partial;
    partial.position.x = 12;
    partial.position.y = 13;
    partial.label = 14;
    double wide = 16777217.0LF;
    String text = "hello";
    Box object = new Box();
    object.value = 42;
    RuntimeArray<uint> dynamic = RuntimeArray<uint>::alloc(2);
    dynamic[0] = 11;
    dynamic[1] = 12;
    float2x2 matrix = float2x2(1, 2, 3, 4);
    matrix[1][0] = 8;
    //CLI-SEND: break '$file':[[@LINE+1]]
    print(text);
    print(items[i].position.x);
    print(wide);
    print(object.value);
    print(dynamic[1]);
    print(matrix[1][0]);
}

//CLI-SEND: run
//CLI-CHECK: Breakpoint 1, #0 main at
//CLI-SEND: p items[1].position
//CLI-CHECK: $1 = (9, 5)
//CLI-SEND: p items[0]
//CLI-CHECK: $2 = Item {position = (1, 2), label = 7}
//CLI-SEND: p items[1].label
//CLI-CHECK: $3 = 6
//CLI-SEND: p wide
//CLI-CHECK: $4 = 16777217
//CLI-SEND: whatis wide
//CLI-CHECK: type = double
//CLI-SEND: p text
//CLI-CHECK: $5 = "hello"
//CLI-SEND: p object.value
//CLI-CHECK: $6 = 42
//CLI-SEND: p dynamic[1]
//CLI-CHECK: $7 = 12
//CLI-SEND: p matrix[1][0]
//CLI-CHECK: $8 = 8
//CLI-SEND: whatis matrix[1]
//CLI-CHECK: type = vector<float, 2>
//CLI-SEND: p partial
//CLI-CHECK: $9 = Item {position = (12, 13), label = 14}
//CLI-SEND: p matrix[1]
//CLI-CHECK: $10 = (8, 4)
//CLI-SEND: p object
//CLI-CHECK: $11 = Box {value = 42}
//CLI-SEND: p items[99]
//CLI-CHECK: Array index out of bounds.
//CLI-SEND: p missing
//CLI-CHECK: No such variable in the selected frame.
//CLI-SEND: continue
//CLI-CHECK: Program exited normally.
//CLI-SEND: quit
