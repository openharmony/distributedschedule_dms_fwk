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

#include "distributed_ability_manager_proxy.h"

#include "dtbschedmgr_log.h"
#include "ipc_types.h"
#include "parcel_helper.h"
#include "string_ex.h"

namespace OHOS {
namespace DistributedSchedule {
namespace {
const std::string TAG = "DistributedAbilityManagerProxy";
const std::u16string DMS_PROXY_INTERFACE_TOKEN = u"ohos.distributedschedule.accessToken";
}

int32_t DistributedAbilityManagerProxy::Register(
    const std::shared_ptr<ContinuationExtraParams>& continuationExtraParams, int32_t& token)
{
    HILOGD("called.");
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOGE("remote is null");
        return ERR_NULL_OBJECT;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(DMS_PROXY_INTERFACE_TOKEN)) {
        return ERR_FLATTEN_OBJECT;
    }
    if (continuationExtraParams == nullptr) {
        PARCEL_WRITE_HELPER(data, Int32, VALUE_NULL);
    } else {
        PARCEL_WRITE_HELPER(data, Int32, VALUE_OBJECT);
        PARCEL_WRITE_HELPER(data, Parcelable, continuationExtraParams.get());
    }
    MessageParcel reply;
    MessageOption option;
    int32_t error = remote->SendRequest(REGISTER, data, reply, option);
    if (error != ERR_NONE) {
        HILOGE("SendRequest error = %{public}d", error);
        return error;
    }
    int32_t result = reply.ReadInt32();
    if (result != ERR_NONE) {
        HILOGE("result = %{public}d", result);
        return result;
    }
    PARCEL_READ_HELPER(reply, Int32, token);
    return ERR_NONE;
}

int32_t DistributedAbilityManagerProxy::Unregister(int32_t token)
{
    HILOGD("called.");
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOGE("remote is null");
        return ERR_NULL_OBJECT;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(DMS_PROXY_INTERFACE_TOKEN)) {
        return ERR_FLATTEN_OBJECT;
    }
    PARCEL_WRITE_HELPER(data, Int32, token);
    MessageParcel reply;
    PARCEL_TRANSACT_SYNC_RET_INT(remote, UNREGISTER, data, reply);
}

int32_t DistributedAbilityManagerProxy::RegisterDeviceSelectionCallback(
    int32_t token, const std::string& cbType, const sptr<IRemoteObject>& notifier)
{
    HILOGD("called.");
    if (cbType.empty()) {
        HILOGE("cbType is empty");
        return ERR_NULL_OBJECT;
    }
    if (notifier == nullptr) {
        HILOGE("notifier is nullptr");
        return ERR_NULL_OBJECT;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOGE("remote is null");
        return ERR_NULL_OBJECT;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(DMS_PROXY_INTERFACE_TOKEN)) {
        return ERR_FLATTEN_OBJECT;
    }
    PARCEL_WRITE_HELPER(data, Int32, token);
    PARCEL_WRITE_HELPER(data, String, cbType);
    PARCEL_WRITE_HELPER(data, RemoteObject, notifier);
    MessageParcel reply;
    PARCEL_TRANSACT_SYNC_RET_INT(remote, REGISTER_DEVICE_SELECTION_CALLBACK, data, reply);
}

int32_t DistributedAbilityManagerProxy::UnregisterDeviceSelectionCallback(int32_t token, const std::string& cbType)
{
    HILOGD("called.");
    if (cbType.empty()) {
        HILOGE("cbType is empty");
        return ERR_NULL_OBJECT;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOGE("remote is null");
        return ERR_NULL_OBJECT;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(DMS_PROXY_INTERFACE_TOKEN)) {
        return ERR_FLATTEN_OBJECT;
    }
    PARCEL_WRITE_HELPER(data, Int32, token);
    PARCEL_WRITE_HELPER(data, String, cbType);
    MessageParcel reply;
    PARCEL_TRANSACT_SYNC_RET_INT(remote, UNREGISTER_DEVICE_SELECTION_CALLBACK, data, reply);
}

int32_t DistributedAbilityManagerProxy::UpdateConnectStatus(int32_t token, const std::string& deviceId,
    const DeviceConnectStatus& deviceConnectStatus)
{
    HILOGD("called.");
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOGE("remote is null");
        return ERR_NULL_OBJECT;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(DMS_PROXY_INTERFACE_TOKEN)) {
        return ERR_FLATTEN_OBJECT;
    }
    PARCEL_WRITE_HELPER(data, Int32, token);
    PARCEL_WRITE_HELPER(data, String, deviceId);
    PARCEL_WRITE_HELPER(data, Int32, static_cast<int32_t>(deviceConnectStatus));
    MessageParcel reply;
    PARCEL_TRANSACT_SYNC_RET_INT(remote, UPDATE_CONNECT_STATUS, data, reply);
}

int32_t DistributedAbilityManagerProxy::StartDeviceManager(
    int32_t token, const std::shared_ptr<ContinuationExtraParams>& continuationExtraParams)
{
    HILOGD("called.");
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOGE("remote is null");
        return ERR_NULL_OBJECT;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(DMS_PROXY_INTERFACE_TOKEN)) {
        return ERR_FLATTEN_OBJECT;
    }
    PARCEL_WRITE_HELPER(data, Int32, token);
    if (continuationExtraParams == nullptr) {
        PARCEL_WRITE_HELPER(data, Int32, VALUE_NULL);
    } else {
        PARCEL_WRITE_HELPER(data, Int32, VALUE_OBJECT);
        PARCEL_WRITE_HELPER(data, Parcelable, continuationExtraParams.get());
    }
    MessageParcel reply;
    PARCEL_TRANSACT_SYNC_RET_INT(remote, START_DEVICE_MANAGER, data, reply);
}
} // namespace DistributedSchedule
} // namespace OHOS