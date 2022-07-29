// cpu-helper-functions.cpp
#include "cpu-helper-functions.h"

#include "cpu-device.h"

namespace gfx
{
using namespace Slang;

Result SLANG_MCALL createCPUDevice(const IDevice::Desc* desc, IDevice** outDevice)
{
    RefPtr<cpu::DeviceImpl> result = new cpu::DeviceImpl();
    SLANG_RETURN_ON_FAIL(result->initialize(*desc));
    returnComPtr(outDevice, result);
    return SLANG_OK;
}

} // namespace gfx
