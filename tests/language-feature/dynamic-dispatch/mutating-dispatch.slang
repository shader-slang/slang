// [mutating] interface methods must pass `this` by inout through the dispatch
// wrapper so the concrete implementation can modify its own state. The wrapper
// unpacks AnyValue → concrete, calls the method, then repacks → AnyValue.
// The tag (witness table ID) remains unchanged since the concrete type doesn't change.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST:SIMPLE(filecheck=REPORT):-target hlsl -stage compute -entry computeMain -report-dynamic-dispatch-sites

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

interface ICounter
{
    int get();
    [mutating] void increment();
    [mutating] void add(int n);
}

struct Counter : ICounter
{
    int value;

    int get() { return value; }
    [mutating] void increment() { value++; }
    [mutating] void add(int n) { value += n; }
}

struct DoubleCounter : ICounter
{
    int value;

    int get() { return value; }
    [mutating] void increment() { value += 2; }
    [mutating] void add(int n) { value += n * 2; }
}

ICounter makeCounter(int id)
{
    if (id == 0)
        return Counter(0);
    else
        return DoubleCounter(0);
}

// Nested dispatch: IContainer.updateItem copies the ICounter parameter into a
// local var, calls [mutating] increment() on it, then reads the result. This
// exercises [mutating] dynamic dispatch inside another dispatched method.
interface IContainer
{
    [mutating] void updateItem(ICounter counter);
    int getCached();
}

struct MyContainer : IContainer
{
    int cached;

    [mutating] void updateItem(ICounter counter)
    {
        var local = counter;
        local.increment();
        cached = local.get();
    }

    int getCached() { return cached; }
}

struct OffsetContainer : IContainer
{
    int cached;
    int offset;

    [mutating] void updateItem(ICounter counter)
    {
        var local = counter;
        local.increment();
        cached = local.get() + offset;
    }

    int getCached() { return cached; }
}

IContainer makeContainer(int id)
{
    if (id == 0)
        return MyContainer(0);
    else
        return OffsetContainer(0, 100);
}

void mutateViaInout(inout ICounter counter)
{
    counter.add(7);
    // REPORT-DAG: ([[# @LINE-1]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
}

[numthreads(1, 1, 1)]
void computeMain(int id : SV_DispatchThreadID)
{
    // --- Test 1: Direct [mutating] calls on interface-typed local (Counter) ---
    // id == 0 at runtime with [numthreads(1,1,1)], but compiler doesn't know that.
    var c = makeCounter(id);

    c.increment();
    // REPORT-DAG: ([[# @LINE-1]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
    outputBuffer[0] = c.get();      // Counter: 0 + 1 = 1
    // REPORT-DAG: ([[# @LINE-1]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
    // CHECK: 1

    c.add(5);
    // REPORT-DAG: ([[# @LINE-1]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
    outputBuffer[1] = c.get();      // Counter: 1 + 5 = 6
    // REPORT-DAG: ([[# @LINE-1]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
    // CHECK: 6

    // --- Test 2: Direct [mutating] calls on interface-typed local (DoubleCounter) ---
    var d = makeCounter(id + 1);

    d.increment();
    // REPORT-DAG: ([[# @LINE-1]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
    outputBuffer[2] = d.get();      // DoubleCounter: 0 + 2 = 2
    // REPORT-DAG: ([[# @LINE-1]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
    // CHECK: 2

    d.add(3);
    // REPORT-DAG: ([[# @LINE-1]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
    outputBuffer[3] = d.get();      // DoubleCounter: 2 + 6 = 8
    // REPORT-DAG: ([[# @LINE-1]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
    // CHECK: 8

    // --- Test 3: Multiple increments to verify state accumulates ---
    var e = makeCounter(id);
    e.increment();
    // REPORT-DAG: ([[# @LINE-1]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
    e.increment();
    // REPORT-DAG: ([[# @LINE-1]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
    e.increment();
    // REPORT-DAG: ([[# @LINE-1]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
    outputBuffer[4] = e.get();      // Counter: 3
    // REPORT-DAG: ([[# @LINE-1]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
    // CHECK: 3

    // --- Test 4: [mutating] on interface passed as inout parameter ---
    var f = makeCounter(id);
    f.add(10);
    // REPORT-DAG: ([[# @LINE-1]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
    mutateViaInout(f);
    outputBuffer[5] = f.get();      // Counter: 10 + 7 = 17
    // REPORT-DAG: ([[# @LINE-1]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
    // CHECK: 17

    // --- Test 5: Nested dispatch — [mutating] on ICounter inside dispatched IContainer method ---
    var g = makeCounter(id);
    g.add(42);
    // REPORT-DAG: ([[# @LINE-1]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
    var container = makeContainer(id);
    container.updateItem(g);                    // updateItem copies counter, calls increment(), reads get()
    // REPORT-DAG: ([[# @LINE-1]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
    outputBuffer[6] = container.getCached();    // MyContainer: local.increment() on Counter(42) → 43
    // REPORT-DAG: ([[# @LINE-1]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
    // CHECK: 43

    // --- Test 5b: Nested dispatch — OffsetContainer path ---
    var g2 = makeCounter(id);
    g2.add(10);
    // REPORT-DAG: ([[# @LINE-1]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
    var container2 = makeContainer(id + 1);
    container2.updateItem(g2);                    // OffsetContainer: local.increment() on Counter(10) → 11, + offset 100
    // REPORT-DAG: ([[# @LINE-1]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
    outputBuffer[7] = container2.getCached();     // OffsetContainer: 11 + 100 = 111
    // REPORT-DAG: ([[# @LINE-1]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
    // CHECK: 111

    // --- Test 6: Mutation after copy does not affect original (value semantics) ---
    var h = makeCounter(id);
    h.add(100);
    // REPORT-DAG: ([[# @LINE-1]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
    var copy = h;
    copy.add(50);
    // REPORT-DAG: ([[# @LINE-1]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
    outputBuffer[8] = h.get();     // original: still 100
    // REPORT-DAG: ([[# @LINE-1]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
    // CHECK: 100
    outputBuffer[9] = copy.get();  // copy: 100 + 50 = 150
    // REPORT-DAG: ([[# @LINE-1]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
    // CHECK: 150
}
