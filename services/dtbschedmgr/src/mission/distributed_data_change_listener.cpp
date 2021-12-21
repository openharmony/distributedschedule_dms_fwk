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

#include "mission/distributed_data_change_listener.h"

#include <cinttypes>
#include "dtbschedmgr_device_info_storage.h"
#include "dtbschedmgr_log.h"
#include "mission/distributed_sched_mission_manager.h"
#include "string_ex.h"

using namespace std;
using namespace OHOS::DistributedKv;

namespace OHOS {
namespace DistributedSchedule {
namespace {
const std::string TAG = "DistributedDataChangeListener";

struct KeyInfo {
    std::string devId;
    int32_t missionId = 0;
    static constexpr int32_t MAX_SPLIT_VARS = 2;
    static std::unique_ptr<KeyInfo> ParseInfo(const std::string& strKey)
    {
        std::vector<string> strVector;
        SplitStr(strKey, "_", strVector);
        if (strVector.size() != MAX_SPLIT_VARS) {
            HILOGI("ParseInfo key invalid!");
            return nullptr;
        }
        int32_t index = 0;
        auto keyInfo = std::make_unique<KeyInfo>();
        keyInfo->devId = strVector[index++];
        int32_t iValue = 0;
        bool ret = StrToInt(strVector[index], iValue);
        if (!ret) {
            HILOGI("ParseInfo key missionId invalid!");
            return nullptr;
        }
        keyInfo->missionId = iValue;
        return keyInfo;
    }
};
}
void DistributedDataChangeListener::OnChange(const DistributedKv::ChangeNotification &appChangeNotification)
{
    HILOGI("OnChange");
    const vector<Entry> inserts = appChangeNotification.GetInsertEntries();
    for (const auto& entry : inserts) {
        unique_ptr<KeyInfo> keyInfo = KeyInfo::ParseInfo(entry.key.ToString());
        if (keyInfo != nullptr) {
            std::string keyStr = DnetworkAdapter::AnonymizeDeviceId(keyInfo->devId) + "_" +
                std::to_string(keyInfo->missionId);
            HILOGI("insertEntries Key:%{public}s, Value:%{public}s",
                keyStr.c_str(), entry.value.ToString().c_str());
            std::string networkId = DtbschedmgrDeviceInfoStorage::GetInstance().GetNetworkIdByUuid(keyInfo->devId);
            if (networkId.empty()) {
                HILOGE("OnChange networkId empty!");
                return;
            }
            DistributedSchedMissionManager::GetInstance().NotifySnapshotChanged(networkId, keyInfo->missionId);
        }
    }

    const vector<Entry> deletes = appChangeNotification.GetDeleteEntries();
    for (const auto& entry : deletes) {
        unique_ptr<KeyInfo> keyInfo = KeyInfo::ParseInfo(entry.key.ToString());
        if (keyInfo != nullptr) {
            std::string keyStr = DnetworkAdapter::AnonymizeDeviceId(keyInfo->devId) + "_" +
                std::to_string(keyInfo->missionId);
            HILOGI("deleteEntries Key:%{public}s, Value:%{public}s",
                keyStr.c_str(), entry.value.ToString().c_str());
            (void)DistributedSchedMissionManager::GetInstance().DequeueCachedSnapshotInfo(keyInfo->devId,
                keyInfo->missionId);
        }
    }

    const vector<Entry> updates = appChangeNotification.GetUpdateEntries();
    for (const auto& entry : updates) {
        unique_ptr<KeyInfo> keyInfo = KeyInfo::ParseInfo(entry.key.ToString());
        if (keyInfo != nullptr) {
            std::string keyStr = DnetworkAdapter::AnonymizeDeviceId(keyInfo->devId) + "_" +
                std::to_string(keyInfo->missionId);
            HILOGI("updateEntries Key:%{public}s, Value:%{public}s",
                keyStr.c_str(), entry.value.ToString().c_str());
            std::string networkId = DtbschedmgrDeviceInfoStorage::GetInstance().GetNetworkIdByUuid(keyInfo->devId);
            if (networkId.empty()) {
                HILOGE("OnChange networkId empty!");
                return;
            }
            DistributedSchedMissionManager::GetInstance().DequeueCachedSnapshotInfo(keyInfo->devId, keyInfo->missionId);
            DistributedSchedMissionManager::GetInstance().NotifySnapshotChanged(networkId, keyInfo->missionId);
        }
    }
}
} // namespace DistributedSchedule
} // namespace OHOS
