// slang-emit-spirv.cpp

#include "slang-compiler.h"
#include "slang-emit-base.h"

#include "slang-ir.h"
#include "slang-ir-insts.h"
#include "slang-ir-layout.h"
#include "slang-ir-spirv-snippet.h"
#include "slang-ir-spirv-legalize.h"
#include "spirv/unified1/spirv.h"
#include "../core/slang-memory-arena.h"

namespace Slang
{

// Our goal in this file is to convert a module in the Slang IR over to an
// equivalent module in the SPIR-V intermediate language.
//
// The Slang IR is (intentionally) similar to SPIR-V in many ways, and both
// can represent shaders at similar levels of abstraction, so much of the
// translation involves one-to-one translation of Slang IR instructions
// to their SPIR-V equivalents.
//
// SPIR-V differs from Slang IR in some key ways, and the SPIR-V
// specification places many restrictions on how the IR can be encoded.
// In some cases we will rely on earlier IR passes to convert Slang IR
// into a form closer to what SPIR-V expects (e.g., by moving all
// varying entry point parameters to global scope), but other differences
// will be handled during the translation process.
//
// The logic in this file relies on the formal [SPIR-V Specification].
// When we are making use of or enforcing some property from the spec,
// we will try to refer to the relevant section in comments.
//
// [SPIR-V Specification]: https://www.khronos.org/registry/spir-v/specs/unified1/SPIRV.html

// [2.3: Physical Layout of a SPIR-V Module and Instruction]
//
// > A SPIR-V module is a single linear stream of words.


// [2.3: Physical Layout of a SPIR-V Module and Instruction]
//
// > All remaining words are a linear sequence of instructions.
// > Each instruction is a stream of words
//
// After a fixed-size header, the contents of a SPIR-V module
// is just a flat sequence of instructions, each of which is
// just a sequence of words.
//
// In principle we could try to emit instructions directly
// in one pass as a stream of words, but there are additional
// constraints placed by the SPIR-V encoding that would make
// a single-pass strategy very hard, so we don't attempt it.
//
// [2.4 Logical Layout of a Module]
//
// SPIR-V imposes some global ordering constraints on instructions,
// such that certain instructions must come before or after others.
// For example, all `OpCapability` instructions must come before any
// `OpEntryPoint` instructions.
//
// While the SPIR-V spec doesn't use such a term, we will take
// the enumeration of the ordering in Section 2.4 and use it to
// define a list of *logical sections* that make up a SPIR-V module.

    /// Logical sections of a SPIR-V module.
enum class SpvLogicalSectionID
{
    Capabilities,
    Extensions,
    ExtIntInstImports,
    MemoryModel,
    EntryPoints,
    ExecutionModes,
    DebugStringsAndSource,
    DebugNames,
    Annotations,
    Types,
    Constants,
    GlobalVariables,
    FunctionDeclarations,
    FunctionDefinitions,

    Count,
};

// While the SPIR-V module is nominally (according to the spec) just
// a flat sequence of instructions, in practice some of the instructions
// are logically in a parent/child relationship.
//
// In particular, functions "own" the instructions between an `OpFunction`
// and the matching `OpFunctionEnd`. We can also think of basic
// blocks within a function as owning the instructions between
// an `OpLabel` (which represents the bloc) and the next label
// or the end of the function.
//
// Furthermore, the common case is SPIR-V is that an instruction
// that defines some value must appear before any instruction
// that uses that value as an operand. This property is often true
// in a Slang IR module, but isn't strictly enforced for things at
// the global scope.
//
// To deal with the above issues, our strategy will be to emit
// SPIR-V instructions into a lightweight intermediate structure
// that simplifies dealing with ordering constraiints on
// instructions.
//
// We will start by forward-declaring the type we will
// use to represent instructions:
//
struct SpvInst;

// Next, we will define a base type that can serve as a parent
// to SPIR-V instructions. Both the logical sections defined
// earlier and instructions such as functions will be used
// as parents.

    /// Base type for SPIR-V instructions and logical sections of a module
    ///
    /// Holds and supports appending to a list of child instructions.
struct SpvInstParent
{
public:
        /// Add an instruction to the end of the list of children
    void addInst(SpvInst* inst);

        /// Dump all children, recursively, to a flattened list of SPIR-V words
    void dumpTo(List<SpvWord>& ioWords);

private:
        /// The first child, if any.
    SpvInst* m_firstChild = nullptr;

        /// A pointer to the null pointer at the end of the linked list.
        ///
        /// If the list of children is empty this points to `m_firstChild`,
        /// while if it is non-empty it points to the `nextSibling` field
        /// of the last instruction.
        ///
    SpvInst** m_link = &m_firstChild;
};

// A SPIR-V instruction is then (in the general case) a potential
// parent to other instructions.

    /// A type to represent a SPIR-V instruction to be emitted.
    ///
    /// This type alows the instruction to be built up across
    /// multiple steps in a mutable fashion.
    ///
struct SpvInst : SpvInstParent
{
    // [2.3: Physical Layout of a SPIR-V Module and Instruction]
    //
    // > Each instruction is a stream of words
    //
    // > Opcode: The 16 high-order bits are the WordCount of the instruction.
    // >         The 16 low-order bits are the opcode enumerant.
    //
    // We will store the "opcode enumerant" directly in our
    // intermediate structure, and compute the word count on
    // the fly when writing an instruction to an output buffer.

        /// The SPIR-V opcode for the instruction
    SpvOp opcode;

    // [2.3: Physical Layout of a SPIR-V Module and Instruction]
    //
    // > Optional instruction type <id> (presence determined by opcode)
    // > Optional instruction Result <id> (presence determined by opcode)
    // > Operand 1 (if needed)
    // > Operand 2 (if needed)
    // > ...
    //
    // We represent the remaining words of the instruction (after
    // the opcode word) as an undifferentiated array. Any code
    // that encodes an instruction is responsible for knowing the
    // opcode-specific data that is required.
    //
    // Our code does not need to process instruction operands after
    // they have been written into a `SpvInst`. If we ever had
    // cases where we needed to do post-processing, then we would
    // need to store a more refined representation here.

        /// The additional words of the instruction after the opcode
    SpvWord* operandWords = nullptr;
        /// The amount of operand words
    uint32_t operandWordsCount = 0;

    // We will store the instructions in a given `SpvInstParent`
    // using an intrusive linked list.

        /// The next instruction in the same `SpvInstParent`
    SpvInst* nextSibling = nullptr;

        /// The result <id> produced by this instruction, or zero if it has no result.
    SpvWord id = 0;

        /// Dump the instruction (and any children, recursively) into the flat array of SPIR-V words.
    void dumpTo(List<SpvWord>& ioWords)
    {
        // [2.2: Terms]
        //
        // > Word Count: The complete number of words taken by an instruction,
        // > including the word holding the word count and opcode, and any optional
        // > operands. An instruction’s word count is the total space taken by the instruction.
        //
        SpvWord wordCount = 1 + SpvWord(operandWordsCount);

        // [2.3: Physical Layout of a SPIR-V Module and Instruction]
        //
        // > Opcode: The 16 high-order bits are the WordCount of the instruction.
        // >         The 16 low-order bits are the opcode enumerant.
        //
        ioWords.add(wordCount << 16 | opcode);

        // The operand words simply follow the opcode word.
        //
        ioWords.addRange(operandWords, operandWordsCount);
        
        // In our representation choice, the children of a
        // parent instruction will always follow the encoded
        // words of a parent:
        //
        // * The instructions inside a function always follow the `OpFunction`
        // * The instructions inside a block always follow the `OpLabel`
        //
        SpvInstParent::dumpTo(ioWords);
    }
};

    /// A logical section of a SPIR-V module
struct SpvLogicalSection : SpvInstParent
{
};

// Now that we've filled in the definition of `SpvInst`, we can
// go back and define the key operations on `SpvInstParent`.

void SpvInstParent::addInst(SpvInst* inst)
{
    SLANG_ASSERT(inst);

    // The user shouldn't be trying to add multiple instructions at once.
    // If they really want that then they probably wanted to give `inst`
    // some children.
    //
    SLANG_ASSERT(!inst->nextSibling);

    *m_link = inst;
    m_link = &inst->nextSibling;
}

void SpvInstParent::dumpTo(List<SpvWord>& ioWords)
{
    for( auto child = m_firstChild; child; child = child->nextSibling )
    {
        child->dumpTo(ioWords);
    }
}

/// The context for inlining a SPV assembly snippet.
struct SpvSnippetEmitContext
{
    SpvInst* resultType;
    IRType* irResultType;
    // True if resultType is float or vector of float.
    bool isResultTypeFloat;
    // True if resultType is signed.
    bool isResultTypeSigned;
    Dictionary<SpvStorageClass, IRInst*> qualifiedResultTypes;
    List<SpvWord> argumentIds;
};

// Now that we've defined the intermediate data structures we will
// use to represent SPIR-V code during emission, we will move on
// to defining the main context type that will drive SPIR-V
// code generation.

