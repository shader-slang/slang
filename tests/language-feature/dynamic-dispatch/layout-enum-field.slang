// Enum fields in interface implementations are marshalled via their
// underlying integer tag type. This tests that enum values survive
// AnyValue packing and can be used in dispatch for branching logic.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-metal -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-wgsl -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

enum Color
{
    Red = 0,
    Green = 1,
    Blue = 2,
}

enum Priority
{
    Low = 10,
    Medium = 20,
    High = 30,
}

interface IItem
{
    int colorValue();
    int priorityValue();
    int combined();
}

struct TaggedItem : IItem
{
    Color color;
    Priority priority;
    int multiplier;

    int colorValue() { return int(color); }
    int priorityValue() { return int(priority); }
    int combined() { return int(color) + int(priority) * multiplier; }
}

struct SimpleItem : IItem
{
    Color color;

    int colorValue() { return int(color); }
    int priorityValue() { return 0; }
    int combined() { return int(color); }
}

IItem makeItem(int id)
{
    if (id == 0)
    {
        TaggedItem item;
        item.color = Color.Blue;
        item.priority = Priority.High;
        item.multiplier = 3;
        return item;
    }
    else
    {
        SimpleItem item;
        item.color = Color.Green;
        return item;
    }
}

[numthreads(1, 1, 1)]
void computeMain(int id : SV_DispatchThreadID)
{
    // TaggedItem: color=Blue(2), priority=High(30), multiplier=3
    var a = makeItem(id);
    outputBuffer[0] = a.colorValue();
    // CHECK: 2
    outputBuffer[1] = a.priorityValue();
    // CHECK: 30
    outputBuffer[2] = a.combined();
    // CHECK: 92

    // SimpleItem: color=Green(1)
    var b = makeItem(id + 1);
    outputBuffer[3] = b.colorValue();
    // CHECK: 1
    outputBuffer[4] = b.priorityValue();
    // CHECK: 0
    outputBuffer[5] = b.combined();
    // CHECK: 1
}
