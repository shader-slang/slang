// slang-emit-spirv.cpp

#include "slang-compiler.h"
#include "slang-emit-base.h"

#include "slang-ir-util.h"
#include "slang-ir.h"
#include "slang-ir-insts.h"
#include "slang-ir-layout.h"
#include "slang-ir-spirv-snippet.h"
#include "slang-ir-spirv-legalize.h"
#include "slang-spirv-val.h"
#include "spirv/unified1/spirv.h"
#include "../core/slang-memory-arena.h"
#include <type_traits>

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
    ConstantsAndTypes,
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

// A structure which can hold an integer literal, either one word or several
struct SpvLiteralInteger
{
    static SpvLiteralInteger from32(int32_t value) { return from32(uint32_t(value)); }
    static SpvLiteralInteger from32(uint32_t value) { return SpvLiteralInteger{{value}}; }
    static SpvLiteralInteger from64(int64_t value) { return from64(uint64_t(value)); }
    static SpvLiteralInteger from64(uint64_t value) { return SpvLiteralInteger{{SpvWord(value), SpvWord(value >> 32)}}; }
    List<SpvWord> value; // Words, stored low words to high (TODO, SmallArray or something here)
};

// A structure which can hold bitwise literal, either one word or several
struct SpvLiteralBits
{
    static SpvLiteralBits from32(uint32_t value) { return SpvLiteralBits{{value}}; }
    static SpvLiteralBits from64(uint64_t value) { return SpvLiteralBits{{SpvWord(value), SpvWord(value >> 32)}}; }
    List<SpvWord> value; // Words, stored low words to high (TODO, SmallArray or something here)
};

// As a convenience, there are often cases where
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

/// Helper type for not emitting an operand in this position
struct SkipThisOptionalOperand {};

template<typename T>
struct OptionalOperand
{
    static_assert(std::is_trivial_v<T>);
    OptionalOperand(SkipThisOptionalOperand) : present(false) {}
    OptionalOperand(T value) : present(true), value(value) {}
    bool present;
    T value;
};

template<typename T>
OptionalOperand<T> nullOptionOperand()
{
    return OptionalOperand<T>{false};
}

template<typename T>
OptionalOperand<T> someOptionOperand(T t)
{
    return OptionalOperand<T>{true, t};
}

template<typename T>
constexpr bool isPlural = false;
template<typename T>
constexpr bool isPlural<List<T>> = true;
template<typename T>
constexpr bool isPlural<IROperandList<T>> = true;
template<typename T, Index N>
constexpr bool isPlural<Array<T, N>> = true;
template<>
constexpr bool isPlural<OperandsOf> = true;
template<>
constexpr bool isPlural<IRUse*> = true;
template<typename T>
constexpr bool isSingular = !isPlural<T>;

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
        m_mapIRInstToSpvInst.add(irInst, spvInst);

        // If we have reserved an SpvID for `irInst`, make sure to use it.
        SpvWord reservedID = 0;
        m_mapIRInstToSpvID.tryGetValue(irInst, reservedID);

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
        if (m_mapIRInstToSpvInst.tryGetValue(inst, spvInst))
            return getID(spvInst);
        // Check if we have reserved an ID for `inst`.
        SpvWord result = 0;
        if (m_mapIRInstToSpvID.tryGetValue(inst, result))
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
    // The current instruction being constructed. Cannot add operands unless it
    // is set, or we are peeking at some operands to see if we have them memoized
    SpvInst* m_currentInst = nullptr;
    bool m_peekingOperands = false;

    // Operands can only be added when inside of a InstConstructScope or...
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

    // ...If we're speculatively adding them to see if we have a memoized results
    struct OperandMemoizeScope
    {
        OperandMemoizeScope(SPIRVEmitContext* context) : m_context(context)
        {
            m_tmpOperandStack.swapWith(m_context->m_operandStack);
            std::swap(m_tmpPeeking, m_context->m_peekingOperands);
            std::swap(m_tmpInst, m_context->m_currentInst);
        }
        ~OperandMemoizeScope()
        {
            std::swap(m_tmpInst, m_context->m_currentInst);
            std::swap(m_tmpPeeking, m_context->m_peekingOperands);
            m_tmpOperandStack.swapWith(m_context->m_operandStack);
        }

        SPIRVEmitContext* m_context;
        List<SpvWord> m_tmpOperandStack;
        bool m_tmpPeeking = true;
        SpvInst* m_tmpInst = nullptr;
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
        if (!m_mapIRInstToSpvInst.tryGetValue(irInst, spvInst))
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
        SLANG_ASSERT(m_currentInst || m_peekingOperands);
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
        SLANG_ASSERT(m_currentInst || m_peekingOperands);
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
        // This is the one case we shouldn't be peeking at operands, as it
        // depends on having an instruction under construction
        SLANG_ASSERT(m_currentInst);

        // A result <id> operand uses the <id> of the instruction itself (which is m_currentInst)
        emitOperand(getID(m_currentInst));
    }

    void emitOperand(const SpvLiteralBits& bits)
    {
        for(const auto v : bits.value)
            emitOperand(v);
    }

    void emitOperand(const SpvLiteralInteger& integer)
    {
        for(const auto v : integer.value)
            emitOperand(v);
    }

    template<typename T>
    void emitOperand(const List<T>& os)
    {
        for(const auto& o : os)
            emitOperand(o);
    }

    template<typename T>
    void emitOperand(const IROperandList<T>& os)
    {
        for(const auto& o : os)
            emitOperand(o);
    }

    template<typename T, Index N>
    void emitOperand(const Array<T, N>& os)
    {
        for(const auto& o : os)
            emitOperand(o);
    }

    template<typename T>
    void emitOperand(const ArrayView<T>& os)
    {
        for(const auto& o : os)
            emitOperand(o);
    }

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
    SpvInst* emitIntConstant(IRIntegerValue val, IRType* type, IRInst* inst = nullptr)
    {
        ConstantValueKey<IRIntegerValue> key;
        key.value = val;
        key.type = type;
        SpvInst* result = nullptr;
        if (m_spvIntConstants.tryGetValue(key, result))
            return result;
        switch (type->getOp())
        {
        case kIROp_Int64Type:
        case kIROp_UInt64Type:
#if SLANG_PTR_IS_64
        case kIROp_PtrType:
        case kIROp_UIntPtrType:
#endif
        {
            result = emitOpConstant(
                inst,
                type,
                SpvLiteralBits::from64(uint64_t(val))
            );
            break;
        }
        default:
        {
            result = emitOpConstant(
                inst,
                type,
                SpvLiteralBits::from32(uint32_t(val))
            );
            break;
        }
        }
        m_spvIntConstants[key] = result;
        return result;
    }
    SpvInst* emitFloatConstant(IRFloatingPointValue val, IRType* type, IRInst* inst = nullptr)
    {
        ConstantValueKey<IRFloatingPointValue> key;
        key.value = val;
        key.type = type;
        SpvInst* result = nullptr;
        if (m_spvFloatConstants.tryGetValue(key, result))
            return result;
        if (type->getOp() == kIROp_DoubleType)
        {
            result = emitOpConstant(
                inst,
                type,
                SpvLiteralBits::from64(uint64_t(DoubleAsInt64(val))));
        }
        else if(type->getOp() == kIROp_FloatType)
        {
            result = emitOpConstant(
                inst,
                type,
                SpvLiteralBits::from32(uint32_t(FloatAsInt(float(val)))));
        }
        else if(type->getOp() == kIROp_HalfType)
        {
            result = emitOpConstant(
                inst,
                type,
                SpvLiteralBits::from32(uint32_t(FloatToHalf(float(val)))));
        }
        else
        {
            SLANG_UNEXPECTED("missing case in SPIR-V emitFloatConstant");
        }
        m_spvFloatConstants[key] = result;
        return result;
    }

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

        /// Do nothing
    void emitOperand(SkipThisOptionalOperand) { }

    template<typename T>
    void emitOperand(OptionalOperand<T> o)
    {
        if(o.present)
            emitOperand(o.value);
    }

    // With the above routines, code can easily construct a SPIR-V
    // instruction with arbitrary operands over multiple lines of code.
    //
    // The safe way to call these routines is encoded in the below `emitInst`
    // function.
    //
    // This allows one to generically output a SPIR-V instruction with any
    // desired operands.
    //
    // This function performs no checks that it is actually being used
    // correctly with respect to the SPIR-V rules for each opcode. As such, a
    // more type safe function for each opcode is included in
    // 'slang-emit-spirv-ops.h', and available in this class. You are
    // encouraged to use these instead.
    //
    template<typename... Operands>
    SpvInst* emitInst(SpvInstParent* parent, IRInst* irInst, SpvOp opcode, const Operands& ...ops)
    {
        InstConstructScope scopeInst(this, opcode, irInst);
        SpvInst* spvInst = scopeInst;
        (emitOperand(ops), ...);
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

    // Emits a SPV Inst with deduplication
    // This is used where our IR doesn't guarantee uniqueness but SPIR-V
    // requires it
    template<typename... Operands>
    SpvInst* emitInstMemoized(
        SpvInstParent* parent,
        IRInst* irInst,
        SpvOp opcode,
        // We take the resultId here explicitly here to make sure we don't try
        // and memoize its value.
        ResultIDToken resultId,
        const Operands& ...ops
    )
    {
        List<SpvWord> ourOperands;
        {
            auto scopePeek = OperandMemoizeScope(this);
            (emitOperand(ops), ...);
            // Steal our operands back, so we don't have to calculate them
            // again
            ourOperands = std::move(m_operandStack);
        }

        // Hash the whole global stack and opcode
        SpvTypeInstKey key;
        key.words.add(opcode);
        key.words.addRange(ourOperands);

        // If we have seen this before, return the memoized instruction
        if (SpvInst** memoized = m_spvTypeInsts.tryGetValue(key))
            return *memoized;

        // Otherwise, we can construct our instruction and record the result
        InstConstructScope scopeInst(this, opcode, irInst);
        SpvInst* spvInst = scopeInst;
        m_spvTypeInsts[key] = spvInst;

        // Emit our operands, this time with the resultId too
        emitOperand(resultId);
        m_operandStack.addRange(ourOperands);

        parent->addInst(spvInst);
        return spvInst;
    }

    //
    // Specific emit funcs
    //

#   define SLANG_IN_SPIRV_EMIT_CONTEXT
#   include "slang-emit-spirv-ops.h"
#   undef SLANG_IN_SPIRV_EMIT_CONTEXT

        /// The SPIRV OpExtInstImport inst that represents the GLSL450
        /// extended instruction set.
    SpvInst* m_glsl450ExtInst = nullptr;

    SpvInst* getGLSL450ExtInst()
    {
        if (m_glsl450ExtInst)
            return m_glsl450ExtInst;
        m_glsl450ExtInst = emitOpExtInstImport(
            getSection(SpvLogicalSectionID::ExtIntInstImports),
            nullptr,
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
        emitOpCapability(
            getSection(SpvLogicalSectionID::Capabilities),
            nullptr,
            SpvCapabilityShader
        );

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
        emitOpMemoryModel(
            getSection(SpvLogicalSectionID::MemoryModel),
            nullptr,
            SpvAddressingModelLogical,
            SpvMemoryModelGLSL450
        );
    }

    Dictionary<UnownedStringSlice, SpvInst*> m_extensionInsts;
    SpvInst* ensureExtensionDeclaration(UnownedStringSlice name)
    {
        SpvInst* result = nullptr;
        if (m_extensionInsts.tryGetValue(name, result))
            return result;
        result = emitOpExtension(
            getSection(SpvLogicalSectionID::Extensions),
            nullptr,
            name
        );
        m_extensionInsts[name] = result;
        return result;
    }

    struct SpvTypeInstKey
    {
        List<SpvWord> words;
        bool operator==(const SpvTypeInstKey& other) const { return words == other.words; }
        const static bool kHasUniformHash = true;
        auto getHashCode() const
        {
            return Slang::getHashCode(
                reinterpret_cast<const char*>(words.getBuffer()),
                words.getCount() * sizeof(SpvWord));
        }
    };

    Dictionary<SpvTypeInstKey, SpvInst*> m_spvTypeInsts;

    // Next, let's look at emitting some of the instructions
    // that can occur at global scope.

        /// Emit an instruction that is expected to appear at the global scope of the SPIR-V module.
        ///
        /// Returns the corresponding SPIR-V instruction.
        ///
    SpvInst* emitGlobalInst(IRInst* inst)
    {
        switch( inst->getOp() & kIROpMask_OpMask )
        {
        // [3.32.6: Type-Declaration Instructions]
        //

        case kIROp_VoidType: return emitOpTypeVoid(inst);
        case kIROp_BoolType: return emitOpTypeBool(inst);

        // > OpTypeInt

        case kIROp_UInt8Type:
        case kIROp_UInt16Type:
        case kIROp_UIntType:
        case kIROp_UInt64Type:
        case kIROp_Int8Type:
        case kIROp_Int16Type:
        case kIROp_IntType:
        case kIROp_Int64Type:
            {
                const IntInfo i = getIntTypeInfo(as<IRType>(inst));
                return emitOpTypeInt(
                    inst,
                    SpvLiteralInteger::from32(int32_t(i.width)),
                    SpvLiteralInteger::from32(i.isSigned)
                );
            }

        // > OpTypeFloat

        case kIROp_HalfType:
        case kIROp_FloatType:
        case kIROp_DoubleType:
            {
                const FloatInfo i = getFloatingTypeInfo(as<IRType>(inst));
                return emitOpTypeFloat(inst, SpvLiteralInteger::from32(int32_t(i.width)));
            }

        case kIROp_PtrType:
        case kIROp_RefType:
        case kIROp_OutType:
        case kIROp_InOutType:
            {
                SpvStorageClass storageClass = SpvStorageClassFunction;
                auto ptrType = as<IRPtrTypeBase>(inst);
                SLANG_ASSERT(ptrType);
                if (ptrType->hasAddressSpace())
                    storageClass = (SpvStorageClass)ptrType->getAddressSpace();
                if (storageClass == SpvStorageClassStorageBuffer)
                    ensureExtensionDeclaration(UnownedStringSlice("SPV_KHR_storage_buffer_storage_class"));
                return emitOpTypePointer(
                    inst,
                    storageClass,
                    inst->getOperand(0)
                );
            }
        case kIROp_ConstantBufferType:
            SLANG_UNEXPECTED("Constant buffer type remaining in spirv emit");
        case kIROp_StructType:
            {
                List<IRType*> types;
                // TODO: decorate offset
                for (auto field : static_cast<IRStructType*>(inst)->getFields())
                    types.add(field->getFieldType());
                auto spvStructType = emitOpTypeStruct(
                    inst,
                    types
                );
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
                const auto columnCount = static_cast<IRIntLit*>(matrixType->getColumnCount())->getValue();
                auto matrixSPVType = emitOpTypeMatrix(
                    inst,
                    vectorSpvType,
                    SpvLiteralInteger::from32(int32_t(columnCount))
                );
                // TODO: properly compute matrix stride.
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
                // TODO: This decoration is not legal here. It must be placed
                // on a struct member (which may entail wrapping matrices)
                emitOpDecorate(
                    getSection(SpvLogicalSectionID::Annotations),
                    nullptr,
                    matrixSPVType,
                    SpvDecorationRowMajor);
                emitOpDecorateMatrixStride(
                    getSection(SpvLogicalSectionID::Annotations),
                    nullptr,
                    matrixSPVType,
                    SpvLiteralInteger::from32(stride));
                return matrixSPVType;
            }
        case kIROp_ArrayType:
        case kIROp_UnsizedArrayType:
            {
                const auto elementType = static_cast<IRArrayTypeBase*>(inst)->getElementType();
                const auto arrayType = inst->getOp() == kIROp_ArrayType
                    ? emitOpTypeArray(inst, elementType, static_cast<IRArrayTypeBase*>(inst)->getElementCount())
                    : emitOpTypeRuntimeArray(inst, elementType);
                // TODO: properly decorate stride.
                // TODO: don't do this more than once
                IRSizeAndAlignment sizeAndAlignment;
                getNaturalSizeAndAlignment(this->m_targetRequest, elementType, &sizeAndAlignment);
                emitOpDecorateArrayStride(
                    getSection(SpvLogicalSectionID::Annotations),
                    nullptr,
                    arrayType,
                    SpvLiteralInteger::from32(int32_t(sizeAndAlignment.getStride())));
                return arrayType;
            }

        case kIROp_TextureType:
            {
                const auto texTypeInst = as<IRTextureType>(inst);
                const auto sampledType = texTypeInst->getElementType();
                SpvDim dim = SpvDim1D; // Silence uninitialized warnings from msvc...
                switch(texTypeInst->GetBaseShape())
                {
                    case TextureFlavor::Shape1D:
                    case TextureFlavor::Shape1DArray:
                        dim = SpvDim1D;
                        break;
                    case TextureFlavor::Shape2D:
                    case TextureFlavor::Shape2DArray:
                        dim = SpvDim2D;
                        break;
                    case TextureFlavor::Shape3D:
                        dim = SpvDim3D;
                        break;
                    case TextureFlavor::ShapeCube:
                    case TextureFlavor::ShapeCubeArray:
                        dim = SpvDimCube;
                        break;
                    case TextureFlavor::ShapeBuffer:
                        dim = SpvDimBuffer;
                        break;
                }
                bool arrayed = texTypeInst->isArray();
                SpvWord depth = 2; // No knowledge of if this is a depth image
                bool ms = texTypeInst->isMultisample();
                // TODO: can we do better here?
                SpvWord sampled = 0; // Only known at run time
                // TODO: can we do better?
                SpvImageFormat format = SpvImageFormatUnknown;
                return emitOpTypeImage(
                    inst,
                    sampledType,
                    dim,
                    SpvLiteralInteger::from32(depth),
                    SpvLiteralInteger::from32(arrayed),
                    SpvLiteralInteger::from32(ms),
                    SpvLiteralInteger::from32(sampled),
                    format
                );
            }
        case kIROp_SamplerStateType:
                return emitOpTypeSampler(inst);
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
            return emitOpTypeFunction(
                inst,
                static_cast<IRFuncType*>(inst)->getResultType(),
                static_cast<IRFuncType*>(inst)->getParamTypes()
            );

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

        case kIROp_Specialize:
            {
                const auto s = as<IRSpecialize>(inst);
                const auto g = s->getBase();
                const auto e =
                    "Specialize instruction remains in IR for SPIR-V emit, is something undefined?\n" +
                    dumpIRToString(g);
                SLANG_UNEXPECTED(e.getBuffer());
            }
        default:
            {
                String e = "Unhandled global inst in spirv-emit:\n"
                    + dumpIRToString(inst, {IRDumpOptions::Mode::Detailed, 0});
                SLANG_UNIMPLEMENTED_X(e.begin());
            }
        }
    }

    // Ensures an SpvInst for the specified vector type is emitted.
    // `inst` represents an optional `IRVectorType` inst representing the vector type, if
    // it is nullptr, this function will create one.
    SpvInst* ensureVectorType(BaseType baseType, IRIntegerValue elementCount, IRVectorType* inst)
    {
        if (!inst)
        {
            IRBuilder builder(m_irModule);
            builder.setInsertInto(m_irModule->getModuleInst());
            inst = builder.getVectorType(
                builder.getBasicType(baseType),
                builder.getIntValue(builder.getIntType(), elementCount));
        }
        auto result = emitOpTypeVector(
            inst,
            inst->getElementType(),
            SpvLiteralInteger::from32(int32_t(elementCount))
        );
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
                emitOpDecorateLocation(
                    getSection(SpvLogicalSectionID::Annotations),
                    nullptr,
                    varInst,
                    SpvLiteralInteger::from32(int32_t(index))
                );
                emitOpDecorateIndex(
                    getSection(SpvLogicalSectionID::Annotations),
                    nullptr,
                    varInst,
                    SpvLiteralInteger::from32(int32_t(space))
                );
                break;
            case LayoutResourceKind::VaryingOutput:
                emitOpDecorateLocation(
                    getSection(SpvLogicalSectionID::Annotations),
                    nullptr,
                    varInst,
                    SpvLiteralInteger::from32(int32_t(index))
                );
                if (space)
                {
                    emitOpDecorateIndex(
                        getSection(SpvLogicalSectionID::Annotations),
                        nullptr,
                        varInst,
                        SpvLiteralInteger::from32(int32_t(space))
                    );
                }
                break;

            case LayoutResourceKind::SpecializationConstant:
                emitOpDecorateSpecId(
                    getSection(SpvLogicalSectionID::Annotations),
                    nullptr,
                    varInst,
                    SpvLiteralInteger::from32(int32_t(index))
                );
                break;

            case LayoutResourceKind::ConstantBuffer:
            case LayoutResourceKind::ShaderResource:
            case LayoutResourceKind::UnorderedAccess:
            case LayoutResourceKind::SamplerState:
            case LayoutResourceKind::DescriptorTableSlot:
                emitOpDecorateBinding(
                    getSection(SpvLogicalSectionID::Annotations),
                    nullptr,
                    varInst,
                    SpvLiteralInteger::from32(int32_t(index))
                );
                emitOpDecorateDescriptorSet(
                    getSection(SpvLogicalSectionID::Annotations),
                    nullptr,
                    varInst,
                    SpvLiteralInteger::from32(int32_t(space))
                );
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
        auto varInst = emitOpVariable(
            getSection(SpvLogicalSectionID::GlobalVariables),
            param,
            param->getDataType(),
            storageClass
        );
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
        auto varInst = emitOpVariable(
            getSection(SpvLogicalSectionID::GlobalVariables),
            globalVar,
            globalVar->getDataType(),
            storageClass
        );
        if(layout)
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
        if(!irFunc->getFirstBlock())
            m_sink->diagnose(irFunc, Diagnostics::noBlocksOrIntrinsic, "spirv");

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
        SpvInst* spvFunc = emitOpFunction(
            section,
            irFunc,
            irFunc->getDataType()->getResultType(),
            spvFunctionControl,
            irFunc->getDataType()
        );

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
            auto spvBlock = emitOpLabel(spvFunc, irBlock);
            if (irBlock == irFunc->getFirstBlock())
            {
                // OpVariable
                // All variables used in the function must be declared before anything else.
                for (auto block : irFunc->getBlocks())
                {
                    for (auto inst : block->getChildren())
                    {
                        if (as<IRVar>(inst))
                            emitLocalInst(spvBlock, inst);
                    }
                }
            }

            // In addition to normal basic blocks,
            // all loops gets a header block.
            for (auto irInst : irBlock->getChildren())
            {
                if (irInst->getOp() == kIROp_loop)
                {
                    emitOpLabel(spvFunc, irInst);
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
            m_mapIRInstToSpvInst.tryGetValue(irBlock, spvBlock);
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
                // Skip vars because they are already emitted.
                if (as<IRVar>(irInst))
                    continue;
                emitLocalInst(spvBlock, irInst);
                if (irInst->getOp() == kIROp_loop)
                    pendingLoopInsts.add(as<IRLoop>(irInst));
            }
        }

        // Finally, we generate the body of loop header blocks.
        for (auto loopInst : pendingLoopInsts)
        {
            SpvInst* headerBlock = nullptr;
            m_mapIRInstToSpvInst.tryGetValue(loopInst, headerBlock);
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
        emitOpFunctionEnd(spvFunc, nullptr);

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
    // When emitting global instructions, we usually have to
    // pick the right logical section to emit them into, while
    // for local instructions they will usually emit into
    // a known parent (the basic block that contains them).

        /// Emit an instruction that is local to the body of the given `parent`.
    SpvInst* emitLocalInst(SpvInstParent* parent, IRInst* inst)
    {
        switch( inst->getOp() )
        {
        default:
            {
                String e = "Unhandled local inst in spirv-emit:\n"
                    + dumpIRToString(inst, {IRDumpOptions::Mode::Detailed, 0});
                SLANG_UNIMPLEMENTED_X(e.getBuffer());
            }
        case kIROp_Specialize:
            return nullptr;
        case kIROp_Var:
            return emitVar(parent, inst);
        case kIROp_Call:
            return emitCall(parent, static_cast<IRCall*>(inst));
        case kIROp_FieldAddress:
            return emitFieldAddress(parent, as<IRFieldAddress>(inst));
        case kIROp_FieldExtract:
            return emitFieldExtract(parent, as<IRFieldExtract>(inst));
        case kIROp_GetElementPtr:
            return emitGetElementPtr(parent, as<IRGetElementPtr>(inst));
        case kIROp_GetElement:
            return emitGetElement(parent, as<IRGetElement>(inst));
        case kIROp_MakeStruct:
            return emitCompositeConstruct(parent, inst);
        case kIROp_Load:
            return emitLoad(parent, as<IRLoad>(inst));
        case kIROp_Store:
            return emitStore(parent, as<IRStore>(inst));
        case kIROp_StructuredBufferLoad:
        case kIROp_StructuredBufferLoadStatus:
        case kIROp_RWStructuredBufferLoad:
        case kIROp_RWStructuredBufferLoadStatus:
            return emitStructuredBufferLoad(parent, inst);
        case kIROp_RWStructuredBufferStore:
            return emitStructuredBufferStore(parent, inst);
        case kIROp_RWStructuredBufferGetElementPtr:
            return emitStructuredBufferGetElementPtr(parent, inst);
        case kIROp_swizzle:
            return emitSwizzle(parent, as<IRSwizzle>(inst));
        case kIROp_IntCast:
            return emitIntCast(parent, as<IRIntCast>(inst));
        case kIROp_FloatCast:
            return emitFloatCast(parent, as<IRFloatCast>(inst));
        case kIROp_CastIntToFloat:
            return emitIntToFloatCast(parent, as<IRCastIntToFloat>(inst));
        case kIROp_CastFloatToInt:
            return emitFloatToIntCast(parent, as<IRCastFloatToInt>(inst));
        case kIROp_MatrixReshape:
        case kIROp_VectorReshape:
            // TODO: break emitConstruct into separate functions for each opcode.
            return emitConstruct(parent, inst);
        case kIROp_BitCast:
            return emitOpBitcast(
                parent,
                inst,
                inst->getDataType(),
                inst->getOperand(0)
            );
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
                return emitOpReturn(parent, inst);
            else
                return emitOpReturnValue(parent, inst, as<IRReturn>(inst)->getVal());
        case kIROp_discard:
            return emitOpKill(parent, inst);
        case kIROp_unconditionalBranch:
            {
                // If we are jumping to the main block of a loop,
                // emit a branch to the loop header instead.
                // The SPV id of the resulting loop header block is associated with the loop inst.
                auto targetBlock = as<IRUnconditionalBranch>(inst)->getTargetBlock();
                IRInst* loopInst = nullptr;
                if (isLoopTargetBlock(targetBlock, loopInst))
                    return emitOpBranch(parent, inst, getIRInstSpvID(loopInst));
                // Otherwise, emit a normal branch inst into the target block.
                return emitOpBranch(parent, inst, getIRInstSpvID(targetBlock));
            }
        case kIROp_loop:
            {
                // Return loop header block in its own block.
                auto blockId = getIRInstSpvID(inst);
                SpvInst* block = nullptr;
                m_mapIRInstToSpvInst.tryGetValue(inst, block);
                SLANG_ASSERT(block);

                // Emit a jump to the loop header block.
                // Note: the body of the loop header block is emitted
                // after everything else to ensure Phi instructions (which come
                // from the actual loop target block) are emitted first.
                emitOpBranch(parent, nullptr, blockId);
        
                return block;
            }
        case kIROp_ifElse:
            {
                auto ifelseInst = as<IRIfElse>(inst);
                auto afterBlockID = getIRInstSpvID(ifelseInst->getAfterBlock());
                emitOpSelectionMerge(parent, nullptr, afterBlockID, SpvSelectionControlMaskNone);
                auto falseLabel = ifelseInst->getFalseBlock();
                return emitOpBranchConditional(
                    parent,
                    inst,
                    ifelseInst->getCondition(),
                    ifelseInst->getTrueBlock(),
                    falseLabel ? getID(ensureInst(falseLabel)) : afterBlockID,
                    makeArray<SpvLiteralInteger>()
                );
            }
        case kIROp_Switch:
            {
                auto switchInst = as<IRSwitch>(inst);
                auto mergeBlockID = getIRInstSpvID(switchInst->getBreakLabel());
                emitOpSelectionMerge(parent, nullptr, mergeBlockID, SpvSelectionControlMaskNone);
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
            return emitOpUnreachable(parent, inst);
        case kIROp_conditionalBranch:
            SLANG_UNEXPECTED("Unstructured branching is not supported by SPIRV.");
        case kIROp_MakeVector:
            return emitConstruct(parent, inst);
        case kIROp_MakeVectorFromScalar:
            {
                const auto scalar = inst->getOperand(0);
                const auto vecTy = as<IRVectorType>(inst->getDataType());
                SLANG_ASSERT(vecTy);
                const auto numElems = as<IRIntLit>(vecTy->getElementCount());
                SLANG_ASSERT(numElems);
                return emitSplat(parent, inst, scalar, numElems->getValue());
            }
        case kIROp_MakeArray:
            return emitConstruct(parent, inst);
        }
    }

    SpvInst* emitLit(IRInst* inst)
    {
        switch (inst->getOp())
        {
        case kIROp_IntLit:
            {
                auto value = as<IRIntLit>(inst)->getValue();
                return emitIntConstant(value, inst->getDataType(), inst);
            }
        case kIROp_FloatLit:
            {
                const auto value = as<IRConstant>(inst)->value.floatVal;
                const auto type = inst->getDataType();
                return emitFloatConstant(value, type, inst);
            }
        case kIROp_BoolLit:
            {
                if (cast<IRBoolLit>(inst)->getValue())
                {
                    return emitOpConstantTrue(
                        inst,
                        inst->getDataType()
                    );
                }
                else
                {
                    return emitOpConstantFalse(
                        inst,
                        inst->getDataType()
                    );
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

        case kIROp_LayoutDecoration:
            {
                // Basic offsets for structs used in buffers
                if(const auto typeLayout = as<IRTypeLayout>(as<IRLayoutDecoration>(decoration)->getLayout()))
                {
                    if(const auto structTypeLayout = as<IRStructTypeLayout>(typeLayout))
                    {
                        auto section = getSection(SpvLogicalSectionID::Annotations);
                        SpvWord i = 0;
                        for(const auto fieldLayoutAttr : structTypeLayout->getFieldLayoutAttrs())
                        {
                            if(const auto structFieldLayoutAttr = as<IRStructFieldLayoutAttr>(fieldLayoutAttr))
                            {
                                const auto varLayout = structFieldLayoutAttr->getLayout();
                                if(const auto varOffsetAttr = varLayout->findOffsetAttr(LayoutResourceKind::Uniform))
                                {
                                    const auto offset = static_cast<SpvWord>(varOffsetAttr->getOffset());
                                    emitOpMemberDecorateOffset(
                                        section,
                                        fieldLayoutAttr,
                                        dstID,
                                        SpvLiteralInteger::from32(i),
                                        SpvLiteralInteger::from32(offset)
                                    );
                                }
                            }
                            ++i;
                        }
                    }
                }
            }
            break;

        // [3.32.2. Debug Instructions]
        //
        // > OpName
        //
        case kIROp_NameHintDecoration:
            {
                auto section = getSection(SpvLogicalSectionID::DebugNames);
                auto nameHint = cast<IRNameHintDecoration>(decoration);
                // We can't associate this spirv instruction with our
                // irInstruction, our instruction may be a hint on several
                // values, however this decoration is specific to a single
                // dstID.
                emitOpName(section, nullptr, dstID, nameHint->getName());
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
                List<IRInst*> params;
                // `interface` part: reference all global variables that are used by this entrypoint.
                // TODO: we may want to perform more accurate tracking.
                for (auto globalInst : m_irModule->getModuleInst()->getChildren())
                {
                    switch (globalInst->getOp())
                    {
                    case kIROp_GlobalVar:
                    case kIROp_GlobalParam:
                        params.add(globalInst);
                        break;
                    }
                }
                emitOpEntryPoint(
                    section,
                    decoration,
                    spvStage,
                    dstID,
                    name,
                    params
                );
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
                emitOpExecutionModeLocalSize(
                    section,
                    decoration,
                    dstID,
                    SpvLiteralInteger::from32(int32_t(numThreads->getX()->getValue())),
                    SpvLiteralInteger::from32(int32_t(numThreads->getY()->getValue())),
                    SpvLiteralInteger::from32(int32_t(numThreads->getZ()->getValue()))
                );
            }
            break;

        case kIROp_SPIRVBufferBlockDecoration:
            {
                emitOpDecorate(
                    getSection(SpvLogicalSectionID::Annotations),
                    decoration,
                    dstID,
                    SpvDecorationBlock
                );
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
        if (m_builtinGlobalVars.tryGetValue(builtinVal, result))
        {
            return result;
        }
        IRBuilder builder(m_irModule);
        builder.setInsertBefore(type);
        auto ptrType = as<IRPtrTypeBase>(type);
        SLANG_ASSERT(ptrType && "`getBuiltinGlobalVar`: `type` must be ptr type.");
        auto varInst = emitOpVariable(
            getSection(SpvLogicalSectionID::GlobalVariables),
            nullptr,
            type,
            static_cast<SpvStorageClass>(ptrType->getAddressSpace())
        );
        emitOpDecorateBuiltIn(
            getSection(SpvLogicalSectionID::Annotations),
            nullptr,
            varInst,
            builtinVal
        );
        m_builtinGlobalVars[builtinVal] = varInst;
        return varInst;
    }

    SpvInst* maybeEmitSystemVal(IRInst* inst)
    {
        IRBuilder builder(m_irModule);
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
                else if (semanticName == "sv_groupindex")
                {
                    return getBuiltinGlobalVar(inst->getFullType(), SpvBuiltInLocalInvocationIndex);
                }
            }
        }
        return nullptr;
    }

    SpvInst* emitParam(SpvInstParent* parent, IRInst* inst)
    {
        return emitOpFunctionParameter(parent, inst, inst->getFullType());
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
        return emitOpVariable(parent, inst, inst->getFullType(), storageClass);
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
        if (m_mapIRBlockToParamIndexInfo.tryGetValue(block, info))
        {
            info->mapParamToIndex.tryGetValue(paramInst, result);
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
        SpvLoopControlMask loopControl = SpvLoopControlMaskNone;
        if (auto loopControlDecoration = loopInst->findDecoration<IRLoopControlDecoration>())
        {
            switch (loopControlDecoration->getMode())
            {
            case IRLoopControl::kIRLoopControl_Unroll:
                loopControl = SpvLoopControlUnrollMask;
                break;
            case IRLoopControl::kIRLoopControl_Loop:
                loopControl = SpvLoopControlDontUnrollMask;
                break;
            default:
                break;
            }
        }
        emitOpLoopMerge(
            loopHeaderBlock,
            nullptr,
            getIRInstSpvID(loopInst->getBreakBlock()),
            getIRInstSpvID(loopInst->getContinueBlock()),
            loopControl
        );
        emitOpBranch(loopHeaderBlock, nullptr, loopInst->getTargetBlock());
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
            m_mapIRInstToSpvInst.tryGetValue(loopInst, loopSpvBlockInst);
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

    SpvInst* emitCall(SpvInstParent* parent, IRCall* inst)
    {
        auto funcValue = inst->getCallee();

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
            return emitOpFunctionCall(
                parent,
                inst,
                inst->getFullType(),
                funcValue,
                inst->getArgsList()
            );
        }
    }

    SpvInst* emitIntrinsicCallExpr(
        SpvInstParent* parent,
        IRCall* inst,
        IRTargetIntrinsicDecoration* intrinsic)
    {
        SpvSnippet* snippet = getParsedSpvSnippet(intrinsic);
        SLANG_ASSERT(snippet);
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
        {
            IRBuilder builder(m_irModule);
            builder.setInsertBefore(inst);
            for (auto storageClass : snippet->usedPtrResultTypeStorageClasses)
            {
                auto newPtrType = builder.getPtrType(
                    kIROp_PtrType,
                    inst->getDataType(),
                    storageClass
                );
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
        if (m_spvSnippetConstantInsts.tryGetValue(constant, result))
            return result;

        IRBuilder builder(m_irModule);
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
                result = emitOpConstantComposite(
                    nullptr,
                    builder.getVectorType(floatType, builder.getIntValue(builder.getIntType(), 2)),
                    makeArray(element1, element2)
                );
            }
            break;
        case SpvSnippet::ASMType::Int:
            result = emitIntConstant((IRIntegerValue)constant.intValues[0], builder.getIntType());
            break;
        case SpvSnippet::ASMType::UInt2:
            {
                auto uintType = builder.getType(kIROp_UIntType);
                auto element1 = emitIntConstant((IRIntegerValue)constant.intValues[0], uintType);
                auto element2 = emitIntConstant((IRIntegerValue)constant.intValues[1], uintType);
                result = emitOpConstantComposite(
                    nullptr,
                    builder.getVectorType(uintType, builder.getIntValue(builder.getIntType(), 2)),
                    makeArray(element1, element2)
                );
            }
            break;
        }
        m_spvSnippetConstantInsts[constant] = result;
        return result;
    }

    // Emit SPV Inst that represents a type defined in a SpvSnippet.
    void emitSpvSnippetASMTypeOperand(SpvSnippet::ASMType type)
    {
        IRBuilder builder(m_irModule);
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
        case SpvSnippet::ASMType::UInt:
            irType = builder.getUIntType();
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
            SLANG_UNEXPECTED("unhandled case in emitSpvSnippetASMTypeOperand");
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
                    if (operand.content != 0xFFFFFFFF)
                    {
                        emitOperand(context.qualifiedResultTypes[(SpvStorageClass)operand.content]
                                        .getValue());
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
        if (!m_structTypeInfos.tryGetValue(structType, info))
        {
            info = createStructTypeInfo(structType);
            m_structTypeInfos[structType] = info;
        }
        Index fieldIndex = -1;
        info->structFieldIndices.tryGetValue(structFieldKey, fieldIndex);
        SLANG_ASSERT(fieldIndex != -1);
        return fieldIndex;
    }

    SpvInst* emitFieldAddress(SpvInstParent* parent, IRFieldAddress* fieldAddress)
    {
        IRBuilder builder(m_irModule);
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
            auto varInst = emitOpVariable(
                parent,
                nullptr,
                structPtrType,
                SpvStorageClassFunction
            );
            emitOpStore(parent, nullptr, varInst, base);
            baseId = getID(varInst);
        }
        SLANG_ASSERT(baseStructType && "field_address requires base to be a struct.");
        auto fieldId = emitIntConstant(
            getStructFieldId(baseStructType, as<IRStructKey>(fieldAddress->getField())),
            builder.getIntType());
        SLANG_ASSERT(as<IRPtrTypeBase>(fieldAddress->getFullType()));
        return emitOpAccessChain(
            parent,
            fieldAddress,
            fieldAddress->getFullType(),
            baseId,
            makeArray(fieldId)
        );
    }

    SpvInst* emitFieldExtract(SpvInstParent* parent, IRFieldExtract* inst)
    {
        IRBuilder builder(m_irModule);
        builder.setInsertBefore(inst);

        IRStructType* baseStructType = as<IRStructType>(inst->getBase()->getDataType());
        SLANG_ASSERT(baseStructType && "field_extract requires base to be a struct.");
        auto fieldId = static_cast<SpvWord>(getStructFieldId(
            baseStructType,
            as<IRStructKey>(inst->getField())));
        
        return emitOpCompositeExtract(
            parent,
            inst,
            inst->getDataType(),
            inst->getBase(),
            makeArray(SpvLiteralInteger::from32(fieldId))
        );
    }

    SpvInst* emitGetElementPtr(SpvInstParent* parent, IRGetElementPtr* inst)
    {
        auto base = inst->getBase();
        SpvWord baseId = 0;
        // Only used in debug build, but we don't want a warning/error for an unused initialized variable

        if (auto ptrLikeType = as<IRPointerLikeType>(base->getDataType()))
        {
            baseId = getID(ensureInst(base));
        }
        else if (auto ptrType = as<IRPtrTypeBase>(base->getDataType()))
        {
            baseId = getID(ensureInst(base));
        }
        else
        {
            SLANG_ASSERT(!"invalid IR: base of getElementPtr must be a pointer.");
        }
        SLANG_ASSERT(as<IRPtrTypeBase>(inst->getFullType()));
        return emitOpAccessChain(
            parent,
            inst,
            inst->getFullType(),
            baseId,
            makeArray(inst->getIndex())
        );
    }

    SpvInst* emitGetElement(SpvInstParent* parent, IRGetElement* inst)
    {
        auto base = inst->getBase();
        const auto baseTy = base->getDataType();
        SLANG_ASSERT(
            as<IRPointerLikeType>(baseTy) ||
            as<IRArrayType>(baseTy) ||
            as<IRVectorType>(baseTy) ||
            as<IRMatrixType>(baseTy));

        IRBuilder builder(m_irModule);
        builder.setInsertBefore(inst);

        auto ptr = emitOpAccessChain(
            parent,
            nullptr,
            builder.getPtrType(inst->getFullType()),
            inst->getBase(),
            makeArray(inst->getIndex())
        );
        return emitOpLoad(
            parent,
            inst,
            inst->getFullType(),
            ptr
        );
    }

    SpvInst* emitLoad(SpvInstParent* parent, IRLoad* inst)
    {
        return emitOpLoad(parent, inst, inst->getDataType(), inst->getPtr());
    }

    SpvInst* emitStore(SpvInstParent* parent, IRStore* inst)
    {
        return emitOpStore(parent, inst, inst->getPtr(), inst->getVal());
    }

    SpvInst* emitStructuredBufferLoad(SpvInstParent* parent, IRInst* inst)
    {
        //"%addr = OpAccessChain resultType*StorageBuffer resultId _0 const(int, 0) _1; OpLoad resultType resultId %addr;"
        IRBuilder builder(inst);
        auto addr = emitInst(parent, inst, SpvOpAccessChain, inst->getOperand(0)->getDataType(), kResultID, inst->getOperand(0), emitIntConstant(0, builder.getIntType()), inst->getOperand(1));
        return emitInst(parent, inst, SpvOpLoad, inst->getFullType(), kResultID, addr);
    }
    
    SpvInst* emitStructuredBufferStore(SpvInstParent* parent, IRInst* inst)
    {
        //"%addr = OpAccessChain resultType*StorageBuffer resultId _0 const(int, 0) _1; OpStore %addr _2;"
        IRBuilder builder(inst);
        auto addr = emitInst(parent, inst, SpvOpAccessChain, inst->getOperand(0)->getDataType(), kResultID, inst->getOperand(0), emitIntConstant(0, builder.getIntType()), inst->getOperand(1));
        return emitInst(parent, inst, SpvOpStore, addr, inst->getOperand(2));
    }

    SpvInst* emitStructuredBufferGetElementPtr(SpvInstParent* parent, IRInst* inst)
    {
        //"%addr = OpAccessChain resultType*StorageBuffer resultId _0 const(int, 0) _1;"
        IRBuilder builder(inst);
        auto addr = emitInst(parent, inst, SpvOpAccessChain, inst->getDataType(), kResultID, inst->getOperand(0), emitIntConstant(0, builder.getIntType()), inst->getOperand(1));
        return addr;
    }

    SpvInst* emitSwizzle(SpvInstParent* parent, IRSwizzle* inst)
    {
        if (inst->getElementCount() == 1)
        {
            const auto index = as<IRIntLit>(inst->getElementIndex(0))->getValue();
            return emitOpCompositeExtract(
                parent,
                inst,
                inst->getDataType(),
                inst->getBase(),
                makeArray(SpvLiteralInteger::from32(int32_t(index)))
            );
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

    IRType* dropVector(IRType* t)
    {
        if(const auto v = as<IRVectorType>(t))
            return v->getElementType();
        return t;
    };

    SpvInst* emitIntCast(SpvInstParent* parent, IRIntCast* inst)
    {
        const auto fromTypeV = inst->getOperand(0)->getDataType();
        const auto toTypeV = inst->getDataType();
        SLANG_ASSERT(!as<IRVectorType>(fromTypeV) == !as<IRVectorType>(toTypeV));
        const auto fromType = dropVector(fromTypeV);
        const auto toType = dropVector(toTypeV);
        SLANG_ASSERT(isIntegralType(fromType));
        SLANG_ASSERT(isIntegralType(toType));

        const auto fromInfo = getIntTypeInfo(fromType);
        const auto toInfo = getIntTypeInfo(toType);

        if(fromInfo == toInfo)
            return emitOpCopyObject(parent, inst, toTypeV, inst->getOperand(0));
        else if(fromInfo.width == toInfo.width)
            return emitOpBitcast(parent, inst, toTypeV, inst->getOperand(0));
        else if(!fromInfo.isSigned && !toInfo.isSigned)
            // unsigned to unsigned, don't sign extend
            return emitOpUConvert(parent, inst, toTypeV, inst->getOperand(0));
        else if(toInfo.isSigned)
            // unsigned to signed, sign extend
            return emitOpSConvert(parent, inst, toTypeV, inst->getOperand(0));
        else if(fromInfo.isSigned)
            // signed to unsigned, sign extend
            return emitOpSConvert(parent, inst, toTypeV, inst->getOperand(0));
        else if(fromInfo.isSigned && toInfo.isSigned)
            // signed to signed, sign extend
            return emitOpSConvert(parent, inst, toTypeV, inst->getOperand(0));

        SLANG_UNREACHABLE(__func__);
    }

    SpvInst* emitFloatCast(SpvInstParent* parent, IRFloatCast* inst)
    {
        const auto fromTypeV = inst->getOperand(0)->getDataType();
        const auto toTypeV = inst->getDataType();
        SLANG_ASSERT(!as<IRVectorType>(fromTypeV) == !as<IRVectorType>(toTypeV));
        const auto fromType = dropVector(fromTypeV);
        const auto toType = dropVector(toTypeV);
        SLANG_ASSERT(isFloatingType(fromType));
        SLANG_ASSERT(isFloatingType(toType));
        SLANG_ASSERT(!isTypeEqual(fromType, toType));

        return emitOpFConvert(parent, inst, toTypeV, inst->getOperand(0));
    }

    SpvInst* emitIntToFloatCast(SpvInstParent* parent, IRCastIntToFloat* inst)
    {
        const auto fromTypeV = inst->getOperand(0)->getDataType();
        const auto toTypeV = inst->getDataType();
        SLANG_ASSERT(!as<IRVectorType>(fromTypeV) == !as<IRVectorType>(toTypeV));
        const auto fromType = dropVector(fromTypeV);
        const auto toType = dropVector(toTypeV);
        SLANG_ASSERT(isIntegralType(fromType));
        SLANG_ASSERT(isFloatingType(toType));

        const auto fromInfo = getIntTypeInfo(fromType);

        return fromInfo.isSigned
            ? emitOpConvertSToF(parent, inst, toTypeV, inst->getOperand(0))
            : emitOpConvertUToF(parent, inst, toTypeV, inst->getOperand(0));
    }

    SpvInst* emitFloatToIntCast(SpvInstParent* parent, IRCastFloatToInt* inst)
    {
        const auto fromTypeV = inst->getOperand(0)->getDataType();
        const auto toTypeV = inst->getDataType();
        SLANG_ASSERT(!as<IRVectorType>(fromTypeV) == !as<IRVectorType>(toTypeV));
        const auto fromType = dropVector(fromTypeV);
        const auto toType = dropVector(toTypeV);
        SLANG_ASSERT(isFloatingType(fromType));
        SLANG_ASSERT(isIntegralType(toType));

        const auto toInfo = getIntTypeInfo(toType);

        return toInfo.isSigned
            ? emitOpConvertFToS(parent, inst, toTypeV, inst->getOperand(0))
            : emitOpConvertFToU(parent, inst, toTypeV, inst->getOperand(0));
    }

    SpvInst* emitCompositeConstruct(SpvInstParent* parent, IRInst* inst)
    {
        return emitOpCompositeConstruct(parent, inst, inst->getDataType(), OperandsOf(inst));
    }

    SpvInst* emitConstruct(SpvInstParent* parent, IRInst* inst)
    {
        if (as<IRBasicType>(inst->getDataType()))
        {
            if (inst->getOperandCount() == 1)
            {
                if (inst->getDataType() == inst->getOperand(0)->getDataType())
                    return emitOpCopyObject(
                        parent,
                        inst,
                        inst->getFullType(),
                        inst->getOperand(0));
                else
                    return emitOpBitcast(
                        parent,
                        inst,
                        inst->getFullType(),
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
            return emitCompositeConstruct(parent, inst);
        }
    }

    SpvInst* emitSplat(SpvInstParent* parent, IRInst* inst, IRInst* scalar, IRIntegerValue numElems)
    {
        const auto scalarTy = as<IRBasicType>(scalar->getDataType());
        SLANG_ASSERT(scalarTy);
        const auto spvVecTy = ensureVectorType(
            scalarTy->getBaseType(),
            numElems,
            nullptr);
        return emitOpCompositeConstruct(
            parent,
            inst,
            spvVecTy,
            List<IRInst*>::makeRepeated(scalar, Index(numElems))
        );
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
        IRType* elementType = dropVector(inst->getOperand(0)->getDataType());
        if (const auto matrixType = as<IRMatrixType>(inst->getDataType()))
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
        if(inst->getOperandCount() == 1)
        {
            return emitInst(parent, inst, opCode, inst->getDataType(), kResultID, OperandsOf(inst));
        }
        else if(inst->getOperandCount() == 2)
        {
            auto l = inst->getOperand(0);
            const auto lVec = as<IRVectorType>(l->getDataType());
            auto r = inst->getOperand(1);
            const auto rVec = as<IRVectorType>(r->getDataType());
            const auto go = [&](const auto l, const auto r){
                return emitInst(parent, inst, opCode, inst->getDataType(), kResultID, l, r);
            };
            if(lVec && !rVec)
            {
                const auto len = as<IRIntLit>(lVec->getElementCount());
                SLANG_ASSERT(len);
                return go(l, emitSplat(parent, nullptr, r, len->getValue()));
            }
            else if (!lVec && rVec)
            {
                const auto len = as<IRIntLit>(rVec->getElementCount());
                SLANG_ASSERT(len);
                return go(emitSplat(parent, nullptr, l, len->getValue()), r);
            }
            return go(l, r);
        }
        SLANG_UNREACHABLE("Arithmetic op with 0 or more than 2 operands");
    }

    OrderedHashSet<SpvCapability> m_capabilities;

    void requireSPIRVCapability(SpvCapability capability)
    {
        if (m_capabilities.add(capability))
        {
            emitOpCapability(
                getSection(SpvLogicalSectionID::Capabilities),
                nullptr,
                capability
            );
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

    SPIRVEmitContext(IRModule* module, TargetRequest* target, DiagnosticSink* sink)
        : SPIRVEmitSharedContext(module, target, sink)
        , m_irModule(module)
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
    legalizeIRForSPIRV(&context, irModule, irEntryPoints, codeGenContext);

#if 0
    DiagnosticSinkWriter writer(codeGenContext->getSink());
    dumpIR(
        irModule,
        {IRDumpOptions::Mode::Simplified, 0},
        "BEFORE SPIR-V EMIT",
        codeGenContext->getSourceManager(),
        &writer);
#endif

    context.emitFrontMatter();
    for (auto irEntryPoint : irEntryPoints)
    {
        context.ensureInst(irEntryPoint);
    }
    context.emitPhysicalLayout();

    spirvOut.addRange(
        (uint8_t const*) context.m_words.getBuffer(),
        context.m_words.getCount() * Index(sizeof(context.m_words[0])));

    const auto validationResult = debugValidateSPIRV(spirvOut);
    // If validation isn't available, don't say it failed, it's just a debug
    // feature so we can skip
    if(SLANG_FAILED(validationResult) && validationResult != SLANG_E_NOT_AVAILABLE)
    {
        codeGenContext->getSink()->diagnoseWithoutSourceView(
            SourceLoc{},
            Diagnostics::spirvValidationFailed
        );
        return validationResult;
    }

    return SLANG_OK;
}


} // namespace Slang
