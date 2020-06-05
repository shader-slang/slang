
#include "internalGroup1.h"
#include "slang.h"
#include <algorithm>

#include "includeGroup2.h"
#include <vector>

#include <vulkan.h>

#ifdef __linux__
#include <X11/Xlib.h>
#else
#include <Windows.h>
#endif

namespace slang
{
/// <summary>
/// Summary of TestClass
/// </summary>
class TestClass
{
private:
    int examplePrivateMember; // Trailing comment1
    int* examplePointerMember = nullptr; // trailing coment2
    float member4;
    float member5;

public:
    TestClass()
        : examplePrivateMember(0)
        , examplePointerMember(nullptr)
        , member4(1.0f)
        , member5(200.0f)
    {}

    virtual void emptyFunction() {}

    int shortFunction() { return 0; }

    /// <summary>
    /// A normal function.
    /// </summary>
    /// <param name="param1">The first parameter.</param>
    /// <param name="param2">The second parameter</param>
    /// <returns></returns>
    int normalFunction(int param1, float* param2)
    {
        for (int i = 0; i < 100; i++) {
            switch (i) {
            case 0:
            {
                return normalFunction(0, nullptr);
            } break;
            case 1:
            {
                float b = *param2;
                i--;
            } break;
            default:
                break;
            }
        }
        if (param1)
            return *param2;
        else
            return -1;
    }
    class LoweredValInfo
    {};

    class IRType
    {};

    class IRInst
    {};

    LoweredValInfo subscriptValue(IRType* type, LoweredValInfo baseVal, IRInst* indexVal)
    {
        auto builder = getBuilder();

        // The `tryGetAddress` operation will take a complex value
        // representation and try to turn it into a single pointer, if
        // possible.
        //
        baseVal = tryGetAddress(context, baseVal, TryGetAddressMode::Aggressive);

        // The `materialize` operation should ensure that we only have to
        // deal with the small number of base cases for lowered value
        // representations.
        //
        baseVal = materialize(context, baseVal);

        switch (baseVal.flavor) {
        case LoweredValInfo::Flavor::Simple:
            return LoweredValInfo::simple(
                builder->emitElementExtract(type, getSimpleVal(context, baseVal), indexVal));

        case LoweredValInfo::Flavor::Ptr:
            return LoweredValInfo::ptr(builder->emitElementAddress(
                context->irBuilder->getPtrType(type), baseVal.val, indexVal));

        default:
            SLANG_UNIMPLEMENTED_X("subscript expr");
            UNREACHABLE_RETURN(LoweredValInfo());
        }
    }

    int longParameterList(
        int param1,
        int param2,
        int param3,
        TestClass* param4,
        float* param5,
        std::pair<int, float> param6)
    {
        if (param1 == 0)
            longParameterList(
                param1,
                param1 + param2 + param3,
                param2 - param1 + (1 << 23) / sizeof(TestClass),
                param4 + (param1) * sizeof(unsigned),
                param6);
    }
};

} // namespace slang
