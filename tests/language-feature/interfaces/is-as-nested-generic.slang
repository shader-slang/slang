// is-as-nested-generic.slang

// Functional test: verify that `is`/`as` on generic-parameterized interface
// values produce correct results (issue #10563).

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

[anyValueSize(16)]
interface IAnimal
{
    int speak();
}

struct Dog : IAnimal
{
    int val;
    int speak() { return val; }
}

struct Cat : IAnimal
{
    int val;
    int speak() { return val; }
}

struct Box<T : IAnimal>
{
    T value;
}

//TEST_INPUT: type_conformance Dog:IAnimal = 0
//TEST_INPUT: type_conformance Cat:IAnimal = 1

bool testIsAnimal<T : IAnimal>(IAnimal animal)
{
    return animal is T;
}

int testAsAnimal<T : IAnimal>(IAnimal animal)
{
    let result = animal as T;
    if (result.hasValue)
        return result.value.speak();
    return -1;
}

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    Dog d = { 10 };
    int2 data = int2(10, 0);
    IAnimal animal = createDynamicObject<IAnimal, int2>(0, data);

    // Dog is Dog -> true (1)
    outputBuffer[0] = testIsAnimal<Dog>(animal) ? 1 : 0;
    // Dog is Cat -> false (0)
    outputBuffer[1] = testIsAnimal<Cat>(animal) ? 1 : 0;
    // Dog as Dog -> 10
    outputBuffer[2] = testAsAnimal<Dog>(animal);
    // Dog as Cat -> -1
    outputBuffer[3] = testAsAnimal<Cat>(animal);
}

// CHECK: 1
// CHECK: 0
// CHECK: 10
// CHECK: -1
