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

#include "dms_free_install_callback_proxy.h"

#include "dtbschedmgr_log.h"
#include "ipc_types.h"
#include "message_parcel.h"

namespace OHOS {
namespace DistributedSchedule {
namespace {
const std::string TAG = "DmsFreeInstallCallbackProxy";
}
using OHOS::DistributedSchedule::IDmsFreeInstallCallback;
DmsFreeInstallCallbackProxy::DmsFreeInstallCallbackProxy(const sptr<IRemoteObject>& impl)
    : IRemoteProxy<IDmsFreeInstallCallback>(impl)
{
}

void DmsFreeInstallCallbackProxy::OnInstallFinished(
    const OHOS::AAFwk::Want& want, int32_t requestCode, int32_t resultCode)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(IDmsFreeInstallCallback::GetDescriptor())) {
        HILOGE("Write interface token failed.");
        return;
    }

    if (!data.WriteInt32(resultCode)) {
        HILOGE("Write resultCode error.");
        return;
    }

    if (!data.WriteParcelable(&want)) {
        HILOGE("want write failed.");
        return;
    }

    if (!data.WriteInt32(requestCode)) {
        HILOGE("requestCode write failed.");
        return;
    }

    int32_t error = Remote()->SendRequest(IDmsFreeInstallCallbackCmd::ON_FREE_INSTALL_DONE, data, reply, option);
    if (error != NO_ERROR) {
        HILOGE("OnFinished fail, error: %{public}d", error);
        return;
    }
}
}  // namespace DistributedSchedule
}  // namespace OHOS
