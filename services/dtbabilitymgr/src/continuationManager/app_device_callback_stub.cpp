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

#include "continuationManager/app_device_callback_stub.h"

#include "dtbschedmgr_log.h"
#include "ipc_types.h"
#include "parcel_helper.h"

namespace OHOS {
namespace DistributedSchedule {
namespace {
const std::string TAG = "AppDeviceCallbackStub";
}

AppDeviceCallbackStub::AppDeviceCallbackStub(const sptr<DmsNotifier>& dmsNotifier)
{
    dmsNotifier_ = dmsNotifier;
}

int32_t AppDeviceCallbackStub::OnRemoteRequest(uint32_t code, MessageParcel& data,
    MessageParcel& reply, MessageOption& option)
{
    HILOGD("code = %{public}u", code);
    std::u16string descriptor = IAppDeviceCallback::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        HILOGE("descriptor check failed");
        return ERR_INVALID_STATE;
    }
    int32_t token = -1;
    switch (code) {
        case IAppDeviceCallback::EVENT_DEVICE_CONNECT: {
            PARCEL_READ_HELPER(data, Int32, token);
            std::vector<ContinuationResult> continuationResults;
            if (!ContinuationResult::ReadContinuationResultsFromParcel(data, continuationResults)) {
                return ERR_FLATTEN_OBJECT;
            }
            int32_t result = OnDeviceConnect(token, continuationResults);
            return result;
        }
        case IAppDeviceCallback::EVENT_DEVICE_DISCONNECT: {
            PARCEL_READ_HELPER(data, Int32, token);
            std::vector<std::u16string> deviceIds;
            PARCEL_READ_HELPER(data, String16Vector, &deviceIds); // use u16string, because from app.
            int32_t result = OnDeviceDisconnect(token, ContinationManagerUtils::Str16VecToStr8Vec(deviceIds));
            return result;
        }
        case IAppDeviceCallback::EVENT_DEVICE_CANCEL: {
            int32_t result = OnDeviceCancel();
            return result;
        }
        default: {
            HILOGE("unknown request code, please check");
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
        }
    }
}

int32_t AppDeviceCallbackStub::OnDeviceConnect(int32_t token,
    const std::vector<ContinuationResult>& continuationResults)
{
    HILOGD("called.");
    if (dmsNotifier_ == nullptr) {
        HILOGE("dmsNotifier_ is nullptr");
        return ERR_NULL_OBJECT;
    }
    int32_t result = dmsNotifier_->OnDeviceConnect(token, continuationResults);
    return result;
}

int32_t AppDeviceCallbackStub::OnDeviceDisconnect(int32_t token, const std::vector<std::string>& deviceIds)
{
    HILOGD("called.");
    if (dmsNotifier_ == nullptr) {
        HILOGE("dmsNotifier_ is nullptr");
        return ERR_NULL_OBJECT;
    }
    int32_t result = dmsNotifier_->OnDeviceDisconnect(token, deviceIds);
    return result;
}

int32_t AppDeviceCallbackStub::OnDeviceCancel()
{
    HILOGD("called.");
    if (dmsNotifier_ == nullptr) {
        HILOGE("dmsNotifier_ is nullptr");
        return ERR_NULL_OBJECT;
    }
    int32_t result = dmsNotifier_->OnDeviceCancel();
    return result;
}
} // namespace DistributedSchedule
} // namespace OHOS