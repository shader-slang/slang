// Deferred function bodies retain the tokens captured by declaration parsing.
// These tests inspect that intermediate AST before semantic checking replaces
// an `UnparsedStmt` with a parsed body.

#include "slang-com-ptr.h"
#include "slang/slang-ast-builder.h"
#include "slang/slang-compile-request.h"
#include "slang/slang-parser.h"
#include "slang/slang-session.h"
#include "slang/slang-translation-unit.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

SLANG_UNIT_TEST(parserDeferredBodyRetainsTerminatingEOF)
{
    // The missing closing brace makes the file's EOF terminate body capture.
    // The final indentation and comment belong to that EOF's leading trivia.
    const auto source = UnownedStringSlice("void f() {\n //last");
    slang::SessionDesc sessionDesc = {};
    ComPtr<slang::ISession> session;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        unitTestContext->slangGlobalSession->createSession(sessionDesc, session.writeRef())));
    auto linkage = static_cast<Linkage*>(session.get());
    auto astBuilder = linkage->getASTBuilder();
    SLANG_AST_BUILDER_RAII(astBuilder);

    // We use the normal translation-unit scope and parser entry point, but stop
    // before checking so the captured token sequence remains observable.
    auto sourceManager = linkage->getSourceManager();
    DiagnosticSink sink(sourceManager, nullptr);
    FrontEndCompileRequest request(linkage, nullptr, &sink);
    auto moduleName = linkage->getNamePool()->getName("deferred_body_eof");
    auto unitIndex = request.addTranslationUnit(SourceLanguage::Slang, moduleName);
    auto translationUnit = request.getTranslationUnit(unitIndex);
    auto moduleDecl = astBuilder->create<ModuleDecl>();
    moduleDecl->nameAndLoc.name = moduleName;
    translationUnit->module->setModuleDecl(moduleDecl);

    // Direct lexing supplies the physical EOF cluster to declaration parsing.
    // We retain that occurrence as the expectation for body capture.
    auto file = sourceManager->createSourceFileWithString(
        PathInfo::makeFromString("deferred_body_eof.slang"),
        source);
    auto view = sourceManager->createSourceView(file, nullptr, SourceLoc());
    Lexer lexer;
    lexer.initialize(view, &sink, linkage->getNamePool());
    auto tokens = lexer.readAllTokens();
    auto actualEOF = *tokens.end();
    SLANG_CHECK_ABORT(actualEOF.getLeadingTrivia().getCount() == 2);
    SLANG_CHECK(actualEOF.getLeadingTrivia()[0].is<WhiteSpaceTrivia>());
    SLANG_CHECK(actualEOF.getLeadingTrivia()[1].is<LineCommentTrivia>());

    parseSourceFile(
        astBuilder,
        translationUnit,
        SourceLanguage::Slang,
        TokenSpan(tokens),
        &sink,
        translationUnit->getLanguageScope(),
        moduleDecl);

    // Capturing EOF must retain the occurrence, including its physical comment,
    // rather than replacing it with an empty endpoint for a balanced body.
    auto function = moduleDecl->findFirstDirectMemberDeclOfType<FuncDecl>();
    SLANG_CHECK_ABORT(function);
    auto body = as<UnparsedStmt>(function->body);
    SLANG_CHECK_ABORT(body);
    SLANG_CHECK_ABORT(body->tokens.getCount() == 2);
    auto capturedEOF = body->tokens.getLast();
    SLANG_CHECK(capturedEOF.is<EndOfFileToken>());
    SLANG_CHECK(capturedEOF.getObject() == actualEOF.getObject());
    SLANG_CHECK(capturedEOF.getLoc() == actualEOF.getLoc());
    SLANG_CHECK(capturedEOF.getFlags() == actualEOF.getFlags());
    SLANG_CHECK_ABORT(capturedEOF.getLeadingTrivia().getCount() == 2);
    SLANG_CHECK(capturedEOF.getLeadingTrivia()[1].getSpelling() == UnownedStringSlice("//last"));
    SLANG_CHECK(
        capturedEOF.getLeadingTrivia()[1].getLoc() == actualEOF.getLeadingTrivia()[1].getLoc());
    SLANG_CHECK(!translationUnit->isChecked);
}
