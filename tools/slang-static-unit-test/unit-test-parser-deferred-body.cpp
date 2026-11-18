// Parser-created AST nodes retain tokens from their input. These tests inspect
// an expression returned after its temporary input is released, and deferred
// function bodies before semantic checking replaces an `UnparsedStmt` with a parsed body.
// We also exercise generic-argument speculation through the expression parser, where a copied
// token reader must keep a split `>>` local to the speculative parse.

#include "slang-com-ptr.h"
#include "slang/slang-ast-builder.h"
#include "slang/slang-compile-request.h"
#include "slang/slang-parser.h"
#include "slang/slang-session.h"
#include "slang/slang-translation-unit.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

namespace
{

/// Check that an unchecked expression names `expectedName` at `expectedLoc`.
void checkVarExpr(Expr* expression, const char* expectedName, SourceLoc expectedLoc)
{
    auto variable = as<VarExpr>(expression);
    SLANG_CHECK_ABORT(variable);
    SLANG_CHECK_ABORT(variable->name);
    SLANG_CHECK(variable->name->text == expectedName);
    SLANG_CHECK(variable->loc == expectedLoc);
}

} // namespace

/// Checks that expression tokens use the AST builder's pool after temporary input is released.
SLANG_UNIT_TEST(parserTermStringUsesBuilderTokenPool)
{
    slang::SessionDesc sessionDesc = {};
    ComPtr<slang::ISession> session;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        unitTestContext->slangGlobalSession->createSession(sessionDesc, session.writeRef())));
    auto linkage = static_cast<Linkage*>(session.get());
    auto astBuilder = linkage->getASTBuilder();
    SLANG_AST_BUILDER_RAII(astBuilder);

    // `parseTermString` releases its temporary source manager before returning.
    // The returned expression's token must use the builder's pool, so its spelling
    // remains available for the same lifetime as the expression.
    auto scope = astBuilder->create<Scope>();
    auto expression = linkage->parseTermString("123", scope);
    auto literal = as<IntegerLiteralExpr>(expression);
    SLANG_CHECK_ABORT(literal);
    SLANG_CHECK(literal->token.getPool() == astBuilder->getTokenPool());
    SLANG_CHECK(literal->token.is<IntegerLiteralToken>());
    SLANG_CHECK(literal->token.getPhysicalSpelling() == UnownedStringSlice("123"));
    SLANG_CHECK(literal->token.getLogicalSpelling() == UnownedStringSlice("123"));
}

/// Checks that an incomplete deferred body retains the physical EOF occurrence and its trivia.
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
    lexer.initialize(view, &sink, linkage->getNamePool(), astBuilder->getTokenPool());
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
    SLANG_CHECK(
        capturedEOF.getLeadingTrivia()[1].getPhysicalSpelling() == UnownedStringSlice("//last"));
    SLANG_CHECK(
        capturedEOF.getLeadingTrivia()[1].getLoc() == actualEOF.getLeadingTrivia()[1].getLoc());
    SLANG_CHECK(!translationUnit->isChecked);
}

