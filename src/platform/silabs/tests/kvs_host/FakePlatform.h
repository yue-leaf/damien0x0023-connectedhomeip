#pragma once

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <map>
#include <vector>

struct CHIP_ERROR
{
    int value;
    constexpr CHIP_ERROR(int v = 0) : value(v) {}
    int Format() const { return value; }
    bool operator==(CHIP_ERROR other) const { return value == other.value; }
    bool operator!=(CHIP_ERROR other) const { return value != other.value; }
};
constexpr CHIP_ERROR CHIP_NO_ERROR(0), CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND(1), CHIP_ERROR_BUFFER_TOO_SMALL(2),
    CHIP_ERROR_INCORRECT_STATE(3), CHIP_ERROR_PERSISTED_STORAGE_FAILED(4), CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND(5),
    CHIP_ERROR_INVALID_ARGUMENT(6);
#define CHIP_ERROR_FORMAT "d"
#define KVS_MAX_ENTRIES 255
#define SL_KVS_SAVE_DELAY_SECONDS 2
#define SuccessOrExit(err) do { if ((err) != CHIP_NO_ERROR) goto exit; } while (0)
#define VerifyOrExit(condition, action) do { if (!(condition)) { action; goto exit; } } while (0)
#define VerifyOrReturnError(condition, err) do { if (!(condition)) return err; } while (0)
#define VerifyOrDie(condition) assert(condition)
#define ChipLogError(...) ((void) 0)
#define ChipLogProgress(...) ((void) 0)

namespace chip {
struct PersistentStorageDelegate { static constexpr unsigned kKeyLengthMax = 32; };
namespace Platform {
inline void * MemoryAlloc(size_t size) { return malloc(size); }
inline void * MemoryCalloc(size_t count, size_t size) { return calloc(count, size); }
inline void MemoryFree(void * ptr) { free(ptr); }
}
namespace Crypto {
constexpr unsigned kSHA256_Hash_Length = 32;
// Deliberately collide all keys to exercise prefix matching and failed reads.
inline void Hash_SHA256(const uint8_t *, size_t, uint8_t * out) { memset(out, 1, kSHA256_Hash_Length); }
}
namespace System {
namespace Clock {
using Timeout = std::chrono::milliseconds;
using Seconds32 = std::chrono::seconds;
}
struct Layer {
    using Callback = void (*)(Layer *, void *);
    Callback pending = nullptr;
    CHIP_ERROR StartTimer(Clock::Timeout, Callback callback, void *) { pending = callback; return CHIP_NO_ERROR; }
};
}
namespace DeviceLayer {
inline System::Layer layer;
inline System::Layer & SystemLayer() { return layer; }
namespace Internal {
struct SilabsConfig {
    static constexpr uint32_t kConfigKey_KvsStringKeyMap = 0x87500;
    static constexpr uint32_t kConfigKey_KvsFirstKeySlot = 0x87501;
    static constexpr uint32_t kConfigKey_KvsLastKeySlot = 0x875ff;
    static constexpr uint32_t kMinConfigKey_MatterKvs = kConfigKey_KvsStringKeyMap;
    inline static std::map<uint32_t, std::vector<uint8_t>> objects;
    inline static uint32_t failedRead = 0, failedWrite = 0, failedDelete = 0;
    static CHIP_ERROR Init() { return CHIP_NO_ERROR; }
    static CHIP_ERROR ReadConfigValueBin(uint32_t key, uint8_t * buf, size_t size, size_t & count, size_t offset)
    {
        count = 0;
        if (key == failedRead) return CHIP_ERROR_PERSISTED_STORAGE_FAILED;
        auto it = objects.find(key);
        if (it == objects.end()) return CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
        if (offset > it->second.size()) return CHIP_ERROR_INVALID_ARGUMENT;
        count = std::min(size, it->second.size() - offset);
        if (count) memcpy(buf, it->second.data() + offset, count);
        return size < it->second.size() - offset ? CHIP_ERROR_BUFFER_TOO_SMALL : CHIP_NO_ERROR;
    }
    static CHIP_ERROR ReadConfigValueBin(uint32_t key, uint8_t * buf, size_t size, size_t & count)
    {
        // The non-offset production overload does not perform partial reads.
        if (objects.count(key) && size < objects[key].size()) { count = 0; return CHIP_ERROR_BUFFER_TOO_SMALL; }
        return ReadConfigValueBin(key, buf, size, count, 0);
    }
    static CHIP_ERROR WriteConfigValueBin(uint32_t key, const uint8_t * value, size_t size)
    {
        if (key == failedWrite) return CHIP_ERROR_PERSISTED_STORAGE_FAILED;
        objects[key] = std::vector<uint8_t>(value, value + size);
        return CHIP_NO_ERROR;
    }
    static CHIP_ERROR ClearConfigValue(uint32_t key)
    {
        if (key == failedDelete) return CHIP_ERROR_PERSISTED_STORAGE_FAILED;
        return objects.erase(key) ? CHIP_NO_ERROR : CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }
    static bool ConfigValueExists(uint32_t key, size_t & size)
    {
        auto it = objects.find(key);
        if (it == objects.end()) return false;
        size = it->second.size();
        return true;
    }
};
}
namespace PersistedStorage { class KeyValueStoreManager {}; }
}
}
