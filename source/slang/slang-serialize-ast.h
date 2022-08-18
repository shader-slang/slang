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
    virtual SerialIndex writePointer(SerialWriter* writer, const RefObject* ptr) SLANG_OVERRIDE;

    ModuleSerialFilter(ModuleDecl* moduleDecl):
        m_moduleDecl(moduleDecl)
    {
    }
    protected:
    ModuleDecl* m_moduleDecl;
};

struct ASTSerialUtil
{
        /// Add the AST related classes
    static void addSerialClasses(SerialClasses* classes);

        /// Tries to serialize out, read back in and test the results are the same.
        /// Will write dumped out node to files 
    static SlangResult testSerialize(NodeBase* node, RootNamePool* rootNamePool, SharedASTBuilder* sharedASTBuilder, SourceManager* sourceManager);

    static List<uint8_t> serializeAST(ModuleDecl* moduleDecl);
};

} // namespace Slang

#endif
