// slang-ir-serialize.h
#ifndef SLANG_IR_SERIALIZE_H_INCLUDED
#define SLANG_IR_SERIALIZE_H_INCLUDED

#include "slang-ir-serialize-types.h"

#include "../core/slang-riff.h"

#include "slang-ir.h"

#include "slang-ir-serialize-types.h"

// For TranslationUnitRequest
// and FrontEndCompileRequest::ExtraEntryPointInfo
#include "slang-compiler.h"

namespace Slang {

struct IRSerialWriter
{
    typedef IRSerialData Ser;
    typedef IRSerialBinary Bin;

    struct OptionFlag
    {
        typedef uint32_t Type;
        enum Enum: Type
        {
            RawSourceLocation       = 0x01,
            DebugInfo               = 0x02,
        };
    };
    typedef OptionFlag::Type OptionFlags;

    Result write(IRModule* module, SourceManager* sourceManager, OptionFlags options, IRSerialData* serialData);
  
    static Result writeStream(const IRSerialData& data, IRSerialCompressionType compressionType, Stream* stream);

        /// Write to a container
    static Result writeContainer(const IRSerialData& data, IRSerialCompressionType compressionType, RiffContainer* container);
    
    /// Get an instruction index from an instruction
    Ser::InstIndex getInstIndex(IRInst* inst) const { return inst ? Ser::InstIndex(m_instMap[inst]) : Ser::InstIndex(0); }

        /// Get a slice from an index
    UnownedStringSlice getStringSlice(Ser::StringIndex index) const { return m_stringSlicePool.getSlice(StringSlicePool::Handle(index)); }
        /// Get index from string representations
    Ser::StringIndex getStringIndex(StringRepresentation* string) { return Ser::StringIndex(m_stringSlicePool.add(string)); }
    Ser::StringIndex getStringIndex(const UnownedStringSlice& slice) { return Ser::StringIndex(m_stringSlicePool.add(slice)); }
    Ser::StringIndex getStringIndex(Name* name) { return name ? getStringIndex(name->text) : Ser::kNullStringIndex; }
    Ser::StringIndex getStringIndex(const char* chars) { return Ser::StringIndex(m_stringSlicePool.add(chars)); }
    Ser::StringIndex getStringIndex(const String& string) { return Ser::StringIndex(m_stringSlicePool.add(string.getUnownedSlice())); }

    StringSlicePool& getStringPool() { return m_stringSlicePool;  }
    StringSlicePool& getDebugStringPool() { return m_debugStringSlicePool; }

    IRSerialWriter() :
        m_serialData(nullptr),
        m_stringSlicePool(StringSlicePool::Style::Default),
        m_debugStringSlicePool(StringSlicePool::Style::Default)
    {}

protected:
    class DebugSourceFile : public RefObject
    {
    public:
        DebugSourceFile(SourceFile* sourceFile, SourceLoc::RawValue baseSourceLoc):
            m_sourceFile(sourceFile),
            m_baseSourceLoc(baseSourceLoc)
        {
            // Need to know how many lines there are
            const List<uint32_t>& lineOffsets = sourceFile->getLineBreakOffsets();

            const auto numLineIndices = lineOffsets.getCount();

            // Set none as being used initially
            m_lineIndexUsed.setCount(numLineIndices);
            ::memset(m_lineIndexUsed.begin(), 0, numLineIndices * sizeof(uint8_t));
        }
            /// True if we have information on that line index
        bool hasLineIndex(int lineIndex) const { return m_lineIndexUsed[lineIndex] != 0; }
        void setHasLineIndex(int lineIndex) { m_lineIndexUsed[lineIndex] = 1; }

        SourceLoc::RawValue m_baseSourceLoc;            ///< The base source location

        SourceFile* m_sourceFile;                       ///< The source file
        List<uint8_t> m_lineIndexUsed;                  ///< Has 1 if the line is used
        List<uint32_t> m_usedLineIndices;               ///< Holds the lines that have been hit                 

        List<IRSerialData::DebugLineInfo> m_lineInfos;   ///< The line infos
        List<IRSerialData::DebugAdjustedLineInfo> m_adjustedLineInfos;  ///< The adjusted line infos
    };

    void _addInstruction(IRInst* inst);
    Result _calcDebugInfo();
        /// Returns the remapped sourceLoc, or 0 if sourceLoc couldn't be added
    void _addDebugSourceLocRun(SourceLoc sourceLoc, uint32_t startInstIndex, uint32_t numInst);

    List<IRInst*> m_insts;                              ///< Instructions in same order as stored in the 

    List<IRDecoration*> m_decorations;                  ///< Holds all decorations in order of the instructions as found
    List<IRInst*> m_instWithFirstDecoration;            ///< All decorations are held in this order after all the regular instructions

    Dictionary<IRInst*, Ser::InstIndex> m_instMap;      ///< Map an instruction to an instruction index

    StringSlicePool m_stringSlicePool;    
    IRSerialData* m_serialData;                         ///< Where the data is stored

    StringSlicePool m_debugStringSlicePool;             ///< Slices held just for debug usage

    SourceLoc::RawValue m_debugFreeSourceLoc;           /// Locations greater than this are free
    Dictionary<SourceFile*, RefPtr<DebugSourceFile> > m_debugSourceFileMap;
    
    SourceManager* m_sourceManager;                     ///< The source manager
};

struct IRSerialReader
{
    typedef IRSerialData Ser;
    typedef StringRepresentationCache::Handle StringHandle;

        /// Read a stream to fill in dataOut IRSerialData
    static Result readStream(Stream* stream, IRSerialData* dataOut);

        /// Read potentially multiple modules from a stream
    static Result readStreamModules(Stream* stream, Session* session, SourceManager* manager, List<RefPtr<IRModule>>& outModules, List<FrontEndCompileRequest::ExtraEntryPointInfo>& outEntryPoints);

        /// Read a stream to fill in dataOut IRSerialData
    static Result readContainer(RiffContainer::ListChunk* module, IRSerialData* outData);

        /// Read a module from serial data
    Result read(const IRSerialData& data, Session* session, SourceManager* sourceManager, RefPtr<IRModule>& moduleOut);

    IRSerialReader():
        m_serialData(nullptr),
        m_module(nullptr)
    {
    }

    protected:

    StringRepresentationCache m_stringRepresentationCache;

    const IRSerialData* m_serialData;
    IRModule* m_module;
};

struct IRSerialUtil
{
        /// Produces an instruction list which is in same order as written through IRSerialWriter
    static void calcInstructionList(IRModule* module, List<IRInst*>& instsOut);

        /// Verify serialization
    static SlangResult verifySerialize(IRModule* module, Session* session, SourceManager* sourceManager, IRSerialCompressionType compressionType, IRSerialWriter::OptionFlags optionFlags);
};


} // namespace Slang

#endif
