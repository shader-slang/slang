// generic-typealias-constraint.slang

// Test generic type alias with interface constraints.
// The alias preserves the constraint semantics and can be used
// as function parameter types.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -output-using-type

//TEST_INPUT: set output = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<int> output;

interface ITransform
{
    int apply(int x);
}

struct Doubler : ITransform
{
    int apply(int x) { return x * 2; }
}

struct Adder : ITransform
{
    int offset;
    int apply(int x) { return x + offset; }
}

struct Box<T : ITransform>
{
    T transformer;

    int run(int x) { return transformer.apply(x); }
}

// Generic type alias with a constraint
typealias TransformBox<T : ITransform> = Box<T>;

int applyBox<T : ITransform>(TransformBox<T> box, int val)
{
    return box.run(val);
}

[numthreads(1, 1, 1)]
void computeMain()
{
    Doubler d;
    TransformBox<Doubler> box1;
    box1.transformer = d;
    // 5 * 2 = 10
    // CHECK: 10
    output[0] = applyBox(box1, 5);

    Adder a;
    a.offset = 100;
    TransformBox<Adder> box2;
    box2.transformer = a;
    // 7 + 100 = 107
    // CHECK: 107
    output[1] = applyBox(box2, 7);

    // Direct use of Box with alias
    // CHECK: 20
    output[2] = box1.run(10);

    // Combined
    // CHECK: 117
    output[3] = applyBox(box1, 5) + applyBox(box2, 7);
}
