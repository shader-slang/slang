//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// A type conjunction (`A & B`) has no representation as a value: it is legal only as a generic
// constraint or an inheritance clause. Written anywhere a value type is expected it must produce
// E30133 (never reach member lookup / subtype checking as a bare `AndType`, which asserts). The
// rejection lives in one bottleneck -- `CoerceToProperType` -- so every value-type position below,
// including a conjunction nested in a pointer, array, or generic argument, is caught. (Functional
// construction `IBoth(x)` is the one exception, rejected in `ResolveInvoke` because a type-callee's
// constructors are looked up before it is coerced as a value type.) (#12873)

interface IFirst { void first(); }
interface ISecond { void second(); }

typealias IBoth = IFirst & ISecond;

// Positive controls: a conjunction remains valid as a generic constraint and an inheritance clause,
// and naming one with a `typealias` is fine -- these must still compile.
void useAsConstraint<T : IBoth>(T value)
{
    value.first();
    value.second();
}

struct Pair : IBoth
{
    void first() {}
    void second() {}
}

// --- Direct value-type positions ---

// This is the reported repro (#12873): the member access is what used to reach member lookup with
// an un-flattened `AndType` and abort. Rejecting the parameter type keeps it from ever getting there.
void useAsParam(IBoth value) { value.first(); }
//CHECK: E30133
//CHECK: conjunction

void useAsLocal() { IBoth local; }
//CHECK: E30133
//CHECK: conjunction

struct Holder { IBoth field; }
//CHECK: E30133
//CHECK: conjunction

IBoth makeBoth() { }
//CHECK: E30133
//CHECK: conjunction

void useAsThrows() throws IBoth { }
//CHECK: E30133
//CHECK: conjunction

bool useAsIs(Pair p) { return p is IBoth; }
//CHECK: E30133
//CHECK: conjunction

void useAsAs(Pair p) { let x = p as IBoth; }
//CHECK: E30133
//CHECK: conjunction

void useAsCast(Pair p) { let x = (IBoth)p; }
//CHECK: E30133
//CHECK: conjunction

void useAsConstruct(Pair p) { let x = IBoth(p); }
//CHECK: E30133
//CHECK: conjunction

interface IHasConjunctionProperty { property IBoth prop { get; } }
//CHECK: E30133
//CHECK: conjunction

interface IHasConjunctionSubscript { __subscript(int i) -> IBoth { get; } }
//CHECK: E30133
//CHECK: conjunction

// --- Conjunction nested in another type used as a value: also caught at the bottleneck ---

void useViaPointer(IBoth* p) { }
//CHECK: E30133
//CHECK: conjunction

struct HolderArray { IBoth values[4]; }
//CHECK: E30133
//CHECK: conjunction

void useViaOptional(Optional<IBoth> o) { }
//CHECK: E30133
//CHECK: conjunction

struct Box<T> { T value; }

void useAsGenericArg(Box<IBoth> box) { }
//CHECK: E30133
//CHECK: conjunction

void take<T>(T v) { }
void useAsGenericFuncArg(Pair p) { take<IBoth>(p); }
//CHECK: E30133
//CHECK: conjunction
