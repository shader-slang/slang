#include "proxy-mutable-file-system.h"

#include "../../core/slang-blob.h"
#include "../../core/slang-crypto.h"
#include "../../core/slang-io.h"

namespace SlangRecord
{

using namespace Slang;

SlangResult MutableFileSystemProxy::loadFile(char const* path, ISlangBlob** outBlob)
{
    RECORD_CALL();
    RECORD_INPUT(path);

    auto& ctx = ReplayContext::get();

    if (ctx.isRecording())
    {
        // Forward to underlying file system
        ComPtr<ISlangBlob> blob;
        SlangResult result = getActual<ISlangMutableFileSystem>()->loadFile(path, blob.writeRef());

        if (SLANG_SUCCEEDED(result) && blob)
        {
            // Compute SHA1 hash of content
            SHA1::Digest digest =
                SHA1::compute(blob->getBufferPointer(), blob->getBufferSize());
            String hash = digest.toString();

            // Store file content by hash (de-duplicated)
            const char* replayPath = ctx.getCurrentReplayPath();
            if (replayPath)
            {
                String filesDir = Path::combine(String(replayPath), "files");
                Path::createDirectoryRecursive(filesDir);

                String contentPath = Path::combine(filesDir, hash);
                if (!File::exists(contentPath))
                {
                    File::writeAllBytes(
                        contentPath,
                        blob->getBufferPointer(),
                        blob->getBufferSize());
                }
            }

            // Record the hash (so playback can find the file)
            const char* hashCStr = hash.getBuffer();
            ctx.record(RecordFlag::Output, hashCStr);

            if (outBlob)
                *outBlob = blob.detach();
        }
        else
        {
            // Record empty hash for failed loads
            const char* emptyHash = "";
            ctx.record(RecordFlag::Output, emptyHash);
        }

        RECORD_RETURN(result);
    }
    else if (ctx.isPlayback())
    {
        // Read the hash from the stream
        const char* hashCStr = nullptr;
        ctx.record(RecordFlag::Output, hashCStr);

        if (!hashCStr || hashCStr[0] == '\0')
        {
            // File load failed during recording
            SlangResult result = SLANG_E_NOT_FOUND;
            RECORD_RETURN(result);
        }

        // Load content from captured file
        const char* replayPath = ctx.getCurrentReplayPath();
        if (!replayPath)
        {
            SlangResult result = SLANG_E_NOT_AVAILABLE;
            RECORD_RETURN(result);
        }

        String filesDir = Path::combine(String(replayPath), "files");
        String contentPath = Path::combine(filesDir, String(hashCStr));

        List<uint8_t> fileData;
        SlangResult readResult = File::readAllBytes(contentPath, fileData);
        if (SLANG_FAILED(readResult))
        {
            RECORD_RETURN(readResult);
        }

        // Create blob from content
        ComPtr<ISlangBlob> blob = RawBlob::create(fileData.getBuffer(), fileData.getCount());
        if (outBlob)
            *outBlob = blob.detach();

        SlangResult result = SLANG_OK;
        RECORD_RETURN(result);
    }
    else
    {
        // Idle mode - just forward
        SlangResult result = getActual<ISlangMutableFileSystem>()->loadFile(path, outBlob);
        RECORD_RETURN(result);
    }
}

} // namespace SlangRecord
