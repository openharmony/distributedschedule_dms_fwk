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

#include "continuationManager/device_selection_notifier_proxy.h"

#include <string>

#include "continuation_result.h"
#include "dtbschedmgr_log.h"
#include "iremote_object.h"
#include "message_option.h"
#include "message_parcel.h"
#include "parcel_helper.h"

namespace OHOS {
namespace DistributedSchedule {
namespace {
const std::u16string DEVICE_SELECTION_NOTIFIER_INTERFACE_TOKEN = u"OHOS.DistributedSchedule.IDeviceSelectionNotifier";
const std::string TAG = "DeviceSelectionNotifierProxy";
}

void DeviceSelectionNotifierProxy::OnDeviceConnect(const std::vector<ContinuationResult>& continuationResults)
{
    HILOGD("called");
    MessageParcel data;
    if (!data.WriteInterfaceToken(DEVICE_SELECTION_NOTIFIER_INTERFACE_TOKEN)) {
        return;
    }
    MessageParcel reply;
    MessageOption option;
    if (!ContinuationResult::WriteContinuationResultsToParcel(data, continuationResults)) {
        return;
    }
    int32_t errCode = Remote()->SendRequest(IDeviceSelectionNotifier::EVENT_DEVICE_CONNECT, data, reply, option);
    HILOGD("result = %{public}d", errCode);
}

void DeviceSelectionNotifierProxy::OnDeviceDisconnect(const std::vector<std::string>& deviceIds)
{
    HILOGD("called");
    MessageParcel data;
    if (!data.WriteInterfaceToken(DEVICE_SELECTION_NOTIFIER_INTERFACE_TOKEN)) {
        return;
    }
    MessageParcel reply;
    MessageOption option;
    PARCEL_WRITE_HELPER_NORET(data, StringVector, deviceIds);
    int32_t errCode = Remote()->SendRequest(IDeviceSelectionNotifier::EVENT_DEVICE_DISCONNECT, data, reply, option);
    HILOGD("result = %{public}d", errCode);
}
} // namespace DistributedSchedule
} // namespace OHOS