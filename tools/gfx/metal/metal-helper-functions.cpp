// metal-helper-functions.cpp
#include "metal-helper-functions.h"
#include "metal-device.h"

namespace gfx
{

using namespace Slang;

Result SLANG_MCALL createMetalDevice(const IDevice::Desc* desc, IDevice** outRenderer)
{
    RefPtr<metal::DeviceImpl> result = new metal::DeviceImpl();
    SLANG_RETURN_ON_FAIL(result->initialize(*desc));
    returnComPtr(outRenderer, result);
    return SLANG_OK;
}

} // namespace gfx
