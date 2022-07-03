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

#include "app_connection_stub.h"

#include "dtbschedmgr_log.h"
#include "ipc_types.h"

namespace OHOS {
namespace DistributedSchedule {
using namespace AAFwk;
namespace {
const std::string TAG = "AppConnectionStub";
}

AppConnectionStub::AppConnectionStub(const sptr<DmsNotifier>& dmsNotifier, int32_t token,
    const std::shared_ptr<ContinuationExtraParams>& continuationExtraParams)
{
    dmsNotifier_ = dmsNotifier;
    token_ = token;
    continuationExtraParams_ = continuationExtraParams;
}

int32_t AppConnectionStub::OnRemoteRequest(uint32_t code, MessageParcel& data,
    MessageParcel& reply, MessageOption& option)
{
    HILOGD("code = %{public}u", code);
    std::u16string descriptor = IAbilityConnection::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        HILOGE("local descriptor is not equal to remote");
        return ERR_INVALID_STATE;
    }

    sptr<AppExecFwk::ElementName> element(data.ReadParcelable<AppExecFwk::ElementName>());
    if (element == nullptr) {
        HILOGE("element is nullptr");
        return ERR_INVALID_VALUE;
    }
    int32_t resultCode = ERR_NONE;
    switch (code) {
        case IAbilityConnection::ON_ABILITY_CONNECT_DONE: {
            if (auto remoteObject = data.ReadRemoteObject()) {
                resultCode = data.ReadInt32();
                OnAbilityConnectDone(*element, remoteObject, resultCode);
                return ERR_NONE;
            }
            HILOGE("remoteObject is nullptr");
            return ERR_INVALID_DATA;
        }
        case IAbilityConnection::ON_ABILITY_DISCONNECT_DONE: {
            resultCode = data.ReadInt32();
            OnAbilityDisconnectDone(*element, resultCode);
            return ERR_NONE;
        }
        default: {
            HILOGE("unknown request code, please check");
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
        }
    }
}

void AppConnectionStub::OnAbilityConnectDone(const AppExecFwk::ElementName& element,
    const sptr<IRemoteObject>& remoteObject, int32_t resultCode)
{
    HILOGD("called.");
    if (dmsNotifier_ == nullptr) {
        HILOGE("dmsNotifier_ is nullptr");
        return;
    }
    dmsNotifier_->ScheduleStartDeviceManager(remoteObject, token_, continuationExtraParams_);
    return;
}

void AppConnectionStub::OnAbilityDisconnectDone(const AppExecFwk::ElementName& element,
    int32_t resultCode)
{
    HILOGD("called.");
    if (dmsNotifier_ == nullptr) {
        HILOGE("dmsNotifier_ is nullptr");
        return;
    }
    dmsNotifier_->ScheduleStartDeviceManager(nullptr, token_, continuationExtraParams_);
    return;
}
} // namespace DistributedSchedule
} // namespace OHOS