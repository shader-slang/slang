//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type -xslang -experimental-feature
//TEST:SIMPLE(filecheck=DEBUG):-target spirv-asm -dump-ir -o /dev/null -entry computeMain -stage compute -g -experimental-feature

// Verify that user code can name multi-index subscript accessors as function-extension targets.
// The two indices also lock down the parent-then-accessor parameter ordering.

struct Grid : IDifferentiable
{
    typedef Grid Differential;

    float4 values;

    [Differentiable]
    static Differential dzero()
    {
        return Grid(float4(0.0));
    }

    [Differentiable]
    static Differential dadd(Differential a, Differential b)
    {
        return Grid(a.values + b.values);
    }

    __subscript(int row, int column) -> float
    {
        get { return values[row * 2 + column]; }
        set { values[row * 2 + column] = newValue; }
    }
}

__func_extension fwd_diff(Grid::__subscript::get)(
    DifferentialPair<Grid> self,
    int row,
    int column) -> DifferentialPair<float>
{
    return diffPair(self.p[row, column], self.d[row, column]);
}

__func_extension bwd_diff(Grid::__subscript::get)(
    inout DifferentialPair<Grid> self,
    int row,
    int column,
    float dOut) -> void
{
    var dSelf = Grid.dzero();
    dSelf[row, column] = dOut;
    self = diffPair(self.p, dSelf);
}

__func_extension fwd_diff(Grid::__subscript::set)(
    inout DifferentialPair<Grid> self,
    int row,
    int column,
    DifferentialPair<float> newValue) -> void
{
    var primal = self.p;
    var differential = self.d;
    primal[row, column] = newValue.p;
    differential[row, column] = newValue.d;
    self = diffPair(primal, differential);
}

__func_extension bwd_diff(Grid::__subscript::set)(
    inout DifferentialPair<Grid> self,
    int row,
    int column,
    inout DifferentialPair<float> newValue) -> void
{
    var dSelf = self.d;
    newValue = diffPair(newValue.p, dSelf[row, column]);
    dSelf[row, column] = 0.0;
    self = diffPair(self.p, dSelf);
}

[Differentiable]
float readGrid(Grid value, int row, int column)
{
    return value[row, column];
}

[Differentiable]
Grid writeGrid(Grid value, int row, int column, float newValue)
{
    value[row, column] = newValue;
    return value;
}

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    let primal = Grid(float4(1.0, 2.0, 3.0, 4.0));
    let differential = Grid(float4(10.0, 20.0, 30.0, 40.0));

    let getResult = fwd_diff(readGrid)(diffPair(primal, differential), 1, 0);
    outputBuffer[0] = getResult.p;
    outputBuffer[1] = getResult.d;

    var getSelf = diffPair(primal);
    bwd_diff(readGrid)(getSelf, 0, 1, 5.0);
    outputBuffer[2] = getSelf.d.values[0];
    outputBuffer[3] = getSelf.d.values[1];

    let setResult =
        fwd_diff(writeGrid)(diffPair(primal, differential), 1, 1, diffPair(9.0, 90.0));
    outputBuffer[4] = setResult.p[1, 1];
    outputBuffer[5] = setResult.d[1, 1];

    var setSelfBackward = diffPair(primal);
    var newValue = diffPair(9.0);
    bwd_diff(writeGrid)(
        setSelfBackward,
        1,
        1,
        newValue,
        Grid(float4(5.0, 6.0, 7.0, 8.0)));
    outputBuffer[6] = setSelfBackward.d.values[2];
    outputBuffer[7] = setSelfBackward.d.values[3];
    outputBuffer[8] = newValue.d;

    // CHECK: 3.0
    // CHECK-NEXT: 30.0
    // CHECK-NEXT: 0.0
    // CHECK-NEXT: 5.0
    // CHECK-NEXT: 9.0
    // CHECK-NEXT: 90.0
    // CHECK-NEXT: 7.0
    // CHECK-NEXT: 0.0
    // CHECK-NEXT: 8.0
}

// The user-written implementations of custom backward derivatives are intentionally unnamed.
// Their linkage names must still identify the DebugFunction paired with each DebugLocation.
// DEBUG: let [[GET_DEBUG:%[0-9]+]] : Void = DebugFunction("_S{{.*}}", [[GET_LINE:[0-9]+]] : UInt, 50 : UInt
// DEBUG: [DebugFunction([[GET_DEBUG]])]
// DEBUG-NEXT: [DebugLocation({{.*}}, [[GET_LINE]] : UInt, 50 : UInt)]
// DEBUG: let [[SET_DEBUG:%[0-9]+]] : Void = DebugFunction("_S{{.*}}", [[SET_LINE:[0-9]+]] : UInt, 50 : UInt
// DEBUG: [DebugFunction([[SET_DEBUG]])]
// DEBUG-NEXT: [DebugLocation({{.*}}, [[SET_LINE]] : UInt, 50 : UInt)]
