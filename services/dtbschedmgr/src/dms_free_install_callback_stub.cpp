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

#include "dms_free_install_callback_stub.h"

#include "dtbschedmgr_log.h"
#include "ipc_types.h"
#include "message_parcel.h"

namespace OHOS {
namespace DistributedSchedule {
namespace {
const std::string TAG = "DmsFreeInstallCallbackStub";
}
DmsFreeInstallCallbackStub::DmsFreeInstallCallbackStub()
{
    memberFuncMap_[IDmsFreeInstallCallbackCmd::ON_FREE_INSTALL_DONE] =
        &DmsFreeInstallCallbackStub::OnInstallFinishedInner;
}

int32_t DmsFreeInstallCallbackStub::OnInstallFinishedInner(MessageParcel& data, MessageParcel& reply)
{
    auto resultCode = data.ReadInt32();
    std::unique_ptr<AAFwk::Want> want(data.ReadParcelable<AAFwk::Want>());
    if (want == nullptr) {
        HILOGE("want is nullptr");
        return ERR_INVALID_VALUE;
    }
    int32_t requestCode = data.ReadInt32();
    OnInstallFinished(*want, requestCode, resultCode);
    return NO_ERROR;
}

int32_t DmsFreeInstallCallbackStub::OnRemoteRequest(
    uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    std::u16string descriptor = DmsFreeInstallCallbackStub::GetDescriptor();
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
