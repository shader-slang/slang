//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type
//TEST:SIMPLE(filecheck=REPORT):-target hlsl -stage compute -entry computeMain -report-dynamic-dispatch-sites -conformance "Vector:IScalable=0" -conformance "Matrix:IScalable=1" -conformance "Circle:IShape=0" -conformance "Square:IShape=1" -conformance "Circle:IScalable=2" -conformance "Square:IScalable=3"

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

interface IScalable
{
    This scale(float x);
}

interface IShape : IScalable
{
    float getArea();
    float getPerimeter();
}

struct Circle : IShape
{
    float radius;

    This scale(float x) { return {radius * x}; }
    float getArea() { return 3.14159f * radius * radius; }
    float getPerimeter() { return 2 * 3.14159f * radius; }
};

struct Square : IShape
{
    float side;

    This scale(float x) { return {side * x}; }
    float getArea() { return side * side; }
    float getPerimeter() { return 4 * side; }
};

struct Vector : IScalable
{
    float x, y;

    This scale(float factor) { return {x * factor, y * factor}; }
};

struct Matrix : IScalable
{
    float m[4][4];

    This scale(float factor)
    {
        Matrix result;
        for (int i = 0; i < 4; ++i)
            for (int j = 0; j < 4; ++j)
                result.m[i][j] = m[i][j] * factor;
        return result;
    }
};

IShape createShape(uint id, float data)
{
    return createDynamicObject<IShape>(id, data);
}

float f(uint id, float data, float scale)
{
    let obj = createShape(id, data);
    let scaledObj = obj.scale(scale);
    // REPORT-DAG: ([[# @LINE-1]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
    return scaledObj.getArea() + scaledObj.getPerimeter();
    // REPORT-DAG: ([[# @LINE-1]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
};


//TEST_INPUT: type_conformance Vector:IScalable = 0
//TEST_INPUT: type_conformance Matrix:IScalable = 1
//TEST_INPUT: type_conformance Circle:IScalable = 2
//TEST_INPUT: type_conformance Square:IScalable = 3

//TEST_INPUT: type_conformance Circle:IShape = 0
//TEST_INPUT: type_conformance Square:IShape = 1

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    outputBuffer[0] = f(0, 3, 3); // CHECK: 311.017426
    outputBuffer[1] = f(1, 3, 3); // CHECK: 117.000000
}