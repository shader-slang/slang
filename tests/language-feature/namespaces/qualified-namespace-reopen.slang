// Regression test for https://github.com/shader-slang/slang/issues/13701
//
// A qualified namespace declaration `namespace A::B { ... }` (or `namespace A.B { ... }`)
// expands as if the user had written `namespace A { namespace B { ... } }`. Unqualified
// lookup inside the body therefore sees the members that this module declares in every
// enclosing namespace, including the enclosing namespaces that already existed before
// the qualified declaration reopened them. Imported members of an existing enclosing
// namespace are covered by namespace-import-reopen/qualified-reopen.slang.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

namespace Outer
{
static const int outerValue = 1;
namespace Inner
{
}
}

// `Outer` and `Inner` both exist already.
namespace Outer::Inner
{
int getWithScopeSeparator()
{
    return outerValue;
}
}

// The `.` separator takes the same path as `::`.
namespace Outer.Inner
{
int getWithDotSeparator()
{
    return outerValue + 1;
}
}

static const int globalOrA = 1000;
int pick()
{
    return 1000;
}

namespace A
{
static const int globalOrA = 30;
int pick()
{
    return 40;
}
static const int aValue = 10;
static const int shadowed = 100;
namespace B
{
static const int bValue = 20;
static const int shadowed = 200;
namespace C
{
}
}
}

// Every component of the qualified name is reopened.
namespace A::B::C
{
int getFromA()
{
    return aValue;
}
int getFromB()
{
    return bValue;
}
// The innermost enclosing namespace that declares the name wins, as it does for the
// nested spelling.
int getShadowed()
{
    return shadowed;
}
// A member of an enclosing namespace hides a global of the same name.
int getGlobalOrA()
{
    return globalOrA;
}
// A function of an enclosing namespace hides a global function of the same name.
int getPick()
{
    return pick();
}
}

// `A` and `B` exist and `D` is new.
namespace A::B::D
{
int getFromA()
{
    return aValue + 1;
}
}

// A qualified declaration inside an enclosing namespace body reopens `B` and `C`.
namespace A
{
namespace B::C
{
int getFromBInsideBody()
{
    return bValue + 1;
}
}
}

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = Outer::Inner::getWithScopeSeparator();
    outputBuffer[1] = Outer::Inner::getWithDotSeparator();
    outputBuffer[2] = A::B::C::getFromA();
    outputBuffer[3] = A::B::C::getFromB();
    outputBuffer[4] = A::B::C::getShadowed();
    outputBuffer[5] = A::B::D::getFromA();
    outputBuffer[6] = A::B::C::getFromBInsideBody();
    outputBuffer[7] = A::B::C::getGlobalOrA();
    outputBuffer[8] = A::B::C::getPick();
}

// CHECK: type: int32_t
// CHECK-NEXT: {{^}}1{{$}}
// CHECK-NEXT: {{^}}2{{$}}
// CHECK-NEXT: {{^}}10{{$}}
// CHECK-NEXT: {{^}}20{{$}}
// CHECK-NEXT: {{^}}200{{$}}
// CHECK-NEXT: {{^}}11{{$}}
// CHECK-NEXT: {{^}}21{{$}}
// CHECK-NEXT: {{^}}30{{$}}
// CHECK-NEXT: {{^}}40{{$}}
