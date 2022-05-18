/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
const string TAG = "DistributedDataChangeListener";

struct KeyInfo {
    string uuid;
    int32_t missionId = 0;
    static constexpr int32_t MAX_SPLIT_VARS = 2;
    static unique_ptr<KeyInfo> ParseInfo(const std::string& strKey)
    {
        vector<string> strVector;
        SplitStr(strKey, "_", strVector);
        if (strVector.size() != MAX_SPLIT_VARS) {
            HILOGI("ParseInfo key invalid!");
            return nullptr;
        }
        int32_t index = 0;
        auto keyInfo = make_unique<KeyInfo>();
        keyInfo->uuid = strVector[index++];
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

void DistributedDataChangeListener::OnChange(const ChangeNotification &changeNotification)
{
    HILOGD("called.");
    const vector<Entry>& inserts = changeNotification.GetInsertEntries();
    for (const auto& entry : inserts) {
        unique_ptr<KeyInfo> keyInfo = KeyInfo::ParseInfo(entry.key.ToString());
        if (keyInfo != nullptr) {
            string keyStr = DnetworkAdapter::AnonymizeDeviceId(keyInfo->uuid) + "_" +
                to_string(keyInfo->missionId);
            HILOGI("insertEntries Key:%{public}s, Value:%{public}s", keyStr.c_str(), entry.value.ToString().c_str());
            string networkId = DtbschedmgrDeviceInfoStorage::GetInstance().GetNetworkIdByUuid(keyInfo->uuid);
            if (networkId.empty()) {
                HILOGI("networkId is empty!");
                return;
            }
            DistributedSchedMissionManager::GetInstance().NotifySnapshotChanged(networkId, keyInfo->missionId);
        }
    }

    const vector<Entry>& deletes = changeNotification.GetDeleteEntries();
    for (const auto& entry : deletes) {
        unique_ptr<KeyInfo> keyInfo = KeyInfo::ParseInfo(entry.key.ToString());
        if (keyInfo != nullptr) {
            string keyStr = DnetworkAdapter::AnonymizeDeviceId(keyInfo->uuid) + "_" +
                to_string(keyInfo->missionId);
            HILOGI("deleteEntries Key:%{public}s, Value:%{public}s", keyStr.c_str(), entry.value.ToString().c_str());
            (void)DistributedSchedMissionManager::GetInstance().DequeueCachedSnapshotInfo(keyInfo->uuid,
                keyInfo->missionId);
        }
    }

    const vector<Entry>& updates = changeNotification.GetUpdateEntries();
    for (const auto& entry : updates) {
        unique_ptr<KeyInfo> keyInfo = KeyInfo::ParseInfo(entry.key.ToString());
        if (keyInfo != nullptr) {
            string keyStr = DnetworkAdapter::AnonymizeDeviceId(keyInfo->uuid) + "_" +
                to_string(keyInfo->missionId);
            HILOGI("updateEntries Key:%{public}s, Value:%{public}s", keyStr.c_str(), entry.value.ToString().c_str());
            string networkId = DtbschedmgrDeviceInfoStorage::GetInstance().GetNetworkIdByUuid(keyInfo->uuid);
            if (networkId.empty()) {
                HILOGI("networkId is empty!");
                return;
            }
            DistributedSchedMissionManager::GetInstance().DequeueCachedSnapshotInfo(keyInfo->uuid, keyInfo->missionId);
            DistributedSchedMissionManager::GetInstance().NotifySnapshotChanged(networkId, keyInfo->missionId);
        }
    }
}
} // namespace DistributedSchedule
} // namespace OHOS