// slang-emit-spirv.cpp
#include "slang-emit.h"

#include "slang-compiler.h"
#include "slang-ir.h"
#include "slang-ir-insts.h"

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
//
// [2.2: Terms]
//
// > Word: 32 bits.
//
// Despite the importance to SPIR-V, the `spirv.h` header doesn't
// define a type for words, so we'll do it here.

    /// A SPIR-V word.
typedef uint32_t SpvWord;

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

// Now that we've defined the intermediate data structures we will
// use to represent SPIR-V code during emission, we will move on
// to defining the main context type that will drive SPIR-V
// code generation.

    /// Context used for translating a Slang IR module to SPIR-V
struct SPIRVEmitContext
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
        m_words.add(SpvVersion);

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

        /// Register that `irInst` maps to `spvInst`
    void registerInst(IRInst* irInst, SpvInst* spvInst)
    {
        m_mapIRInstToSpvInst.Add(irInst, spvInst);
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
        // We first ensure that the `src` instruction has been emitted,
        // and then handle it as for any other <id> operand.
        //
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
        case IROP: return emitInst(getSection(SpvLogicalSectionID::Types), inst, SPVOP, kResultID)

        // > OpTypeVoid
        CASE(kIROp_VoidType, SpvOpTypeVoid);

        // > OpTypeBool
        CASE(kIROp_BoolType, SpvOpTypeBool);

#undef CASE

        // > OpTypeInt

#define CASE(IROP, BITS, SIGNED) \
        case IROP: return emitInst(getSection(SpvLogicalSectionID::Types), inst, SpvOpTypeInt, kResultID, BITS, SIGNED)

        CASE(kIROp_IntType,     32, 1);
        CASE(kIROp_UIntType,    32, 0);
        CASE(kIROp_Int64Type,   64, 1);
        CASE(kIROp_UInt64Type,  64, 0);

#undef CASE

        // > OpTypeFloat

#define CASE(IROP, BITS) \
        case IROP: return emitInst(getSection(SpvLogicalSectionID::Types), inst, SpvOpTypeFloat, kResultID, BITS)

        CASE(kIROp_HalfType,    16);
        CASE(kIROp_FloatType,   32);
        CASE(kIROp_DoubleType,  64);

#undef CASE

        // > OpTypeVector
        // > OpTypeMatrix
        // > OpTypeImage
        // > OpTypeSampler
        // > OpTypeArray
        // > OpTypeRuntimeArray
        // > OpTypeStruct
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

        // ...

        default:
            SLANG_UNIMPLEMENTED_X("unhandled instruction opcode");
            UNREACHABLE_RETURN(nullptr);
        }
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
            emitInst(spvFunc, irParam, SpvOpFunctionParameter,
                irParam->getFullType(),
                kResultID);
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
        }

        // Once all the basic blocks have had instructions allocated
        // for them, we go through and fill them in with their bodies.
        //
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
            //
            // TODO: We eventually need to emit `OpPhi` instructions corresponding
            // to the parameters of any non-entry block, with operands representing
            // the values passed along incoming edges from the predecessor blocks.

            for( auto irInst : irBlock->getOrdinaryInsts() )
            {
                // Any instructions local to the block will be emitted as children
                // of the block.
                //
                emitLocalInst(spvBlock, irInst);
            }
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

        // [3.32.17. Control-Flow Instructions]
        //
        // > OpReturn
        case kIROp_ReturnVoid: return emitInst(parent, inst, SpvOpReturn);
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
                emitInst(section, decoration, SpvOpEntryPoint, spvStage, dstID, name);
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

    SPIRVEmitContext(IRModule* module) :
        m_irModule(module),
        m_memoryArena(2048)
    {
    }
};

SlangResult emitSPIRVFromIR(
    BackEndCompileRequest*  compileRequest,
    IRModule*               irModule,
    const List<IRFunc*>&    irEntryPoints,
    List<uint8_t>&          spirvOut)
{
    SLANG_UNUSED(compileRequest);
    
    spirvOut.clear();

    SPIRVEmitContext context(irModule);

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
