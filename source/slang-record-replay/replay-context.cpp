#include "replay-context.h"

#include "../core/slang-blob.h"
#include "../core/slang-crypto.h"
#include "../core/slang-io.h"
#include "../core/slang-platform.h"
#include "../slang/slang-ast-type.h"
#include "../slang/slang-compiler-api.h"
#include "../slang/slang-syntax.h"
#include "proxy/proxy-component-type.h"

#include <chrono>
#include <cinttypes>
#include <cstdio>

#ifdef _WIN32
#include <windows.h>
#endif

namespace SlangRecord
{

using Slang::File;
using Slang::Path;

// =============================================================================
// Environment variable check
// =============================================================================

static bool isRecordLogRequested()
{
    Slang::StringBuilder envValue;
    if (SLANG_SUCCEEDED(Slang::PlatformUtil::getEnvironmentVariable(
            Slang::UnownedStringSlice("SLANG_RECORD_LOG"),
            envValue)))
    {
        return envValue == "1";
    }
    return false;
}

bool isRecordLayerRequested()
{
    Slang::StringBuilder envValue;
    if (SLANG_SUCCEEDED(Slang::PlatformUtil::getEnvironmentVariable(
            Slang::UnownedStringSlice("SLANG_RECORD_LAYER"),
            envValue)))
    {
        return (envValue == "1") ? 1 : 0;
    }
    else
    {
        return false;
    }
}

// =============================================================================
// TypeId helpers
// =============================================================================

const char* getTypeIdName(TypeId id)
{
    switch (id)
    {
    case TypeId::Int8:
        return "Int8";
    case TypeId::Int16:
        return "Int16";
    case TypeId::Int32:
        return "Int32";
    case TypeId::Int64:
        return "Int64";
    case TypeId::UInt8:
        return "UInt8";
    case TypeId::UInt16:
        return "UInt16";
    case TypeId::UInt32:
        return "UInt32";
    case TypeId::UInt64:
        return "UInt64";
    case TypeId::Float32:
        return "Float32";
    case TypeId::Float64:
        return "Float64";
    case TypeId::Bool:
        return "Bool";
    case TypeId::String:
        return "String";
    case TypeId::Blob:
        return "Blob";
    case TypeId::Array:
        return "Array";
    case TypeId::ObjectHandle:
        return "ObjectHandle";
    case TypeId::Null:
        return "Null";
    case TypeId::TypeReflectionRef:
        return "TypeReflectionRef";
    case TypeId::ProgramLayoutRef:
        return "ProgramLayoutRef";
    default:
        return "Unknown";
    }
}

// =============================================================================
// ReplayContext implementation
// =============================================================================

bool ReplayContext::isRecordingSuppressed() const
{
    return SlangRecord::isRecordingSuppressed();
}

TypeMismatchException::TypeMismatchException(TypeId expected, TypeId actual)
    : Slang::Exception(
          Slang::String("Type mismatch: expected ") + getTypeIdName(expected) + ", got " +
          getTypeIdName(actual))
    , m_expected(expected)
    , m_actual(actual)
{
}

DataMismatchException::DataMismatchException(size_t offset, size_t size)
    : Slang::Exception(
          Slang::String("Data mismatch at offset ") + Slang::String((uint64_t)offset) + " (size " +
          Slang::String((uint64_t)size) + " bytes)")
    , m_offset(offset)
    , m_size(size)
{
}

// =============================================================================
// ReplayContext construction and low-level helpers
// =============================================================================

ReplayContext& ReplayContext::get()
{
    static ReplayContext s_instance;
    return s_instance;
}

ReplayContext::ReplayContext()
    : m_stream()
    , m_referenceStream()
    , m_arena(4096)
    , m_mode(Mode::Idle)
    , m_ttyLogging(isRecordLogRequested())
{
    // Don't call setMode() here - CharEncoding may not be initialized yet.
    // The deferred setup will happen on first use via ensureInitialized().
}

ReplayContext::ReplayContext(const void* data, size_t size)
    : m_stream(data, size)
    , m_referenceStream()
    , m_arena(4096)
    , m_mode(Mode::Playback)
    , m_ttyLogging(isRecordLogRequested())
{
}

ReplayContext::ReplayContext(const void* referenceData, size_t referenceSize, bool syncMode)
    : m_stream()
    , m_referenceStream(referenceData, referenceSize)
    , m_arena(4096)
    , m_mode(Mode::Idle)
    , m_ttyLogging(isRecordLogRequested())
{
    SLANG_UNUSED(syncMode);
    // Set mode through setMode() to trigger mirror file setup if recording
    setMode(syncMode ? Mode::Sync : Mode::Record);
}

ReplayContext::~ReplayContext()
{
    // Destructor must be defined in DLL to properly free Dictionary memory.
    // The compiler will generate calls to ~Dictionary() for each member,
    // and this ensures they run in the DLL's allocator context.
}

void ReplayContext::ensureInitialized()
{
    // Guard against re-entry and multiple initialization
    if (m_initialized)
        return;
    m_initialized = true;

    // Now it's safe to use file system operations (CharEncoding is initialized)
    if (m_mode == Mode::Idle && isRecordLayerRequested())
    {
        setMode(Mode::Record);
    }
}

void ReplayContext::reset()
{
    closeRecordingMirror(); // Close any active mirror file
    m_stream.reset();
    m_indexStream.reset();
    m_referenceStream.reset();
    m_arena.reset();
    m_mode = Mode::Idle;
    m_objectToHandle.clear();
    m_handleToObject.clear();
    m_nextHandle = kFirstValidHandle;
    m_proxyToImpl.clear();
    m_implToProxy.clear();
    m_currentThisHandle = kNullHandle;
    // Note: m_handlers is intentionally NOT cleared - they're typically registered once
}

void ReplayContext::switchToPlayback()
{
    // Clear all local state
    m_referenceStream.reset();
    m_arena.reset();
    m_objectToHandle.clear();
    m_handleToObject.clear();
    m_nextHandle = kFirstValidHandle;
    m_proxyToImpl.clear();
    m_implToProxy.clear();
    m_currentThisHandle = kNullHandle;

    // Switch stream to reading mode and reset position to 0
    m_stream.setReading(true);
    m_stream.seek(0);
    // Index stream stays as-is for navigation purposes
    m_indexStream.setReading(true);
    m_indexStream.seek(0);
    m_mode = Mode::Playback;
}

void ReplayContext::switchToSync()
{
    // Copy recorded data to reference stream for comparison
    m_referenceStream = ReplayStream(m_stream.getData(), m_stream.getSize());

    // Clear local state
    m_arena.reset();
    m_objectToHandle.clear();
    m_handleToObject.clear();
    m_nextHandle = kFirstValidHandle;
    m_proxyToImpl.clear();
    m_implToProxy.clear();
    m_currentThisHandle = kNullHandle;

    // Reset main stream for new recording that will be verified against reference
    m_stream.reset();
    // Also reset index stream for new recording
    m_indexStream.reset();
    m_mode = Mode::Sync;
}

// =============================================================================
// Mode Management
// =============================================================================

void ReplayContext::setMode(Mode mode)
{
    if (mode == m_mode)
        return;

    // Handle transitions to/from Record mode
    if (mode == Mode::Record && m_mode != Mode::Record)
    {
        setupRecordingMirror();
    }
    else if (mode != Mode::Record && m_mode == Mode::Record)
    {
        closeRecordingMirror();
    }

    m_mode = mode;
}

void ReplayContext::enable()
{
    if (m_mode == Mode::Idle)
        setMode(Mode::Record);
}

void ReplayContext::disable()
{
    setMode(Mode::Idle);
}

// =============================================================================
// Replay Directory Management
// =============================================================================

void ReplayContext::setReplayDirectory(const char* path)
{
    m_replayDirectory = path ? path : ".slang-replays";
}

const char* ReplayContext::getReplayDirectory() const
{
    return m_replayDirectory.getBuffer();
}

const char* ReplayContext::getCurrentReplayPath() const
{
    if (m_currentReplayPath.getLength() == 0)
        return nullptr;
    return m_currentReplayPath.getBuffer();
}

String ReplayContext::generateTimestampFolderName()
{
    // Get current time with milliseconds
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    std::tm tm_now;
#ifdef _WIN32
    localtime_s(&tm_now, &time_t_now);
#else
    localtime_r(&time_t_now, &tm_now);
#endif

    // Format: YYYY-MM-DD_HH-MM-SS-mmm
    char buffer[64];
    snprintf(
        buffer,
        sizeof(buffer),
        "%04d-%02d-%02d_%02d-%02d-%02d-%03d",
        tm_now.tm_year + 1900,
        tm_now.tm_mon + 1,
        tm_now.tm_mday,
        tm_now.tm_hour,
        tm_now.tm_min,
        tm_now.tm_sec,
        static_cast<int>(ms.count()));

    return String(buffer);
}

void ReplayContext::setupRecordingMirror()
{
    // Check for SLANG_RECORD_PATH environment variable for explicit path
    Slang::StringBuilder envPath;
    if (SLANG_SUCCEEDED(Slang::PlatformUtil::getEnvironmentVariable(
            Slang::UnownedStringSlice("SLANG_RECORD_PATH"),
            envPath)) &&
        envPath.getLength() > 0)
    {
        // Use the explicit path directly
        m_currentReplayPath = envPath.toString();
    }
    else
    {
        // Generate timestamped folder path
        String timestamp = generateTimestampFolderName();
        m_currentReplayPath = Path::combine(m_replayDirectory, timestamp);
    }

    // Create the directory structure
    if (!Path::createDirectoryRecursive(m_currentReplayPath))
    {
        // If we can't create the directory, just record without mirroring
        m_currentReplayPath = String();
        return;
    }

    // Set up mirror file for main stream
    String streamPath = Path::combine(m_currentReplayPath, "stream.bin");
    try
    {
        m_stream.setMirrorFile(streamPath.getBuffer());
    }
    catch (const Slang::Exception&)
    {
        // If we can't create the mirror file, just record without mirroring
        m_currentReplayPath = String();
        return;
    }

    // Set up mirror file for index stream
    String indexPath = Path::combine(m_currentReplayPath, "index.bin");
    try
    {
        m_indexStream.setMirrorFile(indexPath.getBuffer());
    }
    catch (const Slang::Exception&)
    {
        // Index is optional - continue without it but close main mirror to be consistent
        m_stream.closeMirrorFile();
        m_currentReplayPath = String();
    }
}

void ReplayContext::closeRecordingMirror()
{
    m_stream.closeMirrorFile();
    m_indexStream.closeMirrorFile();
    m_currentReplayPath = String();
}

void ReplayContext::writeIndexEntry()
{
    CallIndexEntry entry;
    entry.streamPosition = m_stream.getPosition();
    m_indexStream.write(&entry, sizeof(entry));
}

// =============================================================================
// Call Index Access
// =============================================================================

size_t ReplayContext::getCallCount() const
{
    if (m_indexStream.getSize() == 0)
        return 0;
    return m_indexStream.getSize() / sizeof(CallIndexEntry);
}

const CallIndexEntry* ReplayContext::getCallIndexEntry(size_t callIndex) const
{
    size_t count = getCallCount();
    if (callIndex >= count)
        return nullptr;

    // The index stream data is a flat array of CallIndexEntry structs
    const uint8_t* data = m_indexStream.getData();
    return reinterpret_cast<const CallIndexEntry*>(data + callIndex * sizeof(CallIndexEntry));
}

SlangResult ReplayContext::seekToCall(size_t callIndex)
{
    const CallIndexEntry* entry = getCallIndexEntry(callIndex);
    if (!entry)
        return SLANG_E_INVALID_ARG;

    m_stream.seek(entry->streamPosition);
    return SLANG_OK;
}

// Helper class for collecting directory entries
class DirectoryCollector : public Path::Visitor
{
public:
    List<String> directories;

