/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "bundle/bundle_manager_callback_stub.h"

#include "ability_manager_client.h"
#include "distributed_sched_service.h"
#include "dtbschedmgr_log.h"
#include "message_parcel.h"
#include "ipc_types.h"

namespace OHOS {
namespace DistributedSchedule {
namespace {
const std::string TAG = "DmsBundleManagerCallbackStub";
constexpr int64_t CONTINUATION_FREE_INSTALL_TIMEOUT = 50000; // 50s
}

DmsBundleManagerCallbackStub::DmsBundleManagerCallbackStub()
{
    memberFuncMap_[IDmsBundleManagerCallbackCmd::ON_QUERY_INSTALLATION_DONE] =
        &DmsBundleManagerCallbackStub::OnQueryInstallationFinishedInner;
}

int32_t DmsBundleManagerCallbackStub::OnQueryInstallationFinishedInner(MessageParcel& data, MessageParcel& reply)
{
    int32_t resultCode = data.ReadInt32();
    uint32_t versionCode = data.ReadUint32();
    std::string deviceId = data.ReadString();
    int32_t missionId = data.ReadInt32();

    int32_t result = OnQueryInstallationFinished(resultCode, deviceId, missionId, versionCode);
    return result;
}

int32_t DmsBundleManagerCallbackStub::OnQueryInstallationFinished(int32_t resultCode,
    std::string deviceId, int32_t missionId, int versionCode)
{
    HILOGI("bms callback received, missionId: %{public}d", missionId);
    int32_t result = 0;
    if (resultCode != ERR_OK) {
        HILOGE("remote not installed and cannot install on remote device");
        result = CONTINUE_REMOTE_UNINSTALLED_CANNOT_FREEINSTALL;
        DistributedSchedService::GetInstance().NotifyContinuationCallbackResult(missionId, result);
        return result;
    }

    DistributedSchedService::GetInstance().RemoveContinuationTimeout(missionId);
    HILOGI("able to install on target device, start continue ability with freeInstall");
    DistributedSchedService::GetInstance().SetContinuationTimeout(missionId, CONTINUATION_FREE_INSTALL_TIMEOUT);
    result = AAFwk::AbilityManagerClient::GetInstance()->ContinueAbility(deviceId, missionId, versionCode);
    return result;
}

int32_t DmsBundleManagerCallbackStub::OnRemoteRequest(
    uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    std::u16string descriptor = GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        HILOGE("Local descriptor is not equal to remote");
        return ERR_INVALID_STATE;
    }

    auto it = memberFuncMap_.find(code);
    if (it == memberFuncMap_.end()) {
        HILOGE("Not found");
        return ERR_INVALID_STATE;
    }

    return (this->*(it->second))(data, reply);
}
}  // namespace DistributedSchedule
}  // namespace OHOS
