//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type
//TEST:SIMPLE(filecheck=REPORT):-target hlsl -stage compute -entry computeMain -report-dynamic-dispatch-sites -conformance "ConcreteNS.Vector:IFNS.IScalable=0" -conformance "ConcreteNS.Matrix:IFNS.IScalable=1" -conformance "ConcreteNS.Circle:IFNS.IShape=0" -conformance "ConcreteNS.Square:IFNS.IShape=1" -conformance "ConcreteNS.Circle:IFNS.IScalable=2" -conformance "ConcreteNS.Square:IFNS.IScalable=3"

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

namespace IFNS
{

interface IScalable
{
    This scale(float x);
}

interface IShape : IScalable
{
    float getArea();
    float getPerimeter();
}

}

namespace ConcreteNS
{

struct Circle : IFNS.IShape
{
    float radius;

    This scale(float x) { return {radius * x}; }
    float getArea() { return 3.14159f * radius * radius; }
    float getPerimeter() { return 2 * 3.14159f * radius; }
};

struct Square : IFNS.IShape
{
    float side;

    This scale(float x) { return {side * x}; }
    float getArea() { return side * side; }
    float getPerimeter() { return 4 * side; }
};

struct Vector : IFNS.IScalable
{
    float x, y;

    This scale(float factor) { return {x * factor, y * factor}; }
};

struct Matrix : IFNS.IScalable
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

}

IFNS.IShape createShape(uint id, float data)
{
    return createDynamicObject<IFNS.IShape>(id, data);
}

float f(uint id, float data, float scale)
{
    let obj = createShape(id, data);
    let scaledObj = obj.scale(scale);
    // REPORT-DAG: note[E50102]: generated dynamic dispatch code
    return scaledObj.getArea() + scaledObj.getPerimeter();
    // REPORT-DAG: note[E50102]: generated dynamic dispatch code
};


//TEST_INPUT: type_conformance ConcreteNS.Vector:IFNS.IScalable = 0
//TEST_INPUT: type_conformance ConcreteNS.Matrix:IFNS.IScalable = 1
//TEST_INPUT: type_conformance ConcreteNS.Circle:IFNS.IScalable = 2
//TEST_INPUT: type_conformance ConcreteNS.Square:IFNS.IScalable = 3

//TEST_INPUT: type_conformance ConcreteNS.Circle:IFNS.IShape = 0
//TEST_INPUT: type_conformance ConcreteNS.Square:IFNS.IShape = 1

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    outputBuffer[0] = f(0, 3, 3); // CHECK: 311.017426
    outputBuffer[1] = f(1, 3, 3); // CHECK: 117.000000
}
