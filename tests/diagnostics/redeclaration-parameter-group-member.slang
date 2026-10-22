//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// The members of a `cbuffer` or `tbuffer` are visible in the enclosing scope, so they
// conflict with other declarations of the same name in that scope (shader-slang/slang#13876).
// The later declaration gets the diagnostic.

// The reproducer from the issue.

static const int overloadedThing = 1;
/*CHECK:
                 ^^^^^^^^^^^^^^^ see previous declaration of 'overloadedThing'
*/

[UnscopedEnum]
enum TestEnum
{
    overloadedThing = 2,
/*CHECK:
    ^^^^^^^^^^^^^^^ conflicting declaration
    ^^^^^^^^^^^^^^^ declaration of 'overloadedThing' conflicts with existing declaration
    ^^^^^^^^^^^^^^^ see previous declaration of 'overloadedThing'
*/
};

cbuffer CBUF
{
    int overloadedThing;
/*CHECK:
        ^^^^^^^^^^^^^^^ conflicting declaration
        ^^^^^^^^^^^^^^^ declaration of 'overloadedThing' conflicts with existing declaration
        ^^^^^^^^^^^^^^^ see previous declaration of 'overloadedThing'
*/
}

tbuffer TBUF
{
    int overloadedThing;
/*CHECK:
        ^^^^^^^^^^^^^^^ conflicting declaration
        ^^^^^^^^^^^^^^^ declaration of 'overloadedThing' conflicts with existing declaration
*/
}

// A declaration after the buffer gets the diagnostic.

cbuffer BufferFirst
{
    int memberThenConst;
/*CHECK:
        ^^^^^^^^^^^^^^^ see previous declaration of 'memberThenConst'
*/
}

static const int memberThenConst = 1;
/*CHECK:
                 ^^^^^^^^^^^^^^^ conflicting declaration
                 ^^^^^^^^^^^^^^^ declaration of 'memberThenConst' conflicts with existing declaration
*/

tbuffer TBufferFirst
{
    int tbufferMemberThenConst;
/*CHECK:
        ^^^^^^^^^^^^^^^^^^^^^^ see previous declaration of 'tbufferMemberThenConst'
*/
}

static const int tbufferMemberThenConst = 1;
/*CHECK:
                 ^^^^^^^^^^^^^^^^^^^^^^ conflicting declaration
                 ^^^^^^^^^^^^^^^^^^^^^^ declaration of 'tbufferMemberThenConst' conflicts with existing declaration
*/

// Members of two buffers.

cbuffer FirstBuffer
{
    int sharedMember;
/*CHECK:
        ^^^^^^^^^^^^ see previous declaration of 'sharedMember'
*/
    float otherMember;
/*CHECK:
          ^^^^^^^^^^^ see previous declaration of 'otherMember'
*/
}

cbuffer SecondBuffer
{
    float sharedMember;
/*CHECK:
          ^^^^^^^^^^^^ conflicting declaration
          ^^^^^^^^^^^^ declaration of 'sharedMember' conflicts with existing declaration
*/
}

tbuffer ThirdBuffer
{
    int otherMember;
/*CHECK:
        ^^^^^^^^^^^ conflicting declaration
        ^^^^^^^^^^^ declaration of 'otherMember' conflicts with existing declaration
*/
}

// A declaration that conflicts with several earlier ones gets one diagnostic, against the
// nearest of them.

cbuffer FirstOfTwo
{
    int afterTwoBuffers;
/*CHECK:
        ^^^^^^^^^^^^^^^ see previous declaration of 'afterTwoBuffers'
*/
}

cbuffer SecondOfTwo
{
    int afterTwoBuffers;
/*CHECK:
        ^^^^^^^^^^^^^^^ conflicting declaration
        ^^^^^^^^^^^^^^^ declaration of 'afterTwoBuffers' conflicts with existing declaration
        ^^^^^^^^^^^^^^^ see previous declaration of 'afterTwoBuffers'
*/
}

static const int afterTwoBuffers = 1;
/*CHECK:
                 ^^^^^^^^^^^^^^^ conflicting declaration
                 ^^^^^^^^^^^^^^^ declaration of 'afterTwoBuffers' conflicts with existing declaration
*/

static const int aroundBuffer = 1;
/*CHECK:
                 ^^^^^^^^^^^^ see previous declaration of 'aroundBuffer'
                 ^^^^^^^^^^^^ see previous declaration of 'aroundBuffer'
*/

cbuffer BufferBetweenConsts
{
    int aroundBuffer;
/*CHECK:
        ^^^^^^^^^^^^ conflicting declaration
        ^^^^^^^^^^^^ declaration of 'aroundBuffer' conflicts with existing declaration
*/
}

static const int aroundBuffer = 2;
/*CHECK:
                 ^^^^^^^^^^^^ conflicting declaration
                 ^^^^^^^^^^^^ declaration of 'aroundBuffer' conflicts with existing declaration
*/

// Functions and types do not overload with buffer members.

cbuffer BufferWithOtherKinds
{
    int funcName;
/*CHECK:
        ^^^^^^^^ see previous declaration of 'funcName'
*/
    int typeName;
/*CHECK:
        ^^^^^^^^ see previous declaration of 'typeName'
*/
}

int funcName()
/*CHECK:
    ^^^^^^^^ conflicting declaration
    ^^^^^^^^ declaration of 'funcName' conflicts with existing declaration
*/
{
    return 0;
}

struct typeName
/*CHECK:
       ^^^^^^^^ conflicting declaration
       ^^^^^^^^ declaration of 'typeName' conflicts with existing declaration
*/
{
}

// These declarations are in different scopes, so they do not conflict with the members.

cbuffer BufferForScopes
{
    int scopedName;
    int funcLocal;
}

struct StructWithField
{
    int scopedName;
}

namespace Namespace
{
static const int scopedName = 1;
}

int useFuncLocal()
{
    int funcLocal = 1;
    return funcLocal;
}

// Attributes are looked up separately from other declarations, so the attribute `Tag`
// that `TagAttribute` declares does not conflict with the member.

cbuffer BufferWithAttributeName
{
    int Tag;
}

[__AttributeUsage(_AttributeTargets.Function)]
struct TagAttribute
{
    int value;
}

[Tag(1)]
int useTag()
{
    return Tag;
}

// A buffer with an instance name is not transparent, so its members do not conflict.

cbuffer NamedBuffer
{
    int namedMember;
} namedBufferInstance;

static const int namedMember = 1;

// Function overloads are still allowed next to buffers.

int overloaded(int x)
{
    return x;
}

int overloaded(float x)
{
    return int(x);
}

cbuffer BufferNextToOverloads
{
    int notOverloaded;
}
