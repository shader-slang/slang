// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <cstring>

using namespace Slang;

SLANG_UNIT_TEST(differentialPairReflection)
{
    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef())));
    slang::TargetDesc target = {};
    target.format = SLANG_HLSL;
    target.profile = globalSession->findProfile("sm_6_6");
    slang::SessionDesc desc = {};
    desc.targets = &target;
    desc.targetCount = 1;
    ComPtr<slang::ISession> session;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(globalSession->createSession(desc, session.writeRef())));

    ComPtr<slang::IBlob> diagnostics;
    auto module = session->loadModuleFromSourceString(
        "differentialPairReflection",
        "differentialPairReflection.slang",
        R"(
        struct Gradient : IDifferentiablePtrType
        {
            typealias Differential = Gradient;
            no_diff RWStructuredBuffer<float> buffer;
        }
        struct Primal : IDifferentiablePtrType
        {
            typealias Differential = Gradient;
            no_diff RWStructuredBuffer<float> buffer;
            no_diff uint index;
        }
        struct Cases
        {
            DifferentialPair<float> scalar;
            DifferentialPair<DifferentialPair<float>> nested;
            DifferentialPtrPair<Primal> pointers;
        }
        )",
        diagnostics.writeRef());
    SLANG_CHECK_ABORT(module);
    auto program = module->getLayout();
    SLANG_CHECK_ABORT(program);
    auto cases = program->findTypeByName("Cases");
    SLANG_CHECK_ABORT(cases && cases->getFieldCount() == 3);

    for (unsigned i = 0; i < cases->getFieldCount(); ++i)
    {
        auto pair = cases->getFieldByIndex(i)->getType();
        SLANG_CHECK_ABORT(pair->getKind() == slang::TypeReflection::Kind::Struct);
        SLANG_CHECK_ABORT(pair->getFieldCount() == 2);
        SLANG_CHECK(std::strcmp(pair->getFieldByIndex(0)->getName(), "primal") == 0);
        SLANG_CHECK(std::strcmp(pair->getFieldByIndex(1)->getName(), "differential") == 0);

        auto layout = program->getTypeLayout(pair);
        SLANG_CHECK_ABORT(layout && layout->getFieldCount() == 2);
        SLANG_CHECK(layout->findFieldIndexByName("primal") == 0);
        SLANG_CHECK(layout->findFieldIndexByName("differential") == 1);
        SLANG_CHECK(layout->findFieldIndexByName("missing") == -1);
        if (i == 0)
        {
            SLANG_CHECK(layout->getSize() == 8);
            SLANG_CHECK(layout->getFieldByIndex(0)->getOffset() == 0);
            SLANG_CHECK(layout->getFieldByIndex(1)->getOffset() == 4);
            SLANG_CHECK(
                pair->getFieldByIndex(1)->getType()->getScalarType() ==
                slang::TypeReflection::ScalarType::Float32);
        }
        else if (i == 1)
        {
            SLANG_CHECK(pair->getFieldByIndex(0)->getType()->getFieldCount() == 2);
            // HLSL constant-buffer layout starts each nested struct on a 16-byte boundary.
            SLANG_CHECK(layout->getSize() == 24);
            SLANG_CHECK(layout->getFieldByIndex(1)->getOffset() == 16);
        }
        else
        {
            SLANG_CHECK(std::strcmp(pair->getFieldByIndex(0)->getType()->getName(), "Primal") == 0);
            SLANG_CHECK(
                std::strcmp(pair->getFieldByIndex(1)->getType()->getName(), "Gradient") == 0);
            SLANG_CHECK(layout->getSize(SLANG_PARAMETER_CATEGORY_UNORDERED_ACCESS) == 2);
            SLANG_CHECK(
                layout->getFieldByIndex(1)->getOffset(SLANG_PARAMETER_CATEGORY_UNORDERED_ACCESS) ==
                1);
        }
    }
}
