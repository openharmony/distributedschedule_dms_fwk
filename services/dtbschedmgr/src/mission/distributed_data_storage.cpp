/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "mission/distributed_data_storage.h"

#include <unistd.h>
#include "datetime_ex.h"
#include "dtbschedmgr_device_info_storage.h"
#include "dtbschedmgr_log.h"
#include "mission/distributed_sched_mission_manager.h"

using namespace std;
using namespace OHOS::DistributedKv;

namespace OHOS {
namespace DistributedSchedule {
namespace {
const std::string TAG = "DistributedDataStorage";
}

bool DistributedDataStorage::Init()
{
    HILOGD("Init begin!");
    InitHandler();
    bool ret = InitKvDataManager();
    if (!ret) {
        HILOGE("Init InitKvDataManager failed!");
        return false;
    }
    SubscribeDistributedDataStorage();
    HILOGD("Init success!");
    return true;
}

bool DistributedDataStorage::InitKvDataManager()
{
    return true;
}

void DistributedDataStorage::SubscribeDistributedDataStorage()
{
}

void DistributedDataStorage::InitHandler()
{
    if (dmsDataStorageHandler_ == nullptr) {
        shared_ptr<AppExecFwk::EventRunner> runner = AppExecFwk::EventRunner::Create("dmsDataStorageHandler");
        dmsDataStorageHandler_ = make_shared<AppExecFwk::EventHandler>(runner);
    }
}

bool DistributedDataStorage::Stop()
{
    HILOGD("Stop begin!");
    bool ret = UninitKvDataManager();
    if (!ret) {
        HILOGW("Stop UninitKvDataManager failed!");
        return false;
    }
    dmsDataStorageHandler_ = nullptr;
    HILOGD("Stop success!");
    return true;
}

bool DistributedDataStorage::UninitKvDataManager()
{
    return true;
}

bool DistributedDataStorage::Insert(const string& deviceId, int32_t missionId,
    const uint8_t* byteStream, size_t len)
{
    return true;
}

bool DistributedDataStorage::InsertInnerLocked(const string& deviceId, int32_t missionId,
    const uint8_t* byteStream, size_t len)
{
    return true;
}

bool DistributedDataStorage::Delete(const string& deviceId, int32_t missionId)
{
    return true;
}

bool DistributedDataStorage::DeleteInnerLocked(const string& deviceId, int32_t missionId)
{
    return true;
}

bool DistributedDataStorage::FuzzyDelete(const string& uuid)
{
    if (uuid.empty()) {
        HILOGW("FuzzyDelete uuid empty!");
        return false;
    }
    {
        unique_lock<shared_mutex> writeLock(initLock_);
        bool ret = FuzzyDeleteInnerLocked(uuid);
        if (!ret) {
            HILOGW("FuzzyDelete deviceId = %{public}s failed!",
                DnetworkAdapter::AnonymizeDeviceId(uuid).c_str());
            return false;
        }
    }
    HILOGI("FuzzyDelete fuzzy delete successful!");
    return true;
}

bool DistributedDataStorage::FuzzyDeleteInnerLocked(const string& deviceId)
{
    return true;
}

bool DistributedDataStorage::Query(const string& deviceId, int32_t missionId, Value& value) const
{
    return true;
}

bool DistributedDataStorage::QueryInnerLocked(const string& deviceId, int32_t missionId, Value& value) const
{
    return true;
}

bool DistributedDataStorage::Sync(const std::string& deviceId)
{
    if (deviceId.empty()) {
        HILOGW("Sync deviceId empty!");
        return false;
    }
    return true;
}

bool DistributedDataStorage::Sync()
{
    return true;
}

bool DistributedDataStorage::SyncInnerLocked(const vector<string>& deviceVec, const SyncMode& mode, uint32_t syncCount)
{
    return true;
}

void DistributedDataStorage::RetrySync(const vector<string>& deviceVec, const DistributedKv::SyncMode& mode,
    uint32_t syncCount)
{
}

void DistributedDataStorage::GenerateKey(const string& deviceId, int32_t missionId, Key& key)
{
    string keyString;
    keyString.append(deviceId).append("_").append(to_string(missionId));
    key = keyString;
}

void DistributedDataStorage::GenerateKey(const string& deviceId, Key& key)
{
    string keyString;
    keyString.append(deviceId);
    key = keyString;
}

void DistributedDataStorage::GenerateValue(const uint8_t* byteStream, size_t len, Value& value)
{
    Value valueString((char *)byteStream, len);
    value = valueString;
}
} // DistributedSchedule
} // namespace OHOS
