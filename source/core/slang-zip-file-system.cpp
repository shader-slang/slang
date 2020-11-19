#include "slang-zip-file-system.h"

// These match what is in amalgamate.sh - so we can just
// use the github repro with the appropriate tag (and submodules)

#ifdef _MSC_VER
// Disable warning on VS
// warning C4334: '<<': result of 32-bit shift implicitly converted to 64 bits (was 64-bit shift intended?)
#   pragma warning(disable:4334)
// warning C4100: : unreferenced formal parameter
#   pragma warning(disable:4100)
// warning C4127: conditional expression is constant
#   pragma warning(disable:4127)
#endif

#include "../../external/miniz/miniz.h"
#include "../../external/miniz/miniz_common.h"
#include "../../external/miniz/miniz_tdef.h"
#include "../../external/miniz/miniz_tinfl.h"
#include "../../external/miniz/miniz_zip.h"

#include "../../external/miniz/miniz.c"
#include "../../external/miniz/miniz_tdef.c"
#include "../../external/miniz/miniz_tinfl.c"
#include "../../external/miniz/miniz_zip.c"

namespace Slang
{

const char input[] = "Hello world!";

/* static */void ZipCompressionUtil::unitTest()
{
    List<uint8_t> compressedInput;

    {
        const mz_ulong inputCount = mz_ulong(SLANG_COUNT_OF(input));

        const mz_ulong compressedInputBoundCount = mz_compressBound(inputCount);

        compressedInput.setCount(compressedInputBoundCount);

        mz_ulong compressedInputCount = 0;

        const int status = mz_compress(compressedInput.getBuffer(), &compressedInputCount, (const uint8_t*)input, inputCount);

        SLANG_ASSERT(status == MZ_OK);

        compressedInput.setCount(Index(compressedInputCount));
    }

    //SLANG_CHECK(_checkLines(UnownedStringSlice::fromLiteral(""), checkLines, SLANG_COUNT_OF(checkLines)));

#if 0
    List<char> output;
    {
        mz_deflateBound(
            const int status = mz_uncompress(pUncomp, &uncomp_len, compressedInput.getBuffer(), compressedInput.getCount());

        SLANG_CHECK(status == MZ_OK);
    }
#endif
}


} // namespace Slang
