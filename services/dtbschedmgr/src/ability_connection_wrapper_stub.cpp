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

#include "ability_connection_wrapper_stub.h"

#include "dtbschedmgr_log.h"
#include "ipc_object_proxy.h"
#include "ipc_types.h"
#include "parcel_helper.h"

namespace OHOS {
namespace DistributedSchedule {
using namespace AAFwk;

namespace {
const std::u16string CONNECTION_CALLBACK_INTERFACE_TOKEN = u"ohos.abilityshell.DistributedConnection";
}

AbilityConnectionWrapperStub::AbilityConnectionWrapperStub(sptr<IRemoteObject> connection)
{
    distributedConnection_ = connection;
}

int32_t AbilityConnectionWrapperStub::OnRemoteRequest(uint32_t code, MessageParcel& data,
    MessageParcel& reply, MessageOption& option)
{
    HILOGD("AbilityConnectionWrapperStub::OnRemoteRequest code = %{public}d", code);
    std::u16string descriptor = IAbilityConnection::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        HILOGE("AbilityConnectionWrapperStub local descriptor is not equal to remote");
        return ERR_INVALID_STATE;
    }

    sptr<AppExecFwk::ElementName> element(data.ReadParcelable<AppExecFwk::ElementName>());
    if (element == nullptr) {
        HILOGE("AbilityConnectionWrapperStub element is null");
        return ERR_INVALID_VALUE;
    }
    int32_t resultCode = ERR_NONE;
    switch (code) {
        case IAbilityConnection::ON_ABILITY_CONNECT_DONE: {
            if (auto remoteObject = data.ReadParcelable<IRemoteObject>()) {
                resultCode = data.ReadInt32();
                OnAbilityConnectDone(*element, remoteObject, resultCode);
                return ERR_NONE;
            }
            HILOGE("AbilityConnectionWrapperStub remoteObject is null");
            return ERR_INVALID_DATA;
        }
        case IAbilityConnection::ON_ABILITY_DISCONNECT_DONE: {
            resultCode = data.ReadInt32();
            OnAbilityDisconnectDone(*element, resultCode);
            return ERR_NONE;
        }
        default: {
            HILOGE("AbilityConnectionWrapperStub unknown code");
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
        }
    }
}

void AbilityConnectionWrapperStub::OnAbilityConnectDone(const AppExecFwk::ElementName& element,
    const sptr<IRemoteObject>& remoteObject, int32_t resultCode)
{
    HILOGD("AbilityConnectionWrapperStub::OnAbilityConnectDone called");
    MessageParcel data;
    if (!data.WriteInterfaceToken(CONNECTION_CALLBACK_INTERFACE_TOKEN)) {
        return;
    }
    MessageParcel reply;
    MessageOption option;
    PARCEL_WRITE_HELPER_NORET(data, Parcelable, &element);
    PARCEL_WRITE_HELPER_NORET(data, RemoteObject, remoteObject);
    PARCEL_WRITE_HELPER_NORET(data, Int32, resultCode);
    int32_t errCode = distributedConnection_->SendRequest(ON_ABILITY_CONNECT_DONE, data, reply, option);
    HILOGI("AbilityConnectionWrapperStub::OnAbilityConnectDone result %{public}d", errCode);
}

void AbilityConnectionWrapperStub::OnAbilityDisconnectDone(const AppExecFwk::ElementName& element,
    int32_t resultCode)
{
    HILOGD("AbilityConnectionWrapperStub::OnAbilityDisconnectDone called");
    MessageParcel data;
    if (!data.WriteInterfaceToken(CONNECTION_CALLBACK_INTERFACE_TOKEN)) {
        return;
    }

    MessageParcel reply;
    MessageOption option;
    PARCEL_WRITE_HELPER_NORET(data, Parcelable, &element);
    PARCEL_WRITE_HELPER_NORET(data, Int32, resultCode);
    int32_t errCode = distributedConnection_->SendRequest(ON_ABILITY_DISCONNECT_DONE, data, reply, option);
    HILOGI("AbilityConnectionWrapperStub::OnAbilityDisconnectDone result %{public}d", errCode);
}
} // namespace DistributedSchedule
} // namespace OHOS