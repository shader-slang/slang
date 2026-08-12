//TEST:INTERPRET(filecheck=CHECK):

// An interface may constrain several associated types to the *same* interface:
//
//     associatedtype NetA : IMLP;
//     associatedtype NetB : IMLP;
//
// Each `: IMLP` bound is represented as an interface-level constraint
// requirement (a sibling of the associated type). These constraints must get
// *distinct* requirement keys -- their mangled names have to encode which
// associated type is constrained, not just the shared bound `IMLP`. If they
// collapse to the same key, witness-table cloning merges the two conformance
// entries (dropping one), so both `getA().run()` and `getB().run()` resolve to
// the *same* implementation and the result is wrong (and, on the deferred
// witness-table-clone path used when linking precompiled modules, the duplicate
// key aborts compilation with an internal "key already exists" error).
//
// Regression test for the Falcor `INeuralMaterialParams` shape (four
// `associatedtype ... : IMLP` members).

interface IMLP
{
    float run(float x);
}

interface IParams
{
    associatedtype NetA : IMLP;
    associatedtype NetB : IMLP;
    associatedtype NetC : IMLP;
    NetA getA();
    NetB getB();
    NetC getC();
}

struct MlpA : IMLP { float run(float x) { return x + 10.0; } }
struct MlpB : IMLP { float run(float x) { return x + 20.0; } }
struct MlpC : IMLP { float run(float x) { return x + 40.0; } }

struct MyParams : IParams
{
    typealias NetA = MlpA;
    typealias NetB = MlpB;
    typealias NetC = MlpC;
    MlpA getA() { MlpA m; return m; }
    MlpB getB() { MlpB m; return m; }
    MlpC getC() { MlpC m; return m; }
}

// Generic over `IParams` so each `NetX : IMLP` conformance is looked up through
// the witness table (the path that exercises the per-constraint keys).
float useParams<T : IParams>(T p, float x)
{
    return p.getA().run(x) + p.getB().run(x) + p.getC().run(x);
}

void main()
{
    MyParams p;
    // (1+10) + (1+20) + (1+40) = 11 + 21 + 41 = 73.
    // If the three `: IMLP` constraint keys collide, the calls resolve to a
    // single implementation and the sum is wrong (e.g. 33).
    // CHECK: 73
    printf("%d\n", (int)useParams(p, 1.0));
}