    void accept(Path::Type type, const Slang::UnownedStringSlice& filename) override
    {
        if (type == Path::Type::Directory)
        {
            directories.add(String(filename));
        }
    }
};

String ReplayContext::findLatestReplayFolder(const char* baseDir)
{
    DirectoryCollector collector;
    SlangResult result = Path::find(String(baseDir), nullptr, &collector);

    if (SLANG_FAILED(result) || collector.directories.getCount() == 0)
        return String();

    // Sort alphabetically - timestamps will sort chronologically
    collector.directories.sort();

    // Return the last one (most recent)
    return collector.directories.getLast();
}

SlangResult ReplayContext::loadReplay(const char* folderPath)
{
    if (!folderPath)
        return SLANG_E_INVALID_ARG;

    String streamPath = Path::combine(String(folderPath), "stream.bin");

    if (!File::exists(streamPath))
        return SLANG_E_NOT_FOUND;

    try
    {
        m_stream = ReplayStream::loadFromFile(streamPath.getBuffer());

        // Also try to load the index stream (optional - may not exist for older recordings)
        String indexPath = Path::combine(String(folderPath), "index.bin");
        if (File::exists(indexPath))
        {
            try
            {
                m_indexStream = ReplayStream::loadFromFile(indexPath.getBuffer());
            }
            catch (const Slang::Exception&)
            {
                // Index is optional, continue without it
                m_indexStream = ReplayStream();
            }
        }
        else
        {
            // No index file, clear any existing index
            m_indexStream = ReplayStream();
        }
        m_currentReplayPath = folderPath;

        m_mode = Mode::Playback;
        return SLANG_OK;
    }
    catch (const Slang::Exception&)
    {
        return SLANG_FAIL;
    }
}

SlangResult ReplayContext::loadLatestReplay()
{
    String latestFolder = findLatestReplayFolder(m_replayDirectory.getBuffer());

    if (latestFolder.getLength() == 0)
        return SLANG_E_NOT_FOUND;

    String fullPath = Path::combine(m_replayDirectory, latestFolder);
    return loadReplay(fullPath.getBuffer());
}

// =============================================================================
// TTY Logging
// =============================================================================

void ReplayContext::setTtyLogging(bool enable)
{
    m_ttyLogging = enable;
}

void ReplayContext::logCall(const char* signature, void* thisPtr)
{
    char buffer[512];
    if (thisPtr)
        snprintf(
            buffer,
            sizeof(buffer),
            "[REPLAY] %s [this=%p, handle=%" PRIu64 "]\n",
            signature,
            thisPtr,
            m_currentThisHandle);
    else
        snprintf(buffer, sizeof(buffer), "[REPLAY] %s [static]\n", signature);

#ifdef _WIN32
    // Use OutputDebugString on Windows since GUI apps don't have stderr
    OutputDebugStringA(buffer);
#endif
    // Also try stderr in case it's connected
    fputs(buffer, stderr);
    fflush(stderr);
}

void ReplayContext::recordError(const char* message)
{
    if (!isActive() || m_mode != Mode::Record)
        return;

    // Write an error marker to the stream
    writeTypeId(TypeId::Error);

    size_t len = message ? strlen(message) : 0;
    uint32_t len32 = static_cast<uint32_t>(len > 4095 ? 4095 : len);
    m_stream.write(&len32, sizeof(len32));
    if (len32 > 0)
        m_stream.write(message, len32);

    // Also log to TTY if enabled
    if (m_ttyLogging)
    {
        char buffer[4200];
        snprintf(buffer, sizeof(buffer), "[REPLAY ERROR] %s\n", message ? message : "(null)");
#ifdef _WIN32
        OutputDebugStringA(buffer);
#endif
        fputs(buffer, stderr);
        fflush(stderr);
    }
}

// =============================================================================
// Signature Parsing
// =============================================================================

const char* ReplayContext::parseSignature(const char* signature, char* buffer, size_t bufferSize)
{
    // Parse __FUNCSIG__ (MSVC) or __PRETTY_FUNCTION__ (GCC/Clang) to extract
    // "ClassName::methodName" format.
    //
    // MSVC __FUNCSIG__ examples:
    //   "SlangResult __cdecl SlangRecord::GlobalSessionProxy::createSession(...)"
    //   "void __cdecl SlangRecord::SessionProxy::addSearchPath(...)"
    //
    // GCC/Clang __PRETTY_FUNCTION__ examples:
    //   "SlangResult SlangRecord::GlobalSessionProxy::createSession(...)"
    //   "void SlangRecord::SessionProxy::addSearchPath(...)"
    //
    // We want to extract: "GlobalSessionProxy::createSession"

    if (!signature || !buffer || bufferSize == 0)
        return signature;

    const char* start = signature;
    const char* end = signature + strlen(signature);

    // Find the opening parenthesis (marks end of function name)
    const char* parenPos = strchr(signature, '(');
    if (parenPos)
        end = parenPos;

    // Walk backwards from end to find the function name
    // Skip any template arguments by counting angle brackets
    const char* funcEnd = end;
    while (funcEnd > start && (funcEnd[-1] == ' ' || funcEnd[-1] == '\t'))
        funcEnd--;

    // Find the start of "ClassName::methodName" by looking for SlangRecord::
    // or the second-to-last "::" before the function name
    const char* namespaceMarker = strstr(signature, "SlangRecord::");
    const char* classStart = nullptr;

    if (namespaceMarker && namespaceMarker < funcEnd)
    {
        // Skip past "SlangRecord::"
        classStart = namespaceMarker + strlen("SlangRecord::");
    }
    else
    {
        // No SlangRecord:: namespace, look for the class name differently
        // Find the last space before the function name (after return type/calling convention)
        const char* lastSpace = nullptr;
        for (const char* p = start; p < funcEnd; p++)
        {
            if (*p == ' ')
                lastSpace = p;
        }
        if (lastSpace)
            classStart = lastSpace + 1;
        else
            classStart = start;
    }

    // Copy to buffer
    size_t len = funcEnd - classStart;
    if (len >= bufferSize)
        len = bufferSize - 1;

    memcpy(buffer, classStart, len);
    buffer[len] = '\0';

    return buffer;
}

uint64_t ReplayContext::testOnlyRegisterProxyImpl(ISlangUnknown* obj)
{
    if (obj == nullptr)
        return kNullHandle;

    // Check if already registered
    uint64_t* existingHandle = m_objectToHandle.tryGetValue(obj);
    if (existingHandle)
        return *existingHandle;

    // Assign new handle
    uint64_t handle = m_nextHandle++;
    m_objectToHandle[obj] = handle;
    m_handleToObject[handle] = obj;
    return handle;
}

uint64_t ReplayContext::registerProxyImpl(ISlangUnknown* proxy, ISlangUnknown* implementation)
{
    if (proxy == nullptr || implementation == nullptr)
        return kNullHandle;

    // Check if already registered
    uint64_t* existingHandle = m_objectToHandle.tryGetValue(proxy);
    if (existingHandle)
        return *existingHandle;

    // Assign new handle
    uint64_t handle = m_nextHandle++;
    m_objectToHandle[proxy] = handle;
    m_handleToObject[handle] = proxy;
    m_proxyToImpl[proxy] = implementation;
    m_implToProxy[implementation] = proxy;
    return handle;
}

void ReplayContext::unregisterProxyImpl(ISlangUnknown* proxy)
{
    if (proxy == nullptr)
        return;

    ISlangUnknown** impl = m_proxyToImpl.tryGetValue(proxy);
    if (impl)
    {
        m_implToProxy.remove(*impl);
    }
    m_proxyToImpl.remove(proxy);

    uint64_t* handle = m_objectToHandle.tryGetValue(proxy);
    if (handle)
    {
        m_handleToObject.remove(*handle);
        m_objectToHandle.remove(proxy);
    }
}

ISlangUnknown* ReplayContext::getProxyImpl(ISlangUnknown* implementation)
{
    if (implementation == nullptr)
        return nullptr;

    ISlangUnknown** proxy = m_implToProxy.tryGetValue(implementation);
    if (!proxy)
        return nullptr;

    return *proxy;
}

// get implementatoin
ISlangUnknown* ReplayContext::getImplementationImpl(ISlangUnknown* proxy)
{
    if (proxy == nullptr)
        return nullptr;

    ISlangUnknown** impl = m_proxyToImpl.tryGetValue(proxy);
    if (!impl)
        return nullptr;

    return *impl;
}

bool ReplayContext::isInterfaceRegisteredImpl(ISlangUnknown* obj) const
{
    if (obj == nullptr)
        return true; // null is always "registered" as kNullHandle
    return m_proxyToImpl.containsKey(obj) || m_implToProxy.containsKey(obj);
}

uint64_t ReplayContext::getProxyHandleImpl(ISlangUnknown* obj) const
{
    if (obj == nullptr)
        return kNullHandle;

    const uint64_t* handle = m_objectToHandle.tryGetValue(obj);
    if (!handle)
        throw UntrackedInterfaceException(obj);

    return *handle;
}

ISlangUnknown* ReplayContext::getProxy(uint64_t handle) const
{
    if (handle == kNullHandle)
        return nullptr;

    ISlangUnknown* const* obj = m_handleToObject.tryGetValue(handle);
    if (!obj)
        throw HandleNotFoundException(handle);

    return *obj;
}

// =============================================================================
// Playback Dispatcher
// =============================================================================

void ReplayContext::registerHandler(const char* signature, PlaybackHandler handler)
{
    m_handlers[String(signature)] = handler;
}

bool ReplayContext::executeNextCall()
{
    if (m_mode != Mode::Playback)
        return false;

    if (m_stream.atEnd())
        return false;

    // Read the stream position so we can peak at the signature + type id
    // before handing it off to the handler.
    uint64_t streamPos = m_stream.getPosition();

    // Read the function signature
    const char* signature = nullptr;
    record(RecordFlag::Input, signature);

    if (signature == nullptr)
        return false;

    // Look up the handler
    PlaybackHandler* handler = m_handlers.tryGetValue(String(signature));
    if (!handler)
    {
        throw Slang::Exception(String("No handler registered for function: ") + signature);
    }

    // Read the 'this' pointer handle (recorded by beginCall)
    uint64_t thisHandle = kNullHandle;
    TypeId typeId = readTypeId();
    if (typeId == TypeId::ObjectHandle)
    {
        m_stream.read(&thisHandle, sizeof(thisHandle));
    }
    else
    {
        throw TypeMismatchException(TypeId::ObjectHandle, typeId);
    }

    // Store the current 'this' handle for the handler to use
    m_currentThisHandle = thisHandle;

    // Seek back to the start of the command before calling the handler.
    m_stream.seek(streamPos);

    // Save the signature for the post-call callback (the handler will
    // consume the stream, invalidating the pointer from the arena).
    String savedSignature(signature);

    // Call the handler - it will read the remaining arguments from the stream
    (*handler)(*this);

    // Invoke the post-call callback if one is registered
    if (m_postCallCallback)
    {
        m_postCallCallback(savedSignature.getBuffer(), thisHandle, m_postCallUserData);
    }

    return true;
}

void ReplayContext::executeAll()
{
    const size_t totalSize = m_stream.getSize();
    auto startTime = std::chrono::steady_clock::now();
    auto lastPrintTime = startTime;
    
    while (executeNextCall())
    {
        auto now = std::chrono::steady_clock::now();
        auto timeSinceLastPrint = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastPrintTime).count();   
        if (timeSinceLastPrint >= 1000)
        {
            size_t currentPos = m_stream.getPosition();
            double percentComplete = totalSize > 0 ? (double)currentPos / (double)totalSize * 100.0 : 0.0;       
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime).count();
            double eta = 0.0; 
            if (currentPos > 0 && elapsed > 0)
            {
                double bytesPerSecond = 1000.0 *(double)currentPos / (double)elapsed;
                size_t remainingBytes = totalSize - currentPos;
                eta = (double)remainingBytes / bytesPerSecond;
            }
            lastPrintTime = now;
            
            printf("Progress: %.1f%% (%zu / %zu bytes)", percentComplete, currentPos, totalSize);
            if (eta > 0)
            {
                int etaMinutes = (int)(eta / 60);
                int etaSeconds = (int)eta % 60;
                printf(" - ETA: %dm %ds", etaMinutes, etaSeconds);
            }
            printf("\n");
        }
    }
    
    // Print final completion message
    auto endTime = std::chrono::steady_clock::now();
    auto totalElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
    printf("Replay complete: %zu bytes in %f seconds\n", totalSize, totalElapsed / 1000.0);
}

} // namespace SlangRecord