    /// Context used for translating a Slang IR module to SPIR-V
struct SPIRVEmitContext
    : public SourceEmitterBase
    , public SPIRVEmitSharedContext
{
        /// The Slang IR module being translated
    IRModule* m_irModule;

    DiagnosticSink* m_sink;

    // [2.2: Terms]
    //
    // > <id>: A numerical name; the name used to refer to an object, a type,
    // > a function, a label, etc. An <id> always consumes one word.
    // > The <id>s defined by a module obey SSA.
    //
    // [2.3: Physical Layout of a SPIR-V Module and Instruction]
    //
    // > Bound; where all <id>s in this module are guaranteed to satisfy
    // > 0 < id < Bound
    // > Bound should be small, smaller is better, with all <id> in a module being densely packed and near 0.
    //
    // Instructions will be referred to by their <id>s.
    // We need to generate <id>s for instructions, and also
    // compute the "bound" value that will be stored in
    // the module header.
    //
    // We will use a single counter and allocate <id>s
    // on demand. There may be some slop where we allocate
    // an <id> for something that never gets referenced,
    // but we expect the amount of slop to be small (and
    // it can be cleaned up by other tools/passes).

        /// The next destination `<id>` to allocate.
    SpvWord m_nextID = 1;

    // We will store the logical sections of the SPIR-V module
    // in a single array so that we can easily look up a
    // section by its `SpvLogicalSectionID`.

        /// The logical sections of the SPIR-V module
    SpvLogicalSection m_sections[int(SpvLogicalSectionID::Count)];

        /// Get a logical section based on its `SpvLogicalSectionID`
    SpvLogicalSection* getSection(SpvLogicalSectionID id)
    {
        return &m_sections[int(id)];
    }

    // At the end of emission we need a single linear stream of words,
    // so we will eventually flatten `m_sections` into a single array.

        /// The final array of SPIR-V words that defines the encoded module
    List<SpvWord> m_words;

        /// Emit the concrete words that make up the binary SPIR-V module.
        ///
        /// This function fills in `m_words` based on the data in `m_sections`.
        /// This function should only be called once.
        ///
    void emitPhysicalLayout()
    {
        // [2.3: Physical Layout of a SPIR-V Module and Instruction]
        //
        // > Magic Number
        //
        m_words.add(SpvMagicNumber);

        // > Version nuumber
        //

        // TODO(JS): 
        // Was previously set to SpvVersion, but that doesn't work since we 
        // upgraded to SPIR-V headers 1.6. (It would lead to validation errors during vk tests)
        // For now mark as version 1.5.0

        static const uint32_t spvVersion1_5_0 = 0x00010500;
        m_words.add(spvVersion1_5_0);

        // > Generator's magic number.
        // > Its value does not affect any semantics, and is allowed to be 0.
        //
        // TODO: We should eventually register a non-zero
        // magic number to represent Slang/slangc.
        //
        m_words.add(0);

        // > Bound
        //
        // As described above, we use `m_nextID` to allocate
        // <id>s, so its value when we are done emitting code
        // can serve as the bound.
        //
        m_words.add(m_nextID);

        // > 0 (Reserved for instruction schema, if needed.)
        //
        m_words.add(0);

        // > First word of instruction stream
        // > All remaining words are a linear sequence of instructions.
        //
        // Once we are done emitting the header, we emit all
        // the instructions in our logical sections.
        // 
        for( int ii = 0; ii < int(SpvLogicalSectionID::Count); ++ii )
        {
            m_sections[ii].dumpTo(m_words);
        }
    }

    // We will often need to refer to an instrcition by its
    // <id>, given only the Slang IR instruction that represents
    // it (e.g., when it is used as an operand of another
    // instruction).
    //
    // To that end we will keep a map of instructions that
    // have been emitted, where a Slang IR instruction maps
    // to the corresponding SPIR-V instruction.

        /// Map a Slang IR instruction to the corresponding SPIR-V instruction
    Dictionary<IRInst*, SpvInst*> m_mapIRInstToSpvInst;

    // Sometimes we need to reserve an ID for an `IRInst` without actually
    // emitting it. We use `m_mapIRInstToSpvID` to hold all reserved SpvIDs.
    // Use `getIRInstSpvID` to obtain an SpvID for an `IRInst` if the
    // `IRInst` may not have been emitted.
    Dictionary<IRInst*, SpvWord> m_mapIRInstToSpvID;

        /// Register that `irInst` maps to `spvInst`
    void registerInst(IRInst* irInst, SpvInst* spvInst)
    {
        m_mapIRInstToSpvInst.Add(irInst, spvInst);

        // If we have reserved an SpvID for `irInst`, make sure to use it.
        SpvWord reservedID = 0;
        m_mapIRInstToSpvID.TryGetValue(irInst, reservedID);

        if (reservedID)
        {
            SLANG_ASSERT(spvInst->id == 0);
            spvInst->id = reservedID;
        }
    }

        /// Get or reserve a SpvID for an IR value.
    SpvWord getIRInstSpvID(IRInst* inst)
    {
        // If we have already emitted an SpvInst for `inst`, return its ID.
        SpvInst* spvInst = nullptr;
        if (m_mapIRInstToSpvInst.TryGetValue(inst, spvInst))
            return getID(spvInst);
        // Check if we have reserved an ID for `inst`.
        SpvWord result = 0;
        if (m_mapIRInstToSpvID.TryGetValue(inst, result))
            return result;
        // Otherwise, reserve a new ID for inst, and register it in `m_mapIRInstToSpvID`.
        result = m_nextID;
        ++m_nextID;
        m_mapIRInstToSpvID[inst] = result;
        return result;
    }

    // When we are emitting an instruction that can produce
    // a result, we will allocate an <id> to it so that other
    // instructions can refer to it.
    //
    // We will allocate <id>s on emand as they are needed.

        /// Get the <id> for `inst`, or assign one if it doesn't have one yet
    SpvWord getID(SpvInst* inst)
    {
        auto id = inst->id;
        if( !id )
        {
            id = m_nextID++;
            inst->id = id;
        }
        return id;
    }

    // We will build up `SpvInst`s in a stateful fashion,
    // mostly for convenience. We could in theory compute
    // the number of words each instruction needs, then allocate
    // the words, then fill them in, but that would make the
    // emit logic more complicated and we'd like to keep it simple
    // until we are sure performance is an issue.
    //
    // Emitting an instruction starts with picking the opcode
    // and allocating the `SpvInst`.

    // Holds a stack of instructions operands *BEFORE* they added to the instruction.
    List<SpvWord> m_operandStack;
    // The current instruction being constructed. Cannot add operands unless it is set.
    SpvInst* m_currentInst = nullptr;

    // Operands can only be added when inside of a InstConstructScope 
    struct InstConstructScope
    {
        SLANG_FORCE_INLINE operator SpvInst*() const { return m_inst; }

        InstConstructScope(SPIRVEmitContext* context, SpvOp opcode, IRInst* irInst = nullptr):
            m_context(context)
        {
            m_context->_beginInst(opcode, irInst, *this);
        }
        ~InstConstructScope()
        {
            m_context->_endInst(*this);
        }

        SpvInst* m_inst;                    ///< The instruction associated with this scope
        SPIRVEmitContext* m_context;        ///< The context
        SpvInst* m_previousInst;            ///< The previously live inst
        Index m_operandsStartIndex;         ///< The start index for operands of m_inst
    };

        /// Holds memory for instructions and operands.
    MemoryArena m_memoryArena;

        /// Begin emitting an instruction with the given SPIR-V `opcode`.
        ///
        /// If `irInst` is non-null, then the resulting SPIR-V instruction
        /// will be registered as corresponding to `irInst`.
        ///
        /// The created instruction is stored in m_currentInst.
        ///
        /// Should not typically be called directly use InstConstructScope to scope construction
    void _beginInst(SpvOp opcode, IRInst* irInst, InstConstructScope& ioScope)
    {
        SLANG_ASSERT(this == ioScope.m_context);

        // Allocate the instruction
        auto spvInst = new (m_memoryArena.allocate(sizeof(SpvInst))) SpvInst();
        spvInst->opcode = opcode;

        if(irInst)
        {
            registerInst(irInst, spvInst);
        }

        // Set up the scope
        ioScope.m_inst = spvInst;
        ioScope.m_previousInst = m_currentInst;
        ioScope.m_operandsStartIndex = m_operandStack.getCount();

        // Set the current instruction
        m_currentInst = spvInst;
    }

        /// End emitting an instruction
        /// Should not typically be called directly use InstConstructScope to scope construction
    void _endInst(const InstConstructScope& scope)
    {
        SLANG_ASSERT(scope.m_inst == m_currentInst);

        const Index operandsStartIndex = scope.m_operandsStartIndex;
        // Work out how many operands were added
        const Index operandsCount = m_operandStack.getCount() - operandsStartIndex;

        
        if (operandsCount)
        {
            // Allocate the operands
            m_currentInst->operandWords = m_memoryArena.allocateAndCopyArray(m_operandStack.getBuffer() + operandsStartIndex, operandsCount);
            // Set the count
            m_currentInst->operandWordsCount = uint32_t(operandsCount);
        }

        // Make the previous inst active
        m_currentInst = scope.m_previousInst;
        // Reset the operand stack
        m_operandStack.setCount(operandsStartIndex);
    }

    /// Ensure that an instruction has been emitted
    SpvInst* ensureInst(IRInst* irInst)
    {
        SpvInst* spvInst = nullptr;
        if (!m_mapIRInstToSpvInst.TryGetValue(irInst, spvInst))
        {
            // If the `irInst` hasn't already been emitted,
            // then we will assume that is is a global instruction
            // (a constant, type, function, etc.) and we should make
            // sure it gets emitted now.
            //
            // Note: this step means that emitting an instruction
            // can be re-entrant/recursive. Because we emit the SPIR-V
            // words for an instruction into an intermediate structure
            // we don't have to worry about the re-entrancy causing
            // the ordering of instruction words to be interleaved.
            //
            spvInst = emitGlobalInst(irInst);
        }
        return spvInst;
    }

    // Whilst an instruction has been created, we append the operand
    // words to it with `emitOperand`. There are a few different
    // case of operands that we handle.
    //
    // The simplest case is when an instruction takes an operand
    // that is just a literal SPIR-V word.

        /// Emit a literal `word` as an operand to the current instruction
    void emitOperand(SpvWord word)
    {
        // Can only add operands if we are constructing an instruction (ie in _beginInst/_endInst)
        SLANG_ASSERT(m_currentInst);
        m_operandStack.add(word);
    }

    // The most common case of operand is an <id> that represents
    // some other instruction. In cases where we already have
    // an <id> we can emit it as a literal and the meaning is
    // the same. If we have a `SpvInst` we can look up or
    // generate an <id> for it.

        /// Emit an operand to the current instruction, which references `src` by its <id>
    void emitOperand(SpvInst* src)
    {
        emitOperand(getID(src));
    }

    // Commonly, we will have an operand in the form of an `IRInst`
    // which might either represent an instruction we've already
    // emitted (e.g., because it came earlier in a function body)
    // or which we have yet to emit (because it is a global-scope
    // instruction that has not been referenced before).

        /// Emit an operand to the current instruction, which references `src` by its <id>
    void emitOperand(IRInst* src)
    {
        SpvInst* spvSrc = ensureInst(src);
        emitOperand(getID(spvSrc));
    }

    // Some instructions take a string as a literal operand,
    // which requires us to follow the SPIR-V rules to
    // encode the string into multiple operand words.

        /// Emit an operand that is encoded as a literal string
    void emitOperand(UnownedStringSlice const& text)
    {
        // Can only emitOperands if we are in an instruction
        SLANG_ASSERT(m_currentInst);
        SLANG_COMPILE_TIME_ASSERT(sizeof(SpvWord) == 4);

        // Assert that `text` doesn't contain any embedded nul bytes, since they
        // could lead to invalid encoded results.
        SLANG_ASSERT(text.indexOf(0) < 0);

        // [Section 2.2.1 : Instructions]
        //
        // > Literal String: A nul-terminated stream of characters consuming
        // > an integral number of words. The character set is Unicode in the
        // > UTF-8 encoding scheme. The UTF-8 octets (8-bit bytes) are packed
        // > four per word, following the little-endian convention (i.e., the
        // > first octet is in the lowest-order 8 bits of the word).
        // > The final word contains the string’s nul-termination character (0), and
        // > all contents past the end of the string in the final word are padded with 0.

        // First work out the amount of words we'll need
        const Index textCount = text.getLength();
        // Calculate the minimum amount of bytes needed - which needs to include terminating 0
        const Index minByteCount = textCount + 1;
        // Calculate the amount of words including padding if necessary
        const Index wordCount = (minByteCount + 3) >> 2;

        // Make space on the operand stack, keeping the free space start in operandStartIndex
        const Index operandStartIndex = m_operandStack.getCount();
        m_operandStack.setCount(operandStartIndex + wordCount);

        // Set dst to the start of the operand memory
        char* dst = (char*)(m_operandStack.getBuffer() + operandStartIndex);

        // Copy the text
        memcpy(dst, text.begin(), textCount);

        // Set terminating 0, and remaining buffer 0s
        memset(dst + textCount, 0, wordCount * sizeof(SpvWord) - textCount);
    }

    // Sometimes we will want to pass down an argument that
    // represents a result <id> operand, but we won't yet
    // have access to the `SpvInst` that will get the <id>.
    // We will use a dummy `enum` type to support this case.

    enum ResultIDToken { kResultID };

    void emitOperand(ResultIDToken)
    {
        SLANG_ASSERT(m_currentInst);

        // A result <id> operand uses the <id> of the instruction itself (which is m_currentInst)
        emitOperand(getID(m_currentInst));
    }

    void emitOperand(SpvDecoration decoration) { emitOperand((SpvWord)decoration); }

    void emitOperand(SpvBuiltIn builtin) { emitOperand((SpvWord)builtin); }
    void emitOperand(SpvStorageClass val) { emitOperand((SpvWord)val); }

    template<typename TConstant>
    struct ConstantValueKey
    {
        IRType* type;
        TConstant value;
        HashCode getHashCode() const
        {
            return combineHash(Slang::getHashCode(type), Slang::getHashCode(value));
        }
        bool operator==(const ConstantValueKey& other) const
        {
            return type == other.type && value == other.value;
        }
    };
    Dictionary<ConstantValueKey<IRIntegerValue>, SpvInst*> m_spvIntConstants;
    Dictionary<ConstantValueKey<IRFloatingPointValue>, SpvInst*> m_spvFloatConstants;
    SpvInst* emitIntConstant(IRIntegerValue val, IRType* type)
    {
        ConstantValueKey<IRIntegerValue> key;
        key.value = val;
        key.type = type;
        SpvInst* result = nullptr;
        if (m_spvIntConstants.TryGetValue(key, result))
            return result;
        SpvWord valWord;
        memcpy(&valWord, &val, sizeof(SpvWord));
        if (type->getOp() == kIROp_Int64Type || type->getOp() == kIROp_UInt64Type)
        {
            SpvWord valHighWord;
            memcpy(&valHighWord, (char*)(&val) + 4, sizeof(SpvWord));
            result = emitInst(
                getSection(SpvLogicalSectionID::Constants),
                nullptr,
                SpvOpConstant,
                type,
                kResultID,
                valWord,
                valHighWord);
        }
        else
        {
            result = emitInst(
                getSection(SpvLogicalSectionID::Constants),
                nullptr,
                SpvOpConstant,
                type,
                kResultID,
                valWord);
        }
        m_spvIntConstants[key] = result;
        return result;
    }
    SpvInst* emitFloatConstant(IRFloatingPointValue val, IRType* type)
    {
        ConstantValueKey<IRFloatingPointValue> key;
        key.value = val;
        key.type = type;
        SpvInst* result = nullptr;
        if (m_spvFloatConstants.TryGetValue(key, result))
            return result;
        SpvWord valWord;
        memcpy(&valWord, &val, sizeof(SpvWord));
        if (type->getOp() == kIROp_DoubleType)
        {
            SpvWord valHighWord;
            memcpy(&valHighWord, (char*)(&val) + 4, sizeof(SpvWord));
            result = emitInst(
                getSection(SpvLogicalSectionID::Constants),
                nullptr,
                SpvOpConstant,
                type,
                kResultID,
                valWord,
                valHighWord);
        }
        else
        {
            result = emitInst(
                getSection(SpvLogicalSectionID::Constants),
                nullptr,
                SpvOpConstant,
                type,
                kResultID,
                valWord);
        }
        m_spvFloatConstants[key] = result;
        return result;
    }
    // As another convenience, there are often cases where
    // we will want to emit all of the operands of some
    // IR instruction as <id> operands of a SPIR-V
    // instruction. This is handy in cases where the
    // Slang IR and SPIR-V instructions agree on the
    // number, order, and meaning of their operands.

        /// Helper type for emitting all the operands of the current IR instruction
    struct OperandsOf
    {
        OperandsOf(IRInst* irInst)
            : irInst(irInst)
        {}

        IRInst* irInst = nullptr;
    };

        /// Emit operand words for all the operands of a given IR instruction
    void emitOperand(OperandsOf const& other)
    {
        auto irInst = other.irInst;
        auto operandCount = irInst->getOperandCount();
        for( UInt ii = 0; ii < operandCount; ++ii )
        {
            emitOperand(irInst->getOperand(ii));
        }
    }

    // With the above routines, code can easily construct a SPIR-V
    // instruction with arbitrary operands over multiple lines of code.
    //
    // In many cases, however, it is desirable to be able to emit
    // an instruction more compactly, and for that we will introduce
    // a number of `emitInst()` helpers that handle creating an
    // instruction, filling in its operands, and adding it to a parent.
    //
    // These routines are overloaded on the number of operands, and
    // also templates to work with any of the types for which
    // `emitOperand()` works.
    //
    // In all of these cases, the caller takes responsibility for
    // correctly matching the SPIR-V encoding rules for the chosen
    // opcode, including whether a type <id> or result <id> is
    // required.

    SpvInst* emitInst(SpvInstParent* parent, IRInst* irInst, SpvOp opcode)
    {
        InstConstructScope scopeInst(this, opcode, irInst);
        SpvInst* spvInst = scopeInst;
        parent->addInst(spvInst);
        return spvInst;
    }

    template<typename A>
    SpvInst* emitInst(SpvInstParent* parent, IRInst* irInst, SpvOp opcode, A const& a)
    {
        InstConstructScope scopeInst(this, opcode, irInst);
        SpvInst* spvInst = scopeInst;
        emitOperand(a);
        parent->addInst(spvInst);
        return spvInst;
    }

    template<typename A, typename B>
    SpvInst* emitInst(SpvInstParent* parent, IRInst* irInst, SpvOp opcode, A const& a, B const& b)
    {
        InstConstructScope scopeInst(this, opcode, irInst);
        SpvInst* spvInst = scopeInst;
        emitOperand(a);
        emitOperand(b);
        parent->addInst(spvInst);
        return spvInst;
    }

    template<typename A, typename B, typename C>
    SpvInst* emitInst(SpvInstParent* parent, IRInst* irInst, SpvOp opcode, A const& a, B const& b, C const& c)
    {
        InstConstructScope scopeInst(this, opcode, irInst);
        SpvInst* spvInst = scopeInst;
        emitOperand(a);
        emitOperand(b);
        emitOperand(c);
        parent->addInst(spvInst);
        return spvInst;
    }

    template<typename A, typename B, typename C, typename D>
    SpvInst* emitInst(SpvInstParent* parent, IRInst* irInst, SpvOp opcode, A const& a, B const& b, C const& c, D const& d)
    {
        InstConstructScope scopeInst(this, opcode, irInst);
        SpvInst* spvInst = scopeInst;
        emitOperand(a);
        emitOperand(b);
        emitOperand(c);
        emitOperand(d);
        parent->addInst(spvInst);
        return spvInst;
    }

    template<typename A, typename B, typename C, typename D, typename E>
    SpvInst* emitInst(SpvInstParent* parent, IRInst* irInst, SpvOp opcode, A const& a, B const& b, C const& c, D const& d, E const& e)
    {
        InstConstructScope scopeInst(this, opcode, irInst);
        SpvInst* spvInst = scopeInst;
        emitOperand(a);
        emitOperand(b);
        emitOperand(c);
        emitOperand(d);
        emitOperand(e);
        parent->addInst(spvInst);
        return spvInst;
    }

    template<typename OperandEmitFunc>
    SpvInst* emitInstCustomOperandFunc(SpvInstParent* parent, IRInst* irInst, SpvOp opcode, const OperandEmitFunc& f)
    {
        InstConstructScope scopeInst(this, opcode, irInst);
        SpvInst* spvInst = scopeInst;
        f();
        parent->addInst(spvInst);
        return spvInst;
    }

        /// The SPIRV OpExtInstImport inst that represents the GLSL450
        /// extended instruction set.
    SpvInst* m_glsl450ExtInst = nullptr;

    SpvInst* getGLSL450ExtInst()
    {
        if (m_glsl450ExtInst)
            return m_glsl450ExtInst;
        m_glsl450ExtInst = emitInst(
            getSection(SpvLogicalSectionID::ExtIntInstImports),
            nullptr,
            SpvOpExtInstImport,
            UnownedStringSlice("GLSL.std.450"));
        return m_glsl450ExtInst;
    }

    // Now that we've gotten the core infrastructure out of the way,
    // let's start looking at emitting some instructions that make
    // up a SPIR-V module.
    //
    // We will start with certain instructions that are required
    // to appear in a well-formed SPIR-V module for Vulkan, but
    // which do not directly relate to any instruction in the
    // Slang IR.

        /// Emit the mandatory "front-matter" instructions that
        /// the SPIR-V module must include to make it usable.
    void emitFrontMatter()
    {
        // TODO: We should ideally add SPIR-V capabilities to
        // the module as we emit instructions that require them.
        // For now we will always emit the `Shader` capability,
        // since every Vulkan shader module will use it.
        //
        emitInst(getSection(SpvLogicalSectionID::Capabilities), nullptr, SpvOpCapability, SpvCapabilityShader);

        // [2.4: Logical Layout of a Module]
        //
        // > The single required OpMemoryModel instruction.
        //
        // A memory model is always required in SPIR-V module.
        //
        // The Vulkan spec further says:
        //
        // > The `Logical` addressing model must be selected
        //
        // It isn't clear if the GLSL450 memory model is also
        // a requirement, but it is what glslang produces,
        // so we will use it for now.
        //
        emitInst(getSection(SpvLogicalSectionID::MemoryModel),  nullptr, SpvOpMemoryModel, SpvAddressingModelLogical, SpvMemoryModelGLSL450);
    }

    Dictionary<UnownedStringSlice, SpvInst*> m_extensionInsts;
    SpvInst* ensureExtensionDeclaration(UnownedStringSlice name)
    {
        SpvInst* result = nullptr;
        if (m_extensionInsts.TryGetValue(name, result))
            return result;
        result =
            emitInst(getSection(SpvLogicalSectionID::Extensions), nullptr, SpvOpExtension, name);
        m_extensionInsts[name] = result;
        return result;
    }

    struct SpvTypeInstKey
    {
        List<SpvWord> words;
        bool operator==(const SpvTypeInstKey& other)
        {
            if (words.getCount() != other.words.getCount())
                return false;
            for (Index i = 0; i < words.getCount(); i++)
                if (words[i] != other.words[i])
                    return false;
            return true;
        }
        HashCode getHashCode()
        {
            HashCode result = 0;
            for (auto word : words)
                result = combineHash(result, word);
            return result;
        }
    };

    Dictionary<SpvTypeInstKey, SpvInst*> m_spvTypeInsts;

    // Emits a SPV Inst that represents a type, with deduplications since
    // our IR doesn't currently guarantee types are unique in generated SPV.
    SpvInst* emitTypeInst(IRInst* typeInst, SpvOp opcode, ArrayView<SpvWord> operands)
    {
        SpvTypeInstKey key;
        key.words.add((SpvWord)opcode);
        for (auto op : operands)
            key.words.add(op);
        SpvInst* result = nullptr;
        if (m_spvTypeInsts.TryGetValue(key, result))
        {
            return result;
        }
        result = emitInstCustomOperandFunc(
            getSection(SpvLogicalSectionID::Types), typeInst, opcode, [&]() {
                emitOperand(kResultID);
                for (auto op : operands)
                {
                    emitOperand(op);
                }
            });
        m_spvTypeInsts[key] = result;
        return result;
    }

    // Next, let's look at emitting some of the instructions
    // that can occur at global scope.

        /// Emit an instruction that is expected to appear at the global scope of the SPIR-V module.
        ///
        /// Returns the corresponding SPIR-V instruction.
        ///
    SpvInst* emitGlobalInst(IRInst* inst)
    {
        switch( inst->getOp() )
        {
        // [3.32.6: Type-Declaration Instructions]
        //

#define CASE(IROP, SPVOP) \
        case IROP: return emitTypeInst(inst, SPVOP, ArrayView<SpvWord>());

        // > OpTypeVoid
        CASE(kIROp_VoidType, SpvOpTypeVoid);

        // > OpTypeBool
        CASE(kIROp_BoolType, SpvOpTypeBool);

#undef CASE

        // > OpTypeInt

#define CASE(IROP, BITS, SIGNED) \
        case IROP:                                                                     \
        return emitTypeInst(inst, SpvOpTypeInt, makeArray<SpvWord>((SpvWord)BITS, (SpvWord)SIGNED).getView()); 

        CASE(kIROp_IntType,     32, 1);
        CASE(kIROp_UIntType,    32, 0);
        CASE(kIROp_Int64Type,   64, 1);
        CASE(kIROp_UInt64Type,  64, 0);

#undef CASE

        // > OpTypeFloat

#define CASE(IROP, BITS) \
        case IROP:                                                                \
        return emitTypeInst(                                                      \
            inst, SpvOpTypeFloat, makeArray<SpvWord>(BITS).getView()); \

        CASE(kIROp_HalfType,    16);
        CASE(kIROp_FloatType,   32);
        CASE(kIROp_DoubleType,  64);

#undef CASE
        case kIROp_PtrType:
        case kIROp_RefType:
        case kIROp_OutType:
        case kIROp_InOutType:
            {
                SpvStorageClass storageClass = SpvStorageClassFunction;
                auto ptrType = as<IRPtrTypeBase>(inst);
                if (ptrType->hasAddressSpace())
                    storageClass = (SpvStorageClass)ptrType->getAddressSpace();
                if (storageClass == SpvStorageClassStorageBuffer)
                    ensureExtensionDeclaration(UnownedStringSlice("SPV_KHR_storage_buffer_storage_class"));
                auto operands = makeArray<SpvWord>(
                    (SpvWord)storageClass, getID(ensureInst(inst->getOperand(0))));
                return emitTypeInst(
                    inst, SpvOpTypePointer, operands.getView());
            }
        case kIROp_StructType:
            {
                auto spvStructType = emitInstCustomOperandFunc(
                    getSection(SpvLogicalSectionID::Types), inst, SpvOpTypeStruct, [&]() {
                        emitOperand(kResultID);
                        for (auto field : static_cast<IRStructType*>(inst)->getFields())
                        {
                            emitOperand(field->getFieldType());
                            // TODO: decorate offset
                        }
                    });
                emitDecorations(inst, getID(spvStructType));
                return spvStructType;
            }
        case kIROp_VectorType:
            {
                auto vectorType = static_cast<IRVectorType*>(inst);
                return ensureVectorType(
                    static_cast<IRBasicType*>(vectorType->getElementType())->getBaseType(),
                    static_cast<IRIntLit*>(vectorType->getElementCount())->getValue(),
                    vectorType);
            }
        case kIROp_MatrixType:
            {
                auto matrixType = static_cast<IRMatrixType*>(inst);
                auto vectorSpvType = ensureVectorType(
                    static_cast<IRBasicType*>(matrixType->getElementType())->getBaseType(),
                    static_cast<IRIntLit*>(matrixType->getRowCount())->getValue(),
                    nullptr);
                auto matrixSPVType = emitInst(
                    getSection(SpvLogicalSectionID::Types),
                    inst,
                    SpvOpTypeMatrix,
                    kResultID,
                    vectorSpvType,
                    (SpvWord)static_cast<IRIntLit*>(matrixType->getColumnCount())->getValue());
                // TODO: properly compute matrix stride.
                auto columnCount = static_cast<IRIntLit*>(matrixType->getRowCount())->getValue();
                uint32_t stride = 0;
                switch (columnCount)
                {
                case 1:
                    stride = 4;
                    break;
                case 2:
                    stride = 8;
                    break;
                case 3:
                case 4:
                    stride = 16;
                    break;
                default:
                    break;
                }
                emitInst(
                    getSection(SpvLogicalSectionID::Annotations),
                    nullptr,
                    SpvOpDecorate,
                    matrixSPVType,
                    SpvDecorationRowMajor,
                    SpvDecorationMatrixStride,
                    stride);
                return matrixSPVType;
            }
        case kIROp_UnsizedArrayType:
            {
                auto elementType = static_cast<IRUnsizedArrayType*>(inst)->getElementType();
                auto runtimeArrayType = emitInst(
                    getSection(SpvLogicalSectionID::Types),
                    nullptr,
                    SpvOpTypeRuntimeArray,
                    kResultID,
                    elementType);
                // TODO: properly decorate stride.
                IRSizeAndAlignment sizeAndAlignment;
                getNaturalSizeAndAlignment(this->m_targetRequest, elementType, &sizeAndAlignment);
                emitInst(
                    getSection(SpvLogicalSectionID::Annotations),
                    nullptr,
                    SpvOpDecorate,
                    runtimeArrayType,
                    SpvDecorationArrayStride,
                    (SpvWord)sizeAndAlignment.getStride());
                return runtimeArrayType;
            }
        // > OpTypeImage
        // > OpTypeSampler
        // > OpTypeArray
        // > OpTypeRuntimeArray
        // > OpTypeOpaque
        // > OpTypePointer

        case kIROp_FuncType:
            // > OpTypeFunction
            //
            // Both Slang and SPIR-V encode a function type
            // with the result-type operand coming first,
            // followed by operand sfor all the parameter types.
            //
            return emitInst(getSection(SpvLogicalSectionID::Types), inst, SpvOpTypeFunction, kResultID, OperandsOf(inst));

        case kIROp_RateQualifiedType:
            {
                auto result = emitGlobalInst(as<IRRateQualifiedType>(inst)->getValueType());
                registerInst(inst, result);
                return result;
            }
        // > OpTypeForwardPointer

        case kIROp_Func:
            // [3.32.6: Function Instructions]
            //
            // > OpFunction
            //
            // Functions are complex enough that we'll handle
            // them in a dedicated subroutine.
            //
            return emitFunc(as<IRFunc>(inst));

         case kIROp_BoolLit:
         case kIROp_IntLit:
         case kIROp_FloatLit:
             return emitLit(inst);

        case kIROp_GlobalParam:
             return emitGlobalParam(as<IRGlobalParam>(inst));
        case kIROp_GlobalVar:
            return emitGlobalVar(as<IRGlobalVar>(inst));
        // ...

        default:
            SLANG_UNIMPLEMENTED_X("unhandled instruction opcode");
            UNREACHABLE_RETURN(nullptr);
        }
    }

    // Ensures an SpvInst for the specified vector type is emitted.
    // `inst` represents an optional `IRVectorType` inst representing the vector type, if
    // it is nullptr, this function will create one.
    SpvInst* ensureVectorType(BaseType baseType, IRIntegerValue elementCount, IRVectorType* inst)
    {
        if (!inst)
        {
            IRBuilder builder(m_sharedIRBuilder);
            builder.setInsertInto(m_irModule->getModuleInst());
            inst = builder.getVectorType(
                builder.getBasicType(baseType),
                builder.getIntValue(builder.getIntType(), elementCount));
        }
        auto operands =
            makeArray<SpvWord>(getID(ensureInst(inst->getElementType())), (SpvWord)elementCount);
        auto result = emitTypeInst(inst, SpvOpTypeVector, operands.getView());
        return result;
    }

    void emitVarLayout(SpvInst* varInst, IRVarLayout* layout)
    {
        for (auto rr : layout->getOffsetAttrs())
        {
            UInt index = rr->getOffset();
            UInt space = rr->getSpace();
            switch (rr->getResourceKind())
            {
            case LayoutResourceKind::Uniform:
                break;

            case LayoutResourceKind::VaryingInput:
                emitInst(
                    getSection(SpvLogicalSectionID::Annotations),
                    nullptr,
                    SpvOpDecorate,
                    varInst,
                    SpvDecorationLocation,
                    (SpvWord)index);
                emitInst(
                    getSection(SpvLogicalSectionID::Annotations),
                    nullptr,
                    SpvOpDecorate,
                    varInst,
                    SpvDecorationIndex,
                    (SpvWord)space);
                break;
            case LayoutResourceKind::VaryingOutput:
                emitInst(
                    getSection(SpvLogicalSectionID::Annotations),
                    nullptr,
                    SpvOpDecorate,
                    varInst,
                    SpvDecorationLocation,
                    (SpvWord)index);
                if (space)
                {
                    emitInst(
                        getSection(SpvLogicalSectionID::Annotations),
                        nullptr,
                        SpvOpDecorate,
                        varInst,
                        SpvDecorationIndex,
                        (SpvWord)space);
                }
                break;

            case LayoutResourceKind::SpecializationConstant:
                emitInst(
                    getSection(SpvLogicalSectionID::Annotations),
                    nullptr,
                    SpvOpDecorate,
                    varInst,
                    SpvDecorationSpecId,
                    (SpvWord)index);
                break;

            case LayoutResourceKind::ConstantBuffer:
            case LayoutResourceKind::ShaderResource:
            case LayoutResourceKind::UnorderedAccess:
            case LayoutResourceKind::SamplerState:
            case LayoutResourceKind::DescriptorTableSlot:
                emitInst(
                    getSection(SpvLogicalSectionID::Annotations),
                    nullptr,
                    SpvOpDecorate,
                    varInst,
                    SpvDecorationBinding,
                    (SpvWord)index);
                emitInst(
                    getSection(SpvLogicalSectionID::Annotations),
                    nullptr,
                    SpvOpDecorate,
                    varInst,
                    SpvDecorationDescriptorSet,
                    (SpvWord)space);
                break;
            default:
                break;
            }
        }
    }
        /// Emit a global parameter definition.
    SpvInst* emitGlobalParam(IRGlobalParam* param)
    {
        auto layout = getVarLayout(param);
        auto storageClass = SpvStorageClassUniform;
        if (auto ptrType = as<IRPtrTypeBase>(param->getDataType()))
        {
            if (ptrType->hasAddressSpace())
                storageClass = (SpvStorageClass)ptrType->getAddressSpace();
        }
        if (auto systemValInst = maybeEmitSystemVal(param))
        {
            registerInst(param, systemValInst);
            return systemValInst;
        }
        auto varInst = emitInst(
            getSection(SpvLogicalSectionID::GlobalVariables),
            param,
            SpvOpVariable,
            param->getDataType(),
            kResultID,
            storageClass);
        emitVarLayout(varInst, layout);
        return varInst;   
    }

        /// Emit a global variable definition.
    SpvInst* emitGlobalVar(IRGlobalVar* globalVar)
    {
        auto layout = getVarLayout(globalVar);
        auto storageClass = SpvStorageClassUniform;
        if (auto ptrType = as<IRPtrTypeBase>(globalVar->getDataType()))
        {
            if (ptrType->hasAddressSpace())
                storageClass = (SpvStorageClass)ptrType->getAddressSpace();
        }
        auto varInst = emitInst(
            getSection(SpvLogicalSectionID::GlobalVariables),
            globalVar,
            SpvOpVariable,
            globalVar->getDataType(),
            kResultID,
            storageClass);
        emitVarLayout(varInst, layout);
        return varInst;
    }

        /// Emit the given `irFunc` to SPIR-V
    SpvInst* emitFunc(IRFunc* irFunc)
    {
        // [2.4: Logical Layout of a Module]
        //
        // > All function declarations ("declarations" are functions
        // > without a body; there is no forward declaration to a
        // > function with a body).
        // > ...
        // > All function definitions (functions with a body).
        //
        // We need to treat functions differently based
        // on whether they have a body or not, since these
        // are encoded differently (and to different sections).
        //
        if( isDefinition(irFunc) )
        {
            return emitFuncDefinition(irFunc);
        }
        else
        {
            return emitFuncDeclaration(irFunc);
        }
    }

        /// Emit a declaration for the given `irFunc`
    SpvInst* emitFuncDeclaration(IRFunc* irFunc)
    {
        // For now we aren't handling function declarations;
        // we expect to deal only with fully linked modules.
        //
        SLANG_UNUSED(irFunc);
        SLANG_UNEXPECTED("function declaration in SPIR-V emit");
        UNREACHABLE_RETURN(nullptr);
    }

        /// Emit a SPIR-V function definition for the Slang IR function `irFunc`.
    SpvInst* emitFuncDefinition(IRFunc* irFunc)
    {
        // [2.4: Logical Layout of a Module]
        //
        // > All function definitions (functions with a body).
        //
        auto section = getSection(SpvLogicalSectionID::FunctionDefinitions);
        //
        // > A function definition is as follows.
        // > * Function definition, using OpFunction.
        // > * Function parameter declarations, using OpFunctionParameter.
        // > * Block
        // > * Block
        // > * ...
        // > * Function end, using OpFunctionEnd.
        //

        // [3.24. Function Control]
        //
        // TODO: We should eventually support emitting the "function control"
        // mask to include inline and other hint bits based on decorations
        // set on `irFunc`.
        //
        SpvFunctionControlMask spvFunctionControl = SpvFunctionControlMaskNone;

        // [3.32.9. Function Instructions]
        //
        // > OpFunction
        //
        // Note that the type <id> of a SPIR-V function uses the
        // *result* type of the function, while the actual function
        // type is given as a later operand. Slan IR instead uses
        // the type of a function instruction store, you know, its *type*.
        //
        SpvInst* spvFunc = emitInst(section, irFunc, SpvOpFunction,
            irFunc->getDataType()->getResultType(),
            kResultID,
            spvFunctionControl,
            irFunc->getDataType());

        // > OpFunctionParameter
        //
        // Unlike Slang, where parameters always belong to blocks,
        // the parameters of a SPIR-V function must appear as direct
        // children of the function instruction, and before any basic blocks.
        //
        for( auto irParam : irFunc->getParams() )
        {
            emitParam(spvFunc, irParam);
        }

        // [3.32.17. Control-Flow Instructions]
        //
        // > OpLabel
        //
        // A Slang `IRBlock` corresponds to a SPIR-V `OpLabel`:
        // each represents a basic block in the control flow
        // graph of a parent function.
        //
        // We will allocate SPIR-V instructions to represent
        // all of the blocks in a function before we emit
        // body instructions into any of them. We do this
        // because it is possible for one block to make
        // forward reference to another (wheras that is
        // not possible for ordinary instructions within
        // the blocks in the Slang IR)
        //
        for( auto irBlock : irFunc->getBlocks() )
        {
            emitInst(spvFunc, irBlock, SpvOpLabel, kResultID);

            // In addition to normal basic blocks,
            // all loops gets a header block.
            for (auto irInst : irBlock->getChildren())
            {
                if (irInst->getOp() == kIROp_loop)
                {
                    emitInst(spvFunc, irInst, SpvOpLabel, kResultID);
                }
            }
        }

        // Once all the basic blocks have had instructions allocated
        // for them, we go through and fill them in with their bodies.
        //
        // Each loop inst results in a loop header block.
        // We will defer the emit of the contents in loop header block
        // until all Phi insts are emitted.
        List<IRLoop*> pendingLoopInsts;
        for( auto irBlock : irFunc->getBlocks() )
        {
            // Note: because we already created the block above,
            // we can be sure that it will have been registred.
            //
            SpvInst* spvBlock = nullptr;
            m_mapIRInstToSpvInst.TryGetValue(irBlock, spvBlock);
            SLANG_ASSERT(spvBlock);

            // [3.32.17. Control-Flow Instructions]
            //
            // > OpPhi
            if (irBlock != irFunc->getFirstBlock())
            {
                for (auto irParam : irBlock->getParams())
                {
                    emitPhi(spvBlock, irParam);
                }
            }
            for( auto irInst : irBlock->getOrdinaryInsts() )
            {
                // Any instructions local to the block will be emitted as children
                // of the block.
                //
                emitLocalInst(spvBlock, irInst);
                if (irInst->getOp() == kIROp_loop)
                    pendingLoopInsts.add(as<IRLoop>(irInst));
            }
        }

        // Finally, we generate the body of loop header blocks.
        for (auto loopInst : pendingLoopInsts)
        {
            SpvInst* headerBlock = nullptr;
            m_mapIRInstToSpvInst.TryGetValue(loopInst, headerBlock);
            SLANG_ASSERT(headerBlock);
            emitLoopHeaderBlock(loopInst, headerBlock);
        }

        // [3.32.9. Function Instructions]
        //
        // > OpFunctionEnd
        //
        // In the SPIR-V encoding a function is logically the parent of any
        // instructions up to a matching `OpFunctionEnd`. In our intermediate
        // structure we will make the `OpFunctionEnd` be the last child of
        // the `OpFunction`.
        //
        emitInst(spvFunc, nullptr, SpvOpFunctionEnd);

        // We will emit any decorations pertinent to the function to the
        // appropriate section of the module.
        //
        emitDecorations(irFunc, getID(spvFunc));

        return spvFunc;
    }

        /// Check if a block is a loop's target block.
    bool isLoopTargetBlock(IRInst* block, IRInst*& loopInst)
    {
        for (auto use = block->firstUse; use; use = use->nextUse)
        {
            if (use->getUser()->getOp() == kIROp_loop &&
                as<IRLoop>(use->getUser())->getTargetBlock() == block)
            {
                loopInst = use->getUser();
                return true;
            }
        }
        return false;
    }

    // The instructions that appear inside the basic blocks of
    // functions are what we will call "local" instructions.
    //
    // When emititng blobal instructions, we usually have to
    // pick the right logical section to emit them into, while
    // for local instructions they will usually emit into
    // a known parent (the basic block that contains them).

        /// Emit an instruction that is local to the body of the given `parent`.
    SpvInst* emitLocalInst(SpvInstParent* parent, IRInst* inst)
    {
        switch( inst->getOp() )
        {
        default:
            SLANG_UNIMPLEMENTED_X("unhandled instruction opcode");
            break;
        case kIROp_Specialize:
            return nullptr;
        case kIROp_Var:
            return emitVar(parent, inst);
        case kIROp_Call:
            return emitCall(parent, inst);
        case kIROp_FieldAddress:
            return emitFieldAddress(parent, as<IRFieldAddress>(inst));
        case kIROp_FieldExtract:
            return emitFieldExtract(parent, as<IRFieldExtract>(inst));
        case kIROp_getElementPtr:
            return emitGetElementPtr(parent, as<IRGetElementPtr>(inst));
        case kIROp_getElement:
            return emitGetElement(parent, as<IRGetElement>(inst));
        case kIROp_Load:
            return emitLoad(parent, as<IRLoad>(inst));
        case kIROp_Store:
            return emitStore(parent, as<IRStore>(inst));
        case kIROp_swizzle:
            return emitSwizzle(parent, as<IRSwizzle>(inst));
        case kIROp_Construct:
            return emitConstruct(parent, inst);
        case kIROp_BitCast:
            return emitInst(
                parent, inst, SpvOpBitcast, inst->getDataType(), kResultID, inst->getOperand(0));
        case kIROp_Add:
        case kIROp_Sub:
        case kIROp_Mul:
        case kIROp_Div:
        case kIROp_IRem:
        case kIROp_FRem:
        case kIROp_Neg:
        case kIROp_Not:
        case kIROp_And:
        case kIROp_Or:
        case kIROp_BitNot:
        case kIROp_BitAnd:
        case kIROp_BitOr:
        case kIROp_BitXor:
        case kIROp_Less:
        case kIROp_Leq:
        case kIROp_Eql:
        case kIROp_Neq:
        case kIROp_Greater:
        case kIROp_Geq:
        case kIROp_Rsh:
        case kIROp_Lsh:
            return emitArithmetic(parent, inst);
        case kIROp_Return:
            if (as<IRReturn>(inst)->getVal()->getOp() == kIROp_VoidLit)
            {
                return emitInst(parent, inst, SpvOpReturn);
            }
            else
            {
                return emitInst(
                    parent, inst, SpvOpReturnValue, as<IRReturn>(inst)->getVal());
            }
        case kIROp_discard:
            return emitInst(parent, inst, SpvOpKill);
        case kIROp_unconditionalBranch:
            {
                // If we are jumping to the main block of a loop,
                // emit a branch to the loop header instead.
                // The SPV id of the resulting loop header block is associated with the loop inst.
                auto targetBlock = as<IRUnconditionalBranch>(inst)->getTargetBlock();
                IRInst* loopInst = nullptr;
                if (isLoopTargetBlock(targetBlock, loopInst))
                {
                    return emitInst(parent, inst, SpvOpBranch, getIRInstSpvID(loopInst));
                }
                // Otherwise, emit a normal branch inst into the target block.
                return emitInst(
                    parent,
                    inst,
                    SpvOpBranch,
                    getIRInstSpvID(targetBlock));
            }
        case kIROp_loop:
            {
                // Return loop header block in its own block.
                auto blockId = getIRInstSpvID(inst);
                SpvInst* block = nullptr;
                m_mapIRInstToSpvInst.TryGetValue(inst, block);
                SLANG_ASSERT(block);

                // Emit a jump to the loop header block.
                // Note: the body of the loop header block is emitted
                // after everything else to ensure Phi instructions (which come
                // from the actual loop target block) are emitted first.
                emitInst(parent, nullptr, SpvOpBranch, blockId);
        
                return block;
            }
        case kIROp_ifElse:
            {
                auto ifelseInst = as<IRIfElse>(inst);
                auto afterBlockID = getIRInstSpvID(ifelseInst->getAfterBlock());
                emitInst(
                    parent,
                    nullptr,
                    SpvOpSelectionMerge,
                    afterBlockID,
                    0);
                auto falseLabel = ifelseInst->getFalseBlock();
                return emitInst(
                    parent,
                    inst,
                    SpvOpBranchConditional,
                    ifelseInst->getCondition(),
                    ifelseInst->getTrueBlock(),
                    falseLabel ? getID(ensureInst(falseLabel)) : afterBlockID);
            }
        case kIROp_Switch:
            {
                auto switchInst = as<IRSwitch>(inst);
                auto mergeBlockID = getIRInstSpvID(switchInst->getBreakLabel());
                emitInst(parent, nullptr, SpvOpSelectionMerge, mergeBlockID, 0);
                return emitInstCustomOperandFunc(parent, inst, SpvOpSwitch, [&]() {
                    emitOperand(switchInst->getCondition());
                    auto defaultLabel = switchInst->getDefaultLabel();
                    emitOperand(defaultLabel ? getID(ensureInst(defaultLabel)) : mergeBlockID);
                    for (UInt c = 0; c < switchInst->getCaseCount(); c++)
                    {
                        auto value = switchInst->getCaseValue(c);
                        auto intLit = as<IRIntLit>(value);
                        SLANG_ASSERT(intLit);
                        emitOperand((SpvWord)intLit->getValue());
                        auto caseLabel = switchInst->getCaseLabel(c);
                        emitOperand(caseLabel ? getID(ensureInst(caseLabel)) : mergeBlockID);
                    }
                });
            }
        case kIROp_Unreachable:
            return emitInst(parent, inst, SpvOpUnreachable);
        case kIROp_conditionalBranch:
            SLANG_UNEXPECTED("Unstructured branching is not supported by SPIRV.");
        }
    }

    SpvInst* emitLit(IRInst* inst)
    {
        switch (inst->getOp())
        {
        case kIROp_IntLit:
            {
                auto value = as<IRIntLit>(inst)->getValue();
                switch (as<IRBasicType>(inst->getDataType())->getBaseType())
                {
                case BaseType::Int64:
                case BaseType::UInt64:
                    return emitInst(
                        getSection(SpvLogicalSectionID::Constants),
                        inst,
                        SpvOpConstant,
                        inst->getDataType(),
                        kResultID,
                        (SpvWord)(value & 0xFFFFFFFF),
                        (SpvWord)((value >> 32) & 0xFFFFFFFF));
                default:
                    return emitInst(
                        getSection(SpvLogicalSectionID::Constants),
                        inst,
                        SpvOpConstant,
                        inst->getDataType(),
                        kResultID,
                        (SpvWord)value);
                }
            }
        case kIROp_FloatLit:
            {
                auto value = as<IRConstant>(inst)->value.floatVal;
                switch (as<IRBasicType>(inst->getDataType())->getBaseType())
                {
                case BaseType::Half:
                    return emitInst(
                        getSection(SpvLogicalSectionID::Constants),
                        inst,
                        SpvOpConstant,
                        inst->getDataType(),
                        kResultID,
                        (SpvWord)(FloatToHalf((float)value)));
                case BaseType::Float:
                    return emitInst(
                        getSection(SpvLogicalSectionID::Constants),
                        inst,
                        SpvOpConstant,
                        inst->getDataType(),
                        kResultID,
                        (SpvWord)(FloatAsInt((float)value)));
                case BaseType::Double:
                    {
                        auto ival = DoubleAsInt64(value);
                        return emitInst(
                            getSection(SpvLogicalSectionID::Constants),
                            inst,
                            SpvOpConstant,
                            inst->getDataType(),
                            kResultID,
                            (SpvWord)(ival&0xFFFFFFFF),
                            (SpvWord)(ival>>32));
                    }
                default:
                    return nullptr;
                }
            }
        case kIROp_BoolLit:
            {
                if (as<IRBoolLit>(inst)->getValue())
                {
                    return emitInst(
                        getSection(SpvLogicalSectionID::Constants),
                        inst,
                        SpvOpConstantTrue,
                        inst->getDataType(),
                        kResultID);
                }
                else
                {
                    return emitInst(
                        getSection(SpvLogicalSectionID::Constants),
                        inst,
                        SpvOpConstantFalse,
                        inst->getDataType(),
                        kResultID);
                }
            }
        default:
            return nullptr;
        }
    }

    // Both "local" and "global" instructions can have decorations.
    // When we decide to emit an instruction, we typically also want
    // to emit any decoratons that were attached to it that have
    // a SPIR-V equivalent.

        /// Emit appropriate SPIR-V decorations for the given IR `irInst`.
        ///
        /// The given `dstID` should be the `<id>` of the SPIR-V instruction being decorated,
        /// and should correspond to `irInst`.
        ///
    void emitDecorations(IRInst* irInst, SpvWord dstID)
    {
        for( auto decoration : irInst->getDecorations() )
        {
            emitDecoration(dstID, decoration);
        }
    }

        /// Emit an appropriate SPIR-V decoration for the given IR `decoration`, if necessary and possible.
        ///
        /// The given `dstID` should be the `<id>` of the SPIR-V instruction being decorated,
        /// and should correspond to the parent of `decoration` in the Slang IR.
        ///
    void emitDecoration(SpvWord dstID, IRDecoration* decoration)
    {
        // Unlike in the Slang IR, decorations in SPIR-V are not children
        // of the instruction they decorate, and instead are free-standing
        // instructions at global scope, which reference their target
        // instruction by its `<id>`.
        //
        // The `IRDecoration` hierarchy in Slang also maps to several
        // different categories of instruction in SPIR-V, only a subset
        // of which are officialy called "decorations."
        //
        // We will continue to use the Slang terminology here, since
        // this code path is a catch-all for stuff that only needs to
        // be emitted if the owning instruction gets emitted.

        switch( decoration->getOp() )
        {
        default:
            break;

        // [3.32.2. Debug Instructions]
        //
        // > OpName
        //
        case kIROp_NameHintDecoration:
            {
                auto section = getSection(SpvLogicalSectionID::DebugNames);
                auto nameHint = cast<IRNameHintDecoration>(decoration);
                emitInst(section, decoration, SpvOpName, dstID, nameHint->getName());
            }
            break;

        // [3.32.5. Mode-Setting Instructions]
        //
        // > OpEntryPoint
        // > Declare an entry point, its execution model, and its interface.
        //
        case kIROp_EntryPointDecoration:
            {
                auto section = getSection(SpvLogicalSectionID::EntryPoints);

                // TODO: The `OpEntryPoint` is required to list an varying
                // input or output parameters (by `<id>`) used by the entry point,
                // although these are encoded as global variables in the IR.
                //
                // Currently we have a pass that moves entry-point varying
                // parameters to global scope for the benefit of GLSL output,
                // but we do not maintain a connection between those parameters
                // and the original entry point. That pass should be updated
                // to attach a decoration linking the original entry point
                // to the new globals, which would be used in the SPIR-V emit case.

                auto entryPointDecor = cast<IREntryPointDecoration>(decoration);
                auto spvStage = mapStageToExecutionModel(entryPointDecor->getProfile().getStage());
                auto name = entryPointDecor->getName()->getStringSlice();
                emitInstCustomOperandFunc(section, decoration, SpvOpEntryPoint, [&]() {
                    emitOperand(spvStage);
                    emitOperand(dstID);
                    emitOperand(name);
                    // `interface` part: reference all global variables that are used by this entrypoint.
                    // TODO: we may want to perform more accurate tracking.
                    for (auto globalInst : m_irModule->getModuleInst()->getChildren())
                    {
                        switch (globalInst->getOp())
                        {
                        case kIROp_GlobalVar:
                        case kIROp_GlobalParam:
                            emitOperand(getIRInstSpvID(globalInst));
                            break;
                        }
                    }
                });
            }
            break;

        // > OpExecutionMode

        // [3.6. Execution Mode]: LocalSize
        case kIROp_NumThreadsDecoration:
            {
                auto section = getSection(SpvLogicalSectionID::ExecutionModes);

                // TODO: The `LocalSize` execution mode option requires
                // literal values for the X,Y,Z thread-group sizes.
                // There is a `LocalSizeId` variant that takes `<id>`s
                // for those sizes, and we should consider using that
                // and requiring the appropriate capabilities
                // if any of the operands to the decoration are not
                // literals (in a future where we support non-literals
                // in those positions in the Slang IR).
                //
                auto numThreads = cast<IRNumThreadsDecoration>(decoration);
                emitInst(section, decoration, SpvOpExecutionMode, dstID, SpvExecutionModeLocalSize,
                    SpvWord(numThreads->getX()->getValue()),
                    SpvWord(numThreads->getY()->getValue()),
                    SpvWord(numThreads->getZ()->getValue()));
            }
            break;

        case kIROp_SPIRVBufferBlockDecoration:
            {
                emitInst(
                    getSection(SpvLogicalSectionID::Annotations),
                    decoration,
                    SpvOpDecorate,
                    dstID,
                    SpvDecorationBlock);
                emitInst(
                    getSection(SpvLogicalSectionID::Annotations),
                    nullptr,
                    SpvOpMemberDecorate,
                    dstID,
                    0,
                    SpvDecorationOffset,
                    0);
            }
            break;
        // ...
        }
    }

        /// Map a Slang `Stage` to a corresponding SPIR-V execution model
    SpvExecutionModel mapStageToExecutionModel(Stage stage)
    {
        switch( stage )
        {
        default:
            SLANG_UNEXPECTED("unhandled stage");
            UNREACHABLE_RETURN((SpvExecutionModel)0);

#define CASE(STAGE, MODEL) \
        case Stage::STAGE: return SpvExecutionModel##MODEL

        CASE(Vertex,    Vertex);
        CASE(Hull,      TessellationControl);
        CASE(Domain,    TessellationEvaluation);
        CASE(Geometry,  Geometry);
        CASE(Fragment,  Fragment);
        CASE(Compute,   GLCompute);

        // TODO: Extended execution models for ray tracing, etc.

#undef CASE
        }
    }

    Dictionary<SpvBuiltIn, SpvInst*> m_builtinGlobalVars;
    SpvInst* getBuiltinGlobalVar(IRType* type, SpvBuiltIn builtinVal)
    {
        SpvInst* result = nullptr;
        if (m_builtinGlobalVars.TryGetValue(builtinVal, result))
        {
            return result;
        }
        IRBuilder builder(m_sharedIRBuilder);
        builder.setInsertBefore(type);
        auto ptrType = as<IRPtrTypeBase>(type);
        SLANG_ASSERT(ptrType && "`getBuiltinGlobalVar`: `type` must be ptr type.");
        auto varInst = emitInst(
            getSection(SpvLogicalSectionID::GlobalVariables),
            nullptr,
            SpvOpVariable,
            type,
            kResultID,
            (SpvStorageClass)ptrType->getAddressSpace());
        emitInst(
            getSection(SpvLogicalSectionID::Annotations),
            nullptr,
            SpvOpDecorate,
            varInst,
            SpvDecorationBuiltIn,
            builtinVal);
        m_builtinGlobalVars[builtinVal] = varInst;
        return varInst;
    }

    SpvInst* maybeEmitSystemVal(IRInst* inst)
    {
        IRBuilder builder(m_sharedIRBuilder);
        builder.setInsertBefore(inst);
        if (auto layout = getVarLayout(inst))
        {
            if (auto systemValueAttr = layout->findAttr<IRSystemValueSemanticAttr>())
            {
                String semanticName = systemValueAttr->getName();
                semanticName = semanticName.toLower();
                if (semanticName == "sv_dispatchthreadid")
                {
                    return getBuiltinGlobalVar(inst->getFullType(), SpvBuiltInGlobalInvocationId);
                }
            }
        }
        return nullptr;
    }

    SpvInst* emitParam(SpvInstParent* parent, IRInst* inst)
    {
        return emitInst(parent, inst, SpvOpFunctionParameter, inst->getFullType(), kResultID);
    }

    SpvInst* emitVar(SpvInstParent* parent, IRInst* inst)
    {
        auto ptrType = as<IRPtrTypeBase>(inst->getDataType());
        SLANG_ASSERT(ptrType);
        SpvStorageClass storageClass = SpvStorageClassFunction;
        if (ptrType->hasAddressSpace())
        {
            storageClass = (SpvStorageClass)ptrType->getAddressSpace();
        }
        return emitInst(parent, inst, SpvOpVariable, inst->getFullType(), kResultID, storageClass);
    }

        /// Cached `IRParam` indices in an `IRBlock`. For use in `getParamIndexInBlock`.
    struct BlockParamIndexInfo : public RefObject
    {
        Dictionary<IRParam*, int> mapParamToIndex;
    };
    Dictionary<IRBlock*, RefPtr<BlockParamIndexInfo>> m_mapIRBlockToParamIndexInfo;

        /// Returns the index of an `IRParam` inside a `IRBlock`.
        /// The results are cached in `m_mapIRBlockToParamIndexInfo` to avoid linear search.
    int getParamIndexInBlock(IRBlock* block, IRParam* paramInst)
    {
        RefPtr<BlockParamIndexInfo> info;
        int result = -1;
        if (m_mapIRBlockToParamIndexInfo.TryGetValue(block, info))
        {
            info->mapParamToIndex.TryGetValue(paramInst, result);
            SLANG_ASSERT(result != -1);
            return result;
        }
        info = new BlockParamIndexInfo();
        int paramIndex = 0;
        for (auto param : block->getParams())
        {
            info->mapParamToIndex[param] = paramIndex;
            if (param == paramInst)
                result = paramIndex;
            paramIndex++;
        }
        m_mapIRBlockToParamIndexInfo[block] = info;
        SLANG_ASSERT(result != -1);
        return result;
    }

    bool isGlobalValueInst(IRInst* inst)
    {
        if (as<IRConstant>(inst))
            return true;
        switch (inst->getOp())
        {
        case kIROp_Func:
        case kIROp_GlobalParam:
        case kIROp_GlobalVar:
            return true;
        default:
            return false;
        }
    }

    void emitLoopHeaderBlock(IRLoop* loopInst, SpvInst* loopHeaderBlock)
    {
        SpvWord loopControl = 0;
        if (auto loopControlDecoration = loopInst->findDecoration<IRLoopControlDecoration>())
        {
            switch (loopControlDecoration->getMode())
            {
            case IRLoopControl::kIRLoopControl_Unroll:
                loopControl = 0x1;
                break;
            case IRLoopControl::kIRLoopControl_Loop:
                loopControl = 0x2;
                break;
            default:
                break;
            }
        }
        emitInst(
            loopHeaderBlock,
            nullptr,
            SpvOpLoopMerge,
            getIRInstSpvID(loopInst->getBreakBlock()),
            getIRInstSpvID(loopInst->getContinueBlock()),
            loopControl);
        emitInst(loopHeaderBlock, nullptr, SpvOpBranch, loopInst->getTargetBlock());
    }

    SpvInst* emitPhi(SpvInstParent* parent, IRParam* inst)
    {
        // An `IRParam` in an ordinary `IRBlock` represents a phi value.
        // We can translate them directly to SPIRV's `Phi` instruction.
        // In order to do that, we need to figure out the source values
        // of this `IRParam`, which can be done by looking at the users
        // of current `IRBlock`.

        // First, we find the index of this param.
        IRBlock* block = as<IRBlock>(inst->getParent());
        // Special case: if block is a loop's target block, emit phis into the header block instead.
        IRInst* loopInst = nullptr;
        if (isLoopTargetBlock(block, loopInst))
        {
            SpvInst* loopSpvBlockInst = nullptr;
            m_mapIRInstToSpvInst.TryGetValue(loopInst, loopSpvBlockInst);
            SLANG_ASSERT(loopSpvBlockInst);
            parent = loopSpvBlockInst;
        }

        SLANG_ASSERT(block);
        int paramIndex = getParamIndexInBlock(block, inst);

        // Emit a Phi instruction.
        return emitInstCustomOperandFunc(parent, inst, SpvOpPhi, [&]() {
            emitOperand(inst->getFullType());
            emitOperand(kResultID);
            // Find phi arguments from incoming branch instructions that target `block`.
            for (auto use = block->firstUse; use; use = use->nextUse)
            {
                auto branchInst = use->getUser();
                UInt argStartIndex = 0;
                switch (branchInst->getOp())
                {
                case kIROp_unconditionalBranch:
                    argStartIndex = 1;
                    break;
                case kIROp_loop:
                    argStartIndex = 3;
                    break;
                default:
                    // A phi argument can only come from an unconditional branch inst.
                    // Other uses are not relavent so we should skip.
                    continue;
                }
                SLANG_ASSERT(argStartIndex + paramIndex < branchInst->getOperandCount());
                auto valueInst = branchInst->getOperand(argStartIndex + paramIndex);
                if (isGlobalValueInst(valueInst))
                    ensureInst(valueInst);
                emitOperand(getIRInstSpvID(valueInst));
                auto sourceBlock = as<IRBlock>(branchInst->getParent());
                SLANG_ASSERT(sourceBlock);
                emitOperand(getIRInstSpvID(sourceBlock));
            }
        });
    }

    SpvInst* emitCall(SpvInstParent* parent, IRInst* inst)
    {
        auto funcValue = inst->getOperand(0);

        // Does this function declare any requirements.
        handleRequiredCapabilities(funcValue);

        // We want to detect any call to an intrinsic operation, and inline
        // the SPIRV snippet directly at the call site.
        if (auto targetIntrinsic = Slang::findBestTargetIntrinsicDecoration(
                funcValue, m_targetRequest->getTargetCaps()))
        {
            return emitIntrinsicCallExpr(parent, static_cast<IRCall*>(inst), targetIntrinsic);
        }
        else
        {
            return emitInst(
                parent, inst, SpvOpFunctionCall, inst->getFullType(), kResultID, OperandsOf(inst));
        }
    }

    SpvInst* emitIntrinsicCallExpr(
        SpvInstParent* parent,
        IRCall* inst,
        IRTargetIntrinsicDecoration* intrinsic)
    {
        SpvSnippet* snippet = getParsedSpvSnippet(intrinsic);
        SpvSnippetEmitContext context;
        context.irResultType = inst->getDataType();
        context.resultType = ensureInst(inst->getFullType());
        context.isResultTypeFloat = isFloatType(inst->getDataType());
        context.isResultTypeSigned = isSignedType((IRType*)inst->getDataType());
        for (SlangUInt i = 0; i < inst->getArgCount(); i++)
        {
            auto argInst = ensureInst(inst->getArg(i));
            if (argInst)
            {
                context.argumentIds.add(getID(argInst));
            }
            else
            {
                context.argumentIds.add(0xFFFFFFFF);
            }
        }
        // A SPIRV snippet may refer to the result type of this inst with a
        // different storage-class qualifier. We need to pre-create these
        // storage-class-qualified result pointer types so they can be used
        // during inlining of the snippet.
        if (auto oldPtrType = as<IRPtrTypeBase>(inst->getDataType()))
        {
            for (auto storageClass : snippet->usedResultTypeStorageClasses)
            {
                IRBuilder builder(m_sharedIRBuilder);
                builder.setInsertBefore(inst);
                auto newPtrType = builder.getPtrType(
                    oldPtrType->getOp(), oldPtrType->getValueType(), storageClass);
                context.qualifiedResultTypes[storageClass] = newPtrType;
            }
        }
        return emitSpvSnippet(parent, inst, context, snippet);
    }

    Dictionary<SpvSnippet::ASMConstant, SpvInst*> m_spvSnippetConstantInsts;

    // Emit SPV Inst that represents a constant defined in a SpvSnippet.
    SpvInst* maybeEmitSpvConstant(SpvSnippet::ASMConstant constant)
    {
        SpvInst* result = nullptr;
        if (m_spvSnippetConstantInsts.TryGetValue(constant, result))
            return result;

        IRBuilder builder(m_sharedIRBuilder);
        builder.setInsertInto(m_irModule->getModuleInst());
        switch (constant.type)
        {
        case SpvSnippet::ASMType::Float:
            result = emitFloatConstant(constant.floatValues[0], builder.getType(kIROp_FloatType));
            break;
        case SpvSnippet::ASMType::Float2:
            {
                auto floatType = builder.getType(kIROp_FloatType);
                auto element1 = emitFloatConstant(constant.floatValues[0], floatType);
                auto element2 = emitFloatConstant(constant.floatValues[1], floatType);
                result = emitInst(
                    getSection(SpvLogicalSectionID::Constants),
                    nullptr,
                    SpvOpConstantComposite,
                    builder.getVectorType(floatType, builder.getIntValue(builder.getIntType(), 2)),
                    kResultID,
                    element1,
                    element2);
            }
        case SpvSnippet::ASMType::Int:
            result = emitIntConstant((IRIntegerValue)constant.intValues[0], builder.getIntType());
            break;
        case SpvSnippet::ASMType::UInt2:
            {
                auto uintType = builder.getType(kIROp_UIntType);
                auto element1 = emitIntConstant((IRIntegerValue)constant.intValues[0], uintType);
                auto element2 = emitIntConstant((IRIntegerValue)constant.intValues[1], uintType);
                result = emitInst(
                    getSection(SpvLogicalSectionID::Constants),
                    nullptr,
                    SpvOpConstantComposite,
                    builder.getVectorType(uintType, builder.getIntValue(builder.getIntType(), 2)),
                    kResultID,
                    element1,
                    element2);
            }
            break;
        }
        m_spvSnippetConstantInsts[constant] = result;
        return result;
    }

    // Emit SPV Inst that represents a type defined in a SpvSnippet.
    void emitSpvSnippetASMTypeOperand(SpvSnippet::ASMType type)
    {
        IRBuilder builder(m_sharedIRBuilder);
        builder.setInsertInto(m_irModule->getModuleInst());
        IRType* irType = nullptr;
        switch (type)
        {
        case SpvSnippet::ASMType::Float:
            irType = builder.getType(kIROp_FloatType);
            break;
        case SpvSnippet::ASMType::Int:
            irType = builder.getIntType();
            break;
        case SpvSnippet::ASMType::Float2:
            irType = builder.getVectorType(
                builder.getType(kIROp_FloatType), builder.getIntValue(builder.getIntType(), 2));
            break;
        case SpvSnippet::ASMType::UInt2:
            irType = builder.getVectorType(
                builder.getType(kIROp_UIntType), builder.getIntValue(builder.getIntType(), 2));
            break;
        default:
            break;
        }
        emitOperand(irType);
    }

    SpvInst* emitSpvSnippet(
        SpvInstParent* parent,
        IRCall* inst,
        const SpvSnippetEmitContext& context,
        SpvSnippet* snippet)
    {
        ShortList<SpvInst*> emittedInsts;
        for (Index i = 0; i < snippet->instructions.getCount(); i++)
        {
            auto& spvSnippetInst = snippet->instructions[i];
            InstConstructScope scopeInst(this, (SpvOp)spvSnippetInst.opCode, nullptr);
            SpvInst* spvInst = scopeInst;
            for (auto operand : spvSnippetInst.operands)
            {
                switch (operand.type)
                {
                case SpvSnippet::ASMOperandType::SpvWord:
                    emitOperand(operand.content);
                    break;
                case SpvSnippet::ASMOperandType::ObjectReference:
                    SLANG_ASSERT(operand.content < (SpvWord)context.argumentIds.getCount());
                    emitOperand(context.argumentIds[operand.content]);
                    break;
                case SpvSnippet::ASMOperandType::ResultId:
                    emitOperand(kResultID);
                    break;
                case SpvSnippet::ASMOperandType::ResultTypeId:
                    if (operand.content != -1)
                    {
                        emitOperand(context.qualifiedResultTypes[(SpvStorageClass)operand.content]
                                        .GetValue());
                    }
                    else
                    {
                        emitOperand(context.resultType);
                    }
                    break;
                case SpvSnippet::ASMOperandType::InstReference:
                    SLANG_ASSERT(operand.content < (SpvWord)emittedInsts.getCount());
                    emitOperand(emittedInsts[operand.content]);
                    break;
                case SpvSnippet::ASMOperandType::GLSL450ExtInstSet:
                    emitOperand(getGLSL450ExtInst());
                    break;
                case SpvSnippet::ASMOperandType::FloatIntegerSelection:
                    if (context.isResultTypeFloat)
                    {
                        emitOperand(operand.content);
                    }
                    else
                    {
                        emitOperand(operand.content2);
                    }
                    break;
                case SpvSnippet::ASMOperandType::FloatUnsignedSignedSelection:
                    if (context.isResultTypeFloat)
                    {
                        emitOperand(operand.content);
                    }
                    else
                    {
                        if (context.isResultTypeSigned)
                        {
                            emitOperand(operand.content3);
                        }
                        else
                        {
                            emitOperand(operand.content2);
                        }
                    }
                    break;
                case SpvSnippet::ASMOperandType::TypeReference:
                    {
                        emitSpvSnippetASMTypeOperand((SpvSnippet::ASMType)operand.content);
                    }
                    break;
                case SpvSnippet::ASMOperandType::ConstantReference:
                    {
                        auto constant = snippet->constants[operand.content];
                        if (constant.type == SpvSnippet::ASMType::FloatOrDouble)
                        {
                            switch (extractBaseType(context.irResultType))
                            {
                            case BaseType::Float:
                                constant.type = SpvSnippet::ASMType::Float;
                                break;
                            case BaseType::Double:
                                constant.type = SpvSnippet::ASMType::Double;
                                break;
                            default:
                                break;
                            }
                        }
                        SpvInst* spvConstant = maybeEmitSpvConstant(constant);
                        emitOperand(spvConstant);
                    }
                    break;
                }
            }
            parent->addInst(spvInst);
            emittedInsts.add(spvInst);
        }
        auto resultInst = emittedInsts.getLast();
        registerInst(inst, resultInst);
        return resultInst;
    }

    struct StructTypeInfo : public RefObject
    {
        Dictionary<IRStructKey*, Index> structFieldIndices;
    };

    Dictionary<IRStructType*, RefPtr<StructTypeInfo>> m_structTypeInfos;

    RefPtr<StructTypeInfo> createStructTypeInfo(IRStructType* structType)
    {
        RefPtr<StructTypeInfo> typeInfo = new StructTypeInfo();
        Index index = 0;
        for (auto field : structType->getFields())
        {
            typeInfo->structFieldIndices[field->getKey()] = index;
            index++;
        }
        return typeInfo;
    }
    Index getStructFieldId(IRStructType* structType, IRStructKey* structFieldKey)
    {
        RefPtr<StructTypeInfo> info;
        if (!m_structTypeInfos.TryGetValue(structType, info))
        {
            info = createStructTypeInfo(structType);
            m_structTypeInfos[structType] = info;
        }
        Index fieldIndex = -1;
        info->structFieldIndices.TryGetValue(structFieldKey, fieldIndex);
        SLANG_ASSERT(fieldIndex != -1);
        return fieldIndex;
    }

    SpvInst* emitFieldAddress(SpvInstParent* parent, IRFieldAddress* fieldAddress)
    {
        IRBuilder builder(m_sharedIRBuilder);
        builder.setInsertBefore(fieldAddress);

        auto base = fieldAddress->getBase();
        SpvWord baseId = 0;
        IRStructType* baseStructType = nullptr;

        if (auto ptrLikeType = as<IRPointerLikeType>(base->getDataType()))
        {
            baseStructType = as<IRStructType>(ptrLikeType->getElementType());
            baseId = getID(ensureInst(base));
        }
        else if (auto ptrType = as<IRPtrTypeBase>(base->getDataType()))
        {
            baseStructType = as<IRStructType>(ptrType->getValueType());
            baseId = getID(ensureInst(base));
        }
        else
        {
            baseStructType = as<IRStructType>(base->getDataType());
            
            auto structPtrType = builder.getPtrType(baseStructType);
            auto varInst = emitInst(
                parent, nullptr, SpvOpVariable, structPtrType, kResultID, SpvStorageClassFunction);
            emitInst(parent, nullptr, SpvOpStore, varInst, base);
            baseId = getID(varInst);
        }
        SLANG_ASSERT(baseStructType && "field_address require base to be a struct.");
        auto fieldId = emitIntConstant(
            getStructFieldId(baseStructType, as<IRStructKey>(fieldAddress->getField())),
            builder.getIntType());
        return emitInst(
            parent,
            fieldAddress,
            SpvOpAccessChain,
            fieldAddress->getFullType(),
            kResultID,
            baseId,
            fieldId);
    }

    SpvInst* emitFieldExtract(SpvInstParent* parent, IRFieldExtract* inst)
    {
        IRBuilder builder(m_sharedIRBuilder);
        builder.setInsertBefore(inst);

        IRStructType* baseStructType = as<IRStructType>(inst->getBase()->getDataType());
        SLANG_ASSERT(baseStructType && "field_extract require base to be a struct.");
        auto fieldId = emitIntConstant(
            getStructFieldId(baseStructType, as<IRStructKey>(inst->getField())),
            builder.getIntType());
        
        return emitInst(
            parent,
            inst,
            SpvOpCompositeExtract,
            inst->getDataType(),
            kResultID,
            inst->getBase(),
            fieldId);
    }

    SpvInst* emitGetElementPtr(SpvInstParent* parent, IRGetElementPtr* inst)
    {
        auto base = inst->getBase();
        SpvWord baseId = 0;
        IRArrayType* baseArrayType = nullptr;
        // Only used in debug build, but we don't want a warning/error for an unused initialized variable
        SLANG_UNUSED(baseArrayType);

        if (auto ptrLikeType = as<IRPointerLikeType>(base->getDataType()))
        {
            baseArrayType = as<IRArrayType>(ptrLikeType->getElementType());
            baseId = getID(ensureInst(base));
        }
        else if (auto ptrType = as<IRPtrTypeBase>(base->getDataType()))
        {
            baseArrayType = as<IRArrayType>(ptrType->getValueType());
            baseId = getID(ensureInst(base));
        }
        else
        {
            SLANG_ASSERT(!"invalid IR: base of getElementPtr must be a pointer.");
        }
        SLANG_ASSERT(baseArrayType && "getElementPtr require base to be an array.");
        return emitInst(
            parent,
            inst,
            SpvOpAccessChain,
            inst->getFullType(),
            kResultID,
            baseId,
            inst->getIndex());
    }

    SpvInst* emitGetElement(SpvInstParent* parent, IRGetElement* inst)
    {
        auto base = inst->getBase();
        SpvWord baseId = 0;
        IRArrayType* baseArrayType = nullptr;
        // Only used in debug build, but we don't want a warning/error for an unused initialized variable
        SLANG_UNUSED(baseArrayType);

        if (auto ptrLikeType = as<IRPointerLikeType>(base->getDataType()))
        {
            baseArrayType = as<IRArrayType>(ptrLikeType->getElementType());
            baseId = getID(ensureInst(base));
        }
        else if (auto ptrType = as<IRPtrTypeBase>(base->getDataType()))
        {
            baseArrayType = as<IRArrayType>(ptrType->getValueType());
            baseId = getID(ensureInst(base));
        }
        else
        {
            SLANG_ASSERT(!"invalid IR: base of getElement must be a pointer.");
        }
        SLANG_ASSERT(baseArrayType && "getElement require base to be an array.");

        IRBuilder builder(m_sharedIRBuilder);
        builder.setInsertBefore(inst);

        auto ptr = emitInst(
            parent,
            nullptr,
            SpvOpAccessChain,
            builder.getPtrType(inst->getFullType()),
            kResultID,
            baseId,
            inst->getIndex());
        return emitInst(parent, inst, SpvOpLoad, inst->getFullType(), kResultID, ptr);
    }

    SpvInst* emitLoad(SpvInstParent* parent, IRLoad* inst)
    {
        return emitInst(parent, inst, SpvOpLoad, inst->getDataType(), kResultID, inst->getPtr());
    }

    SpvInst* emitStore(SpvInstParent* parent, IRStore* inst)
    {
        return emitInst(parent, inst, SpvOpStore, inst->getPtr(), inst->getVal());
    }

    SpvInst* emitSwizzle(SpvInstParent* parent, IRSwizzle* inst)
    {
        if (inst->getElementCount() == 1)
        {
            return emitInst(
                parent,
                inst,
                SpvOpCompositeExtract,
                inst->getDataType(),
                kResultID,
                inst->getBase(),
                (SpvWord)as<IRIntLit>(inst->getElementIndex(0))->getValue());
        }
        else
        {
            return emitInstCustomOperandFunc(parent, inst, SpvOpVectorShuffle, [&]() {
                emitOperand(inst->getDataType());
                emitOperand(kResultID);
                emitOperand(inst->getBase());
                emitOperand(inst->getBase());
                for (UInt i = 0; i < inst->getElementCount(); i++)
                {
                    auto index = as<IRIntLit>(inst->getElementIndex(i));
                    emitOperand((SpvWord)index->getValue());
                }
            });
        }
    }

    SpvInst* emitConstruct(SpvInstParent* parent, IRInst* inst)
    {
        if (as<IRBasicType>(inst->getDataType()))
        {
            if (inst->getOperandCount() == 1)
            {
                if (inst->getDataType() == inst->getOperand(0)->getDataType())
                    return emitInst(
                        parent,
                        inst,
                        SpvOpCopyObject,
                        inst->getFullType(),
                        kResultID,
                        inst->getOperand(0));
                else
                    return emitInst(
                        parent,
                        inst,
                        SpvOpBitcast,
                        inst->getFullType(),
                        kResultID,
                        inst->getOperand(0));
            }
            else
            {
                SLANG_ASSERT(!"spirv emit: unsupported Construct inst.");
                return nullptr;
            }
        }
        else
        {
            return emitInst(
                parent,
                inst,
                SpvOpCompositeConstruct,
                inst->getDataType(),
                kResultID,
                OperandsOf(inst));
        }
    }

    bool isSignedType(IRType* type)
    {
        switch (type->getOp())
        {
        case kIROp_FloatType:
        case kIROp_DoubleType:
            return true;
        case kIROp_IntType:
        case kIROp_Int16Type:
        case kIROp_Int64Type:
        case kIROp_Int8Type:
            return true;
        case kIROp_VectorType:
            return isSignedType(as<IRVectorType>(type)->getElementType());
        case kIROp_MatrixType:
            return isSignedType(as<IRMatrixType>(type)->getElementType());
        default:
            return false;
        }
    }

    bool isFloatType(IRInst* type)
    {
        switch (type->getOp())
        {
        case kIROp_FloatType:
        case kIROp_DoubleType:
        case kIROp_HalfType:
            return true;
        case kIROp_VectorType:
            return isFloatType(as<IRVectorType>(type)->getElementType());
        case kIROp_MatrixType:
            return isFloatType(as<IRMatrixType>(type)->getElementType());
        default:
            return false;
        }
    }

    SpvInst* emitArithmetic(SpvInstParent* parent, IRInst* inst)
    {
        IRType* elementType = inst->getOperand(0)->getDataType();
        if (auto vectorType = as<IRVectorType>(inst->getDataType()))
        {
            elementType = vectorType->getElementType();
        }
        else if (auto matrixType = as<IRMatrixType>(inst->getDataType()))
        {
            //TODO: implement.
            SLANG_ASSERT(!"unimplemented: matrix arithemetic");
        }
        IRBasicType* basicType = as<IRBasicType>(elementType);
        bool isFloatingPoint = false;
        bool isBool = false;
        switch (basicType->getBaseType())
        {
        case BaseType::Float:
        case BaseType::Double:
            isFloatingPoint = true;
            break;
        case BaseType::Bool:
            isBool = true;
            break;
        default:
            break;
        }
        SpvOp opCode = SpvOpUndef;
        bool isSigned = isSignedType(basicType);
        switch (inst->getOp())
        {
        case kIROp_Add:
            opCode = isFloatingPoint ? SpvOpFAdd : SpvOpIAdd;
            break;
        case kIROp_Sub:
            opCode = isFloatingPoint ? SpvOpFSub : SpvOpISub;
            break;
        case kIROp_Mul:
            opCode = isFloatingPoint ? SpvOpFMul : SpvOpIMul;
            break;
        case kIROp_Div:
            opCode = isFloatingPoint ? SpvOpFDiv : isSigned ? SpvOpSDiv : SpvOpUDiv;
            break;
        case kIROp_IRem:
            opCode = isSigned ? SpvOpSRem : SpvOpUMod;
            break;
        case kIROp_FRem:
            opCode = SpvOpFRem;
            break;
        case kIROp_Less:
            opCode = isFloatingPoint ? SpvOpFOrdLessThan
                                     : isSigned ? SpvOpSLessThan : SpvOpULessThan;
            break;
        case kIROp_Leq:
            opCode = isFloatingPoint ? SpvOpFOrdLessThanEqual
                                     : isSigned ? SpvOpSLessThanEqual : SpvOpULessThanEqual;
            break;
        case kIROp_Eql:
            opCode = isFloatingPoint ? SpvOpFOrdEqual : isBool ? SpvOpLogicalEqual : SpvOpIEqual;
            break;
        case kIROp_Neq:
            opCode = isFloatingPoint ? SpvOpFOrdNotEqual
                                     : isBool ? SpvOpLogicalNotEqual : SpvOpINotEqual;
            break;
        case kIROp_Geq:
            opCode = isFloatingPoint ? SpvOpFOrdGreaterThanEqual
                                     : isSigned ? SpvOpSGreaterThanEqual : SpvOpUGreaterThanEqual;
            break;
        case kIROp_Greater:
            opCode = isFloatingPoint ? SpvOpFOrdGreaterThan
                                     : isSigned ? SpvOpSGreaterThan : SpvOpUGreaterThan;
            break;
        case kIROp_Neg:
            opCode = isFloatingPoint ? SpvOpFNegate : SpvOpSNegate;
            break;
        case kIROp_And:
            opCode = SpvOpLogicalAnd;
            break;
        case kIROp_Or:
            opCode = SpvOpLogicalOr;
            break;
        case kIROp_Not:
            opCode = SpvOpLogicalNot;
            break;
        case kIROp_BitAnd:
            opCode = SpvOpBitwiseAnd;
            break;
        case kIROp_BitOr:
            opCode = SpvOpBitwiseOr;
            break;
        case kIROp_BitXor:
            opCode = SpvOpBitwiseXor;
            break;
        case kIROp_BitNot:
            opCode = SpvOpBitReverse;
            break;
        case kIROp_Rsh:
            opCode = isSigned ? SpvOpShiftRightArithmetic : SpvOpShiftRightLogical;
            break;
        case kIROp_Lsh:
            opCode = SpvOpShiftLeftLogical;
            break;
        default:
            SLANG_ASSERT(!"unknown arithmetic opcode");
            break;
        }
        return emitInst(parent, inst, opCode, inst->getDataType(), kResultID, OperandsOf(inst));
    }

    OrderedHashSet<SpvCapability> m_capabilities;

    void requireSPIRVCapability(SpvCapability capability)
    {
        if (m_capabilities.Add(capability))
        {
            emitInst(
                getSection(SpvLogicalSectionID::Capabilities),
                nullptr,
                SpvOpCapability,
                capability);
        }
    }

    void handleRequiredCapabilitiesImpl(IRInst* inst)
    {
        // TODO: declare required SPV capabilities.

        for (auto decoration : inst->getDecorations())
        {
            switch (decoration->getOp())
            {
            default:
                break;

            case kIROp_RequireGLSLExtensionDecoration:
                {
                    break;
                }
            case kIROp_RequireGLSLVersionDecoration:
                {
                    break;
                }
            case kIROp_RequireSPIRVVersionDecoration:
                {
                    break;
                }
            }
        }
    }

    void diagnoseUnhandledInst(IRInst* inst)
    {
        m_sink->diagnose(
            inst, Diagnostics::unimplemented, "unexpected IR opcode during code emit");
    }

    SPIRVEmitContext(IRModule* module, TargetRequest* target, DiagnosticSink* sink)
        : SPIRVEmitSharedContext(module, target)
        , m_irModule(module)
        , m_sink(sink)
        , m_memoryArena(2048)
    {
    }
};

SlangResult emitSPIRVFromIR(
    CodeGenContext*         codeGenContext,
    IRModule*               irModule,
    const List<IRFunc*>&    irEntryPoints,
    List<uint8_t>&          spirvOut)
{
    spirvOut.clear();

    auto targetRequest = codeGenContext->getTargetReq();
    auto sink = codeGenContext->getSink();

    SPIRVEmitContext context(irModule, targetRequest, sink);
    legalizeIRForSPIRV(&context, irModule, irEntryPoints, sink);

    context.emitFrontMatter();
    for (auto irEntryPoint : irEntryPoints)
    {
        context.ensureInst(irEntryPoint);
    }
    context.emitPhysicalLayout();

    spirvOut.addRange(
        (uint8_t const*) context.m_words.getBuffer(),
        context.m_words.getCount() * sizeof(context.m_words[0]));

    return SLANG_OK;
}


} // namespace Slang
