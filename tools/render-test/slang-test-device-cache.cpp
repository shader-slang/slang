#include <algorithm>
#include "slang-test-device-cache.h"

// Static member definitions
std::mutex DeviceCache::s_mutex;
std::unordered_map<DeviceCache::DeviceCacheKey, DeviceCache::CachedDevice, DeviceCache::DeviceCacheKeyHash> DeviceCache::s_deviceCache;
std::chrono::steady_clock::time_point DeviceCache::s_lastCleanup = std::chrono::steady_clock::now();

bool DeviceCache::DeviceCacheKey::operator==(const DeviceCacheKey& other) const
{
    return deviceType == other.deviceType &&
           enableValidation == other.enableValidation &&
           enableRayTracingValidation == other.enableRayTracingValidation &&
           profileName == other.profileName &&
           requiredFeatures == other.requiredFeatures;
}

std::size_t DeviceCache::DeviceCacheKeyHash::operator()(const DeviceCacheKey& key) const
{
    std::size_t h1 = std::hash<int>{}(static_cast<int>(key.deviceType));
    std::size_t h2 = std::hash<bool>{}(key.enableValidation);
    std::size_t h3 = std::hash<bool>{}(key.enableRayTracingValidation);
    std::size_t h4 = std::hash<std::string>{}(key.profileName);
    
    std::size_t h5 = 0;
    for (const auto& feature : key.requiredFeatures)
    {
        h5 ^= std::hash<std::string>{}(feature) + 0x9e3779b9 + (h5 << 6) + (h5 >> 2);
    }
    
    return h1 ^ (h2 << 1) ^ (h3 << 2) ^ (h4 << 3) ^ (h5 << 4);
}

DeviceCache::CachedDevice::CachedDevice() 
    : refCount(0), lastUsed(std::chrono::steady_clock::now()) 
{
}

void DeviceCache::cleanupUnusedDevices()
{
    auto now = std::chrono::steady_clock::now();
    if (now - s_lastCleanup < CLEANUP_INTERVAL)
        return;
        
    s_lastCleanup = now;
    
    auto it = s_deviceCache.begin();
    while (it != s_deviceCache.end())
    {
        if (it->second.refCount == 0 && (now - it->second.lastUsed) > DEVICE_TIMEOUT)
        {
            it = s_deviceCache.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

Slang::ComPtr<rhi::IDevice> DeviceCache::acquireDevice(const rhi::DeviceDesc& desc)
{
    // Only cache Vulkan devices to avoid the Tegra driver issue
    if (desc.deviceType != rhi::DeviceType::Vulkan)
    {
        Slang::ComPtr<rhi::IDevice> device;
        auto result = rhi::getRHI()->createDevice(desc, device.writeRef());
        if (SLANG_SUCCEEDED(result))
            return device;
        return nullptr;
    }
    
    std::lock_guard<std::mutex> lock(s_mutex);
    
    // Create cache key
    DeviceCacheKey key;
    key.deviceType = desc.deviceType;
    key.enableValidation = desc.enableValidation;
    key.enableRayTracingValidation = desc.enableRayTracingValidation;
    key.profileName = desc.slang.targetProfile ? desc.slang.targetProfile : "";
    
    // Add required features to key
    for (int i = 0; i < desc.requiredFeatureCount; ++i)
    {
        key.requiredFeatures.push_back(desc.requiredFeatures[i]);
    }
    std::sort(key.requiredFeatures.begin(), key.requiredFeatures.end());
    
    // Clean up old devices periodically
    cleanupUnusedDevices();
    
    // Check if we have a cached device
    auto it = s_deviceCache.find(key);
    if (it != s_deviceCache.end())
    {
        it->second.refCount++;
        it->second.lastUsed = std::chrono::steady_clock::now();
        return it->second.device;
    }
    
    // Create new device
    Slang::ComPtr<rhi::IDevice> device;
    auto result = rhi::getRHI()->createDevice(desc, device.writeRef());
    if (SLANG_FAILED(result))
    {
        return nullptr;
    }
    
    // Cache the device
    CachedDevice& cached = s_deviceCache[key];
    cached.device = device;
    cached.refCount = 1;
    cached.lastUsed = std::chrono::steady_clock::now();
    
    return device;
}

void DeviceCache::releaseDevice(rhi::IDevice* device)
{
    if (!device)
        return;
        
    // Only manage Vulkan devices
    if (device->getDeviceType() != rhi::DeviceType::Vulkan)
        return;
        
    std::lock_guard<std::mutex> lock(s_mutex);
    
    // Find the device in cache and decrement ref count
    for (auto& pair : s_deviceCache)
    {
        if (pair.second.device.get() == device)
        {
            pair.second.refCount--;
            pair.second.lastUsed = std::chrono::steady_clock::now();
            break;
        }
    }
}

void DeviceCache::forceCleanup()
{
    std::lock_guard<std::mutex> lock(s_mutex);
    s_deviceCache.clear();
}
