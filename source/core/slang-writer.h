#ifndef SLANG_WRITER_H
#define SLANG_WRITER_H

#include "slang-string.h"

#include "../../slang-com-helper.h"

namespace Slang
{


class WriterHelper
{
public:
    SlangResult print(const char* format, ...);
    SlangResult put(const char* text);

    SLANG_FORCE_INLINE void flush() { m_writer->flush(); }
    
    ISlangWriter* getWriter() const { return m_writer;  }

    WriterHelper(ISlangWriter* writer) :m_writer(writer) {}

protected:
    ISlangWriter* m_writer;
};

struct WriterFlag
{
    enum Enum :uint32_t
    {
        RefCounted = 0x1,
        IsConsole = 0x2,
        IsOwned = 0x4,
    };
private:
    WriterFlag() = delete;
};
typedef uint32_t WriterFlags;

class BaseWriter : public ISlangWriter, public RefObject
{
public:
    // ISlangUnknown
    SLANG_REF_OBJECT_IUNKNOWN_QUERY_INTERFACE
    SLANG_REF_OBJECT_IUNKNOWN_ADD_REF
    SLANG_NO_THROW uint32_t SLANG_MCALL release() { return (m_flags & WriterFlag::RefCounted) ? (uint32_t)releaseReference() : 1; }

    // ISlangWriter - default impl
    virtual SlangResult writeVaList(const char* format, va_list args) { SLANG_UNUSED(args); SLANG_UNUSED(format); return SLANG_E_NOT_IMPLEMENTED; }
    virtual void flush() SLANG_OVERRIDE {}
    virtual SlangBool isConsole() SLANG_OVERRIDE { return SlangBool((m_flags & WriterFlag::IsConsole) != 0); }
    virtual SlangResult setMode(SlangWriterMode mode) SLANG_OVERRIDE { SLANG_UNUSED(mode);  return SLANG_FAIL; }

    BaseWriter(WriterFlags flags) :
        m_flags(flags)
    {
    }

protected:
    ISlangUnknown * getInterface(const Guid& guid);
    WriterFlags m_flags;
};

class CallbackWriter : BaseWriter
{
public:
    typedef BaseWriter Parent;   
    // ISlangWriter
    virtual SlangResult write(const char* chars, size_t numChars) SLANG_OVERRIDE;
    
    CallbackWriter(SlangDiagnosticCallback callback, const void* data, WriterFlags flags) :
        Parent(flags),
        m_callback(callback),
        m_data(data)
    {}

protected:
 
    SlangDiagnosticCallback m_callback;
    const void* m_data;
};

class FileWriter : public BaseWriter
{
public:
    typedef BaseWriter Parent;
    // ISlangWriter
    virtual SlangResult writeVaList(const char* format, va_list args) SLANG_OVERRIDE;
    virtual SlangResult write(const char* chars, size_t numChars) SLANG_OVERRIDE;
    virtual void flush() SLANG_OVERRIDE;
    virtual SlangResult setMode(SlangWriterMode mode) SLANG_OVERRIDE;

    static const bool isConsole(FILE* file);
    static WriterFlags getIsConsoleFlag(FILE* file) { return isConsole(file) ? WriterFlag::IsConsole : 0; }

        /// Ctor
    FileWriter(FILE* file, WriterFlags flags) :
        Parent(flags),
        m_file(file)
    {}

        /// Dtor
    ~FileWriter();

protected:
    FILE* m_file;
};

class StringWriter : public BaseWriter
{
public:
    typedef BaseWriter Parent;
    // ISlangWriter
    virtual SlangResult writeVaList(const char* format, va_list args) SLANG_OVERRIDE;
    virtual SlangResult write(const char* chars, size_t numChars) SLANG_OVERRIDE;
    
        /// Ctor
    StringWriter(StringBuilder* builder, WriterFlags flags) :
        Parent(flags),
        m_builder(builder)
    {}

protected:
    StringBuilder* m_builder;
};

class NullWriter : public BaseWriter
{
public:
    typedef BaseWriter Parent;
    // ISlangWriter
    virtual SlangResult writeVaList(const char* format, va_list args) SLANG_OVERRIDE { SLANG_UNUSED(format); SLANG_UNUSED(args); return SLANG_OK; }
    virtual SlangResult write(const char* chars, size_t numChars) SLANG_OVERRIDE { SLANG_UNUSED(chars); SLANG_UNUSED(numChars); return SLANG_OK; }

    /// Ctor
    NullWriter(WriterFlags flags) :
        Parent(flags)
    {}
};

}

#endif // SLANG_TEXT_WRITER_H
