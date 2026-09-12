#include "KeyValueStoreManagerImpl.cpp"

namespace chip {
namespace DeviceLayer {
namespace Silabs {
MigrationManager & MigrationManager::GetMigrationInstance() { static MigrationManager instance; return instance; }
void MigrationManager::applyMigrations() {}
}
}
}

using Config = chip::DeviceLayer::Internal::SilabsConfig;
using namespace chip::DeviceLayer::PersistedStorage;

static void Reset()
{
    Config::objects.clear();
    Config::failedRead = Config::failedWrite = Config::failedDelete = 0;
    chip::DeviceLayer::layer.pending = nullptr;
    assert(KeyValueStoreMgrImpl().Init() == CHIP_NO_ERROR);
}

static size_t Used()
{
    return std::count_if(std::begin(mKvsKeyMap), std::end(mKvsKeyMap), [](uint16_t hash) { return hash != 0; });
}

int main()
{
    auto & kvs = KeyValueStoreMgrImpl();
    const uint8_t value = 42;
    uint8_t output = 0;
    Reset();
    // Reproduce the old full map with just seven live records, then boot.
    for (unsigned i = 0; i < 255; ++i)
    {
        char key[16];
        snprintf(key, sizeof(key), "key/%03u", i);
        assert(kvs._Put(key, &value, 1) == CHIP_NO_ERROR);
    }
    assert(kvs.ForceKeyMapSave() == CHIP_NO_ERROR);
    for (unsigned i = 7; i < 255; ++i) Config::objects.erase(Config::kConfigKey_KvsFirstKeySlot + i);
    assert(kvs.Init() == CHIP_NO_ERROR);
    assert(Used() == 7);
    assert(kvs.Init() == CHIP_NO_ERROR && Used() == 7);
    for (unsigned i = 0; i < 7; ++i)
    {
        char key[16];
        snprintf(key, sizeof(key), "key/%03u", i);
        assert(kvs._Get(key, &output, 1) == CHIP_NO_ERROR && output == value);
    }
    puts("PASS: recover 248 stale slots and preserve seven live records");

    Reset();
    for (unsigned cycle = 0; cycle < 200; ++cycle)
    {
        for (const char * key : { "g/sum", "f/1/g", "g/fs/n", "other/1", "other/2" })
            assert(kvs._Put(key, &value, 1) == CHIP_NO_ERROR);
        assert(kvs.ForceKeyMapSave() == CHIP_NO_ERROR);
        for (const char * key : { "g/sum", "f/1/g", "g/fs/n", "other/1", "other/2" })
            assert(kvs._Delete(key) == CHIP_NO_ERROR);
        // Reset immediately, without firing the delayed save callback.
        assert(kvs.Init() == CHIP_NO_ERROR && Used() == 0);
    }
    puts("PASS: 200 delete/immediate-reboot cycles without slot growth");

    Reset();
    assert(kvs._Put("abc", &value, 1) == CHIP_NO_ERROR);
    assert(kvs.ForceKeyMapSave() == CHIP_NO_ERROR);
    Config::failedRead = Config::kConfigKey_KvsFirstKeySlot;
    const auto saved = Config::objects[Config::kConfigKey_KvsStringKeyMap];
    assert(kvs.Init() == CHIP_ERROR_PERSISTED_STORAGE_FAILED);
    assert(Config::objects[Config::kConfigKey_KvsStringKeyMap] == saved);
    assert(kvs._Get("abc", &output, 1) == CHIP_ERROR_PERSISTED_STORAGE_FAILED);
    assert(kvs._Put("xyz", &value, 1) == CHIP_ERROR_PERSISTED_STORAGE_FAILED);
    Config::failedRead = 0;
    Config::failedDelete = Config::kConfigKey_KvsFirstKeySlot;
    assert(kvs._Delete("abc") == CHIP_ERROR_PERSISTED_STORAGE_FAILED && Used() == 1);
    Config::failedDelete = 0;
    Config::failedWrite = Config::kConfigKey_KvsStringKeyMap;
    assert(kvs._Delete("abc") == CHIP_ERROR_PERSISTED_STORAGE_FAILED);
    assert(kvs.ForceKeyMapSave() == CHIP_ERROR_PERSISTED_STORAGE_FAILED);
    assert(kvs.Init() == CHIP_ERROR_PERSISTED_STORAGE_FAILED);
    Config::failedWrite = 0;
    assert(kvs.Init() == CHIP_NO_ERROR && Used() == 0);
    puts("PASS: propagate read/delete/save failures without discarding valid records");

    Reset();
    Config::objects[Config::kConfigKey_KvsStringKeyMap] = { 1, 0 };
    assert(kvs.Init() == CHIP_ERROR_INCORRECT_STATE);
    puts("PASS: reject truncated index map");
}