/// Checks that a balanced deferred body ends with a trivia-free EOF at the next declaration.
SLANG_UNIT_TEST(parserBalancedDeferredBodyEndsBeforeNextDeclarationTrivia)
{
    // We parse two bodies without checking them. The first body's synthetic EOF belongs at
    // the next `void`, but the intervening comment must remain attached to that declaration.
    const auto source = UnownedStringSlice("void f() {}\n\n// next\nvoid g() {}");
    slang::SessionDesc sessionDesc = {};
    ComPtr<slang::ISession> session;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        unitTestContext->slangGlobalSession->createSession(sessionDesc, session.writeRef())));
    auto linkage = static_cast<Linkage*>(session.get());
    auto astBuilder = linkage->getASTBuilder();
    SLANG_AST_BUILDER_RAII(astBuilder);

    // We use the translation unit's language scope so declaration parsing recognizes `void`.
    auto sourceManager = linkage->getSourceManager();
    DiagnosticSink sink(sourceManager, nullptr);
    FrontEndCompileRequest request(linkage, nullptr, &sink);
    auto moduleName = linkage->getNamePool()->getName("balanced_deferred_body");
    auto unitIndex = request.addTranslationUnit(SourceLanguage::Slang, moduleName);
    auto translationUnit = request.getTranslationUnit(unitIndex);
    auto moduleDecl = astBuilder->create<ModuleDecl>();
    moduleDecl->nameAndLoc.name = moduleName;
    translationUnit->module->setModuleDecl(moduleDecl);

    // Direct lexing lets us compare the captured endpoint with the untouched continuation token.
    auto file = sourceManager->createSourceFileWithString(
        PathInfo::makeFromString("balanced_deferred_body.slang"),
        source);
    auto view = sourceManager->createSourceView(file, nullptr, SourceLoc());
    Lexer lexer;
    lexer.initialize(view, &sink, linkage->getNamePool(), astBuilder->getTokenPool());
    auto tokenList = lexer.readAllTokens();
    const auto& tokens = tokenList.getTokens();
    SLANG_CHECK_ABORT(tokens.getCount() == 13);
    auto continuation = tokens[6];
    SLANG_CHECK(continuation.getLogicalSpelling() == UnownedStringSlice("void"));
    SLANG_CHECK_ABORT(continuation.getLeadingTrivia().getCount() == 3);
    SLANG_CHECK(continuation.getLeadingTrivia()[0].is<NewLineTrivia>());
    SLANG_CHECK(continuation.getLeadingTrivia()[1].is<LineCommentTrivia>());
    SLANG_CHECK(continuation.getLeadingTrivia()[2].is<NewLineTrivia>());
    SLANG_CHECK(
        continuation.getLeadingTrivia()[1].getPhysicalSpelling() == UnownedStringSlice("// next"));

    parseSourceFile(
        astBuilder,
        translationUnit,
        SourceLanguage::Slang,
        TokenSpan(tokenList),
        &sink,
        translationUnit->getLanguageScope(),
        moduleDecl);
    SLANG_CHECK(sink.getErrorCount() == 0);

    // Both functions must be present; retaining the continuation position must not consume it.
    List<FuncDecl*> functions;
    for (auto function : moduleDecl->getDirectMemberDeclsOfType<FuncDecl>())
        functions.add(function);
    SLANG_CHECK_ABORT(functions.getCount() == 2);
    SLANG_CHECK(functions[0]->getName()->text == "f");
    SLANG_CHECK(functions[1]->getName()->text == "g");
    SLANG_CHECK(functions[1]->loc == tokens[7].getLoc());

    // Only braces and a trivia-free EOF belong to the first captured body. The EOF's location
    // is the parser's continuation position, rather than the closing brace or physical file EOF.
    auto body = as<UnparsedStmt>(functions[0]->body);
    SLANG_CHECK_ABORT(body);
    SLANG_CHECK_ABORT(body->tokens.getCount() == 3);
    SLANG_CHECK(body->tokens[0].is<LBraceToken>());
    SLANG_CHECK(body->tokens[1].is<RBraceToken>());
    SLANG_CHECK(body->tokens[0].getObject() == tokens[4].getObject());
    SLANG_CHECK(body->tokens[1].getObject() == tokens[5].getObject());
    SLANG_CHECK(body->tokens[0].getLoc() == tokens[4].getLoc());
    SLANG_CHECK(body->tokens[1].getLoc() == tokens[5].getLoc());
    auto capturedEOF = body->tokens.getLast();
    SLANG_CHECK(capturedEOF.is<EndOfFileToken>());
    SLANG_CHECK(capturedEOF.getLoc() == continuation.getLoc());
    SLANG_CHECK(capturedEOF.getLoc() != tokens.getLast().getLoc());
    SLANG_CHECK(capturedEOF.getPool() == astBuilder->getTokenPool());
    SLANG_CHECK(capturedEOF.getPhysicalSpelling().getLength() == 0);
    SLANG_CHECK(capturedEOF.getLogicalSpelling().getLength() == 0);
    SLANG_CHECK(capturedEOF.getLeadingTrivia().getCount() == 0);
    SLANG_CHECK(capturedEOF.getTrailingTrivia().getCount() == 0);
    SLANG_CHECK(tokens[6].getObject() == continuation.getObject());
    SLANG_CHECK(as<UnparsedStmt>(functions[1]->body) != nullptr);
    SLANG_CHECK(!translationUnit->isChecked);
}

