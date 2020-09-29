// slang-serialize-ast.h
#ifndef SLANG_SERIALIZE_AST_H
#define SLANG_SERIALIZE_AST_H

#include "slang-ast-support-types.h"
#include "slang-ast-all.h"

#include "../core/slang-riff.h"

#include "slang-ast-builder.h"

#include "slang-serialize.h"

namespace Slang
{

/* Holds RIFF FourCC codes for AST types */
struct ASTSerialBinary
{
    static const FourCC kRiffFourCC = RiffFourCC::kRiff;

        /// AST module LIST container
    static const FourCC kSlangASTModuleFourCC = SLANG_FOUR_CC('S', 'A', 'm', 'l');
        /// AST module data 
    static const FourCC kSlangASTModuleDataFourCC = SLANG_FOUR_CC('S', 'A', 'm', 'd');
};

class ModuleSerialFilter : public SerialFilter
{
public:
    // SerialFilter impl
    virtual SerialIndex writePointer(SerialWriter* writer, const NodeBase* ptr) SLANG_OVERRIDE;

    ModuleSerialFilter(ModuleDecl* moduleDecl):
        m_moduleDecl(moduleDecl)
    {
    }
    protected:
    ModuleDecl* m_moduleDecl;
};

class DefaultSerialObjectFactory : public SerialObjectFactory
{
public:

    virtual void* create(SerialTypeKind typeKind, SerialSubType subType) SLANG_OVERRIDE;

    DefaultSerialObjectFactory(ASTBuilder* astBuilder) :
        m_astBuilder(astBuilder)
    {
    }

protected:
    RefObject* _add(RefObject* obj)
    {
        m_scope.add(obj);
        return obj;
    }

    // We keep RefObjects in scope 
    List<RefPtr<RefObject>> m_scope;
    ASTBuilder* m_astBuilder;
};

/* None of the functions in this util should *not* be called from production code,
they exist to test features of AST Serialization */
struct ASTSerialTestUtil
{
    static SlangResult selfTest();

        /// Tries to serialize out, read back in and test the results are the same.
        /// Will write dumped out node to files 
    static SlangResult testSerialize(NodeBase* node, RootNamePool* rootNamePool, SharedASTBuilder* sharedASTBuilder, SourceManager* sourceManager);
};

} // namespace Slang

#endif
