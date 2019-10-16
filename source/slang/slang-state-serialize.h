// slang-state-serialize.h
#ifndef SLANG_STATE_SERIALIZE_H_INCLUDED
#define SLANG_STATE_SERIALIZE_H_INCLUDED

#include "../core/slang-basic.h"
#include "../core/slang-stream.h"
#include "../core/slang-string.h"

// For TranslationUnitRequest
#include "slang-compiler.h"

namespace Slang {

enum
{
    kNullOffset = 0x80000000
};

class RelativeContainer;

template <typename T>
class Safe32Ptr
{
public:
    typedef Safe32Ptr ThisType;

    T& operator*() const { return *get(); }
    T* operator->() const { return get(); }
    operator T*() const { return get(); }

    Safe32Ptr(const ThisType& rhs): m_offset(rhs.m_offset), m_container(rhs.m_container) {}
    const Safe32Ptr& operator=(const ThisType& rhs) { m_offset = rhs.m_offset; return *this; }

    SLANG_FORCE_INLINE T* get() const;

    Safe32Ptr() : m_container(nullptr), m_offset(kNullOffset) {}

    RelativeContainer* m_container;
    int32_t m_offset;
};

template <typename T>
class Relative32Ptr
{
public:
    typedef Relative32Ptr ThisType;

    T& operator*() const { return *get(); }
    T* operator->() const { return get(); }
    operator T*() const { return get(); }

    T* get()
    {
        uint8_t* nonConstThis = (uint8_t*)this;
        return (m_offset == kNullOffset) ? nullptr : (T*)(nonConstThis + m_offset); 
    }
    T* get() const
    {
        uint8_t* nonConstThis = const_cast<uint8_t*>((const uint8_t*)this); 
        return (m_offset == kNullOffset) ? nullptr : (T*)(nonConstThis + m_offset);
    }

    T* detach() { T* ptr = get(); m_offset = kNullOffset; }

    void setNull() { m_offset = kNullOffset; }

    SLANG_FORCE_INLINE void set(T* ptr) { m_offset = ptr ? int32_t(((uint8_t*)ptr) - ((const uint8_t*)this)) : uint32_t(kNullOffset); }

    Relative32Ptr(const Safe32Ptr<T>& rhs) { set(rhs.get()); }
    Relative32Ptr() :m_offset(kNullOffset) {}
    Relative32Ptr(T* ptr) { set(ptr); }

    const Relative32Ptr& operator=(const ThisType& rhs) { m_offset = rhs.m_offset; }
    const Relative32Ptr& operator=(const Safe32Ptr& rhs) { set(rhs.get()); }

    int32_t m_offset;           
};

template <typename T>
class Relative32Array
{
    Relative32Array():m_count(0) {}
    uint32_t m_count;               ///< the size of the data
    Relative32Ptr m_data;           ///< The data
};

class RelativeContainer
{
public:

    template <typename T>
    Safe32Ptr<T> allocate()
    {
        void* data = allocate(sizeof(T), SLANG_ALIGN_OF(T));
        memset(data, 0, size);
        return Safe32Ptr<T>(getOffset(data), this);
    }

    void* allocate(size_t size, size_t alignment);
    void* allocateAndZero(size_t size, size_t alignment);

    template <typename T>
    T* toPtr(Safe32Ptr<T> ptr)
    {
        if (ptr.m_offset == kNullOffset)
        {
            return nullptr;
        }
        SLANG_ASSERT(ptr.m_offset >= 0 && ptr.m_offset <= m_current)
        return (T*)(m_data.getBuffer() + ptr.m_offset);
    }

    template <typename T>
    Safe32Ptr<T> toSafe(T* ptr) { SafePtr<T> safePtr; relPtr.m_offset = getOffset(); }
    int32_t getOffset(const void* ptr)
    {
        ptrdiff_t offset = ((const uint8_t*)ptr) - m_data.getBuffer();
        if (offset < 0 || size_t(offset) > m_current)
        {
            return int32_t(kNullOffset);
        }
        return int32_t(offset);
    }

        /// Get the contained data
    uint8_t* getData() { return m_data.getBuffer(); }

        /// Ctor
    RelativeContainer();

protected:
    size_t m_current;
    List<uint8_t> m_data; 
};


// --------------------------------------------------------------------------

template <typename T>
SLANG_FORCE_INLINE T* Safe32Ptr<T>::get() const
{
    return m_container ? ((T*)(m_container->getData() + m_offset)) : (T*)nullptr;
}

// Work out the state based on the api
class CompileState
{
    class File
    {
        enum Type
        {
            FileSystem,             ///< Loaded from the file system
            String,           ///< Specified as a String
        };

        Type type;
        Slang::String path;
        Slang::String contents;
    };

    // Ignore
    // spSessionSetSharedLibraryLoader
    // spAddBuiltins
    // spSetFileSystem
    // spSetDiagnosticCallback
    // spSetWriter

    struct SessionState
    {
        
        //List<File> builtIns;


    };

    // spSetCodeGenTarget/spAddCodeGenTarget
    // spSetTargetProfile
    // spSetTargetFlags
    // spSetTargetFloatingPointMode
    // spSetTargetMatrixLayoutMode
    struct TargetState
    {
        Slang::Profile profile;
        Slang::CodeGenTarget target;
        SlangTargetFlags targetFlags;
        Slang::FloatingPointMode floatingPointMode;

        SlangMatrixLayoutMode defaultMatrixLayoutMode;
    };

    struct Define
    {
        String key;
        String value;
    };

    // spAddTranslationUnit
    struct TranslationUnitState
    {
        SourceLanguage language;

        String moduleName;

        // spTranslationUnit_addPreprocessorDefine
        List<Define> preprocessorDefinitions;
    };

    struct RequestState
    {
        // spSetCompileFlags
        SlangCompileFlags compileFlags;
        // spSetDumpIntermediates
        bool shouldDumpIntermediates;
        // spSetLineDirectiveMode
        Slang::LineDirectiveMode lineDirectiveMode;
        
        List<TargetState> targets;

        // spSetDebugInfoLevel
        Slang::DebugInfoLevel debugInfoLevel;
        // spSetOptimizationLevel
        Slang::OptimizationLevel optimizationLevel;
        // spSetOutputContainerFormat
        Slang::ContainerFormat containerFormat;
        // spSetPassThrough
        Slang::PassThroughMode passThroughMode;

        // spAddSearchPath
        List<String> searchPaths;

        // spAddPreprocessorDefine
        List<Define> preprocessorDefinitions;

        List<TranslationUnitState> translationUnits;
    };

    SessionState sessionState;
    RequestState requestState;

    SlangResult loadState(Session* session);
    SlangResult loadState(EndToEndCompileRequest* request);
};

} // namespace Slang

#endif