/// Checks that failed generic speculation restores a split `>>` and the remaining expression.
SLANG_UNIT_TEST(parserGenericSpeculationRestoresSplitToken)
{
    // With no declarations in scope, `tryParseGenericApp` speculates that `a` is generic.
    // Parsing `a<b<c>>` splits `>>` for the inner application, then consumes both closers.
    // The following `+` is outside the accepted FOLLOW set, so that outer speculation fails.
    const auto source = UnownedStringSlice("a<b<c>\\\n> /*tail*/ + d");
    slang::SessionDesc sessionDesc = {};
    ComPtr<slang::ISession> session;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        unitTestContext->slangGlobalSession->createSession(sessionDesc, session.writeRef())));
    auto linkage = static_cast<Linkage*>(session.get());
    auto astBuilder = linkage->getASTBuilder();
    SLANG_AST_BUILDER_RAII(astBuilder);

    auto sourceManager = linkage->getSourceManager();
    DiagnosticSink sink(sourceManager, nullptr);
    auto file = sourceManager->createSourceFileWithString(
        PathInfo::makeFromString("generic_speculation.slang"),
        source);
    auto view = sourceManager->createSourceView(file, nullptr, SourceLoc());
    Lexer lexer;
    lexer.initialize(view, &sink, linkage->getNamePool(), astBuilder->getTokenPool());
    auto tokenList = lexer.readAllTokens();
    const auto& tokens = tokenList.getTokens();
    SLANG_CHECK_ABORT(tokens.getCount() == 9);
    auto rightShift = tokens[5];
    SLANG_CHECK(rightShift.is<OpRshToken>());
    SLANG_CHECK(rightShift.getPhysicalSpelling() == UnownedStringSlice(">\\\n>"));
    SLANG_CHECK(rightShift.getLogicalSpelling() == UnownedStringSlice(">>"));
    SLANG_CHECK_ABORT(rightShift.getTrailingTrivia().getCount() == 3);
    SLANG_CHECK(rightShift.getTrailingTrivia()[1].is<BlockCommentTrivia>());

    auto scope = astBuilder->create<Scope>();
    auto expression = parseTermFromSourceFile(
        astBuilder,
        TokenSpan(tokenList),
        &sink,
        scope,
        linkage->getNamePool(),
        SourceLanguage::Slang);
    SLANG_CHECK(sink.getErrorCount() == 0);

    // After rollback, the expression is `(a < b<c>) > (+d)`: the inner `b<c>` is accepted
    // because its following `>` is in the FOLLOW set. We inspect that complete tree so a
    // corrupted reader cannot pass merely by returning a non-null expression.
    auto greater = as<InfixExpr>(expression);
    SLANG_CHECK_ABORT(greater);
    SLANG_CHECK_ABORT(greater->arguments.getCount() == 2);
    auto secondGreaterLoc = view->getRange().getSourceLocFromOffset(8);
    checkVarExpr(greater->functionExpr, ">", secondGreaterLoc);
    SLANG_CHECK(greater->loc == secondGreaterLoc);

    auto less = as<InfixExpr>(greater->arguments[0]);
    SLANG_CHECK_ABORT(less);
    SLANG_CHECK_ABORT(less->arguments.getCount() == 2);
    checkVarExpr(less->functionExpr, "<", tokens[1].getLoc());
    checkVarExpr(less->arguments[0], "a", tokens[0].getLoc());

    auto generic = as<GenericAppExpr>(less->arguments[1]);
    SLANG_CHECK_ABORT(generic);
    checkVarExpr(generic->functionExpr, "b", tokens[2].getLoc());
    SLANG_CHECK_ABORT(generic->arguments.getCount() == 1);
    checkVarExpr(generic->arguments[0], "c", tokens[4].getLoc());
    SLANG_CHECK_ABORT(generic->argumentDelimeterLocs.getCount() == 2);
    SLANG_CHECK(generic->argumentDelimeterLocs[0] == tokens[3].getLoc());
    SLANG_CHECK(generic->argumentDelimeterLocs[1] == rightShift.getLoc());

    // The token after the original `>>` must still be `+`, and the parser must reach `d`.
    auto positive = as<PrefixExpr>(greater->arguments[1]);
    SLANG_CHECK_ABORT(positive);
    SLANG_CHECK_ABORT(positive->arguments.getCount() == 1);
    checkVarExpr(positive->functionExpr, "+", tokens[6].getLoc());
    checkVarExpr(positive->arguments[0], "d", tokens[7].getLoc());
    SLANG_CHECK(tokens[5].getObject() == rightShift.getObject());
    SLANG_CHECK(tokens[5].getPhysicalSpelling() == UnownedStringSlice(">\\\n>"));
    SLANG_CHECK(
        tokens[5].getTrailingTrivia()[1].getPhysicalSpelling() == UnownedStringSlice("/*tail*/"));
}
