// unit-test-check-conversion.cpp
//
// Tests the semantic checker's internal conversion contracts.

#include "slang/slang-ast-builder.h"
#include "slang/slang-check-impl.h"
#include "slang/slang-module.h"
#include "slang/slang-syntax.h"
#include "static-unit-test-env.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

// A caller can request the source access performed by a conversion without providing a diagnostic
// sink. The conversion must still report that converting `nullptr` to a pointer does not evaluate
// the source value.
SLANG_UNIT_TEST(coercionSourceAccessDoesNotDependOnDiagnosticSink)
{
    StaticUnitTestEnv env(unitTestContext);

    Module* module = env.checkModuleFromSource(
        "coercionSourceAccessDoesNotDependOnDiagnosticSink",
        "void anchor() {}\n");
    SLANG_CHECK_ABORT(module != nullptr);

    SharedSemanticsContext shared(module->getLinkage(), module, nullptr);
    SemanticsVisitor visitor(&shared);
    auto astBuilder = env.getASTBuilder();

    auto sourceExpr = astBuilder->create<NullPtrLiteralExpr>();
    sourceExpr->type = QualType(astBuilder->getNullPtrType());
    sourceExpr->checked = true;
    auto targetType = astBuilder->getPtrType(
        astBuilder->getIntType(),
        AccessQualifier::ReadWrite,
        AddressSpace::Generic,
        astBuilder->getDefaultLayoutType());

    CoercionSourceAccess sourceAccess;
    auto resultExpr = visitor.coerce(
        CoercionSite::General,
        targetType,
        sourceExpr,
        nullptr,
        CoercionSourceAccessCheck::Skip,
        &sourceAccess);

    SLANG_CHECK_ABORT(resultExpr != nullptr);
    SLANG_CHECK(!sourceAccess.needsFallbackCheck());
    SLANG_CHECK(!sourceAccess.hasAccess());
}
