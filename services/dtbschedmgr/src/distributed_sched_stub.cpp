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

#include "distributed_sched_stub.h"
#include "dtbschedmgr_log.h"
#include "ipc_skeleton.h"
#include "message_parcel.h"
#include "parcel_helper.h"

using namespace std;

namespace OHOS {
namespace DistributedSchedule {
using namespace AAFwk;
using namespace AppExecFwk;
namespace {
    const std::u16string DMS_STUB_INTERFACE_TOKEN = u"ohos.distributedschedule.accessToken";
}
DistributedSchedStub::DistributedSchedStub()
{
    localMemberFuncMap_[START_REMOTE_ABILITY] = &DistributedSchedStub::StartRemoteAbilityInner;
    memberFuncMap_[START_ABILITY_FROM_REMOTE] = &DistributedSchedStub::StartAbilityFromRemoteInner;
}

DistributedSchedStub::~DistributedSchedStub()
{
    memberFuncMap_.clear();
    localMemberFuncMap_.clear();
}

int32_t DistributedSchedStub::OnRemoteRequest(uint32_t code,
    MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    bool IsLocalCalling = IPCSkeleton::IsLocalCalling();
    HILOGI("DistributedSchedStub::OnReceived, code = %{public}d, flags = %{public}d IsLocalCalling = %{public}d.", code,
        option.GetFlags(), IsLocalCalling);
    auto localFuncIt = localMemberFuncMap_.find(code);
    if (localFuncIt != localMemberFuncMap_.end()) {
        auto memberFunc = localFuncIt->second;
        if (memberFunc != nullptr) {
            return (this->*memberFunc)(data, reply);
        }
    }
    auto distributedFuncIt = memberFuncMap_.find(code);
    if (distributedFuncIt != memberFuncMap_.end()) {
        auto memberFunc = distributedFuncIt->second;
        if (memberFunc != nullptr) {
            return (this->*memberFunc)(data, reply);
        }
    }
    HILOGW("DistributedSchedStub: default case, need check.");
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

int32_t DistributedSchedStub::StartRemoteAbilityInner(MessageParcel& data, MessageParcel& reply)
{
    if (!EnforceInterfaceToken(data)) {
        HILOGW("DistributedSchedStub:: StartRemoteAbilityInner interface token check failed!");
        return DMS_PERMISSION_DENIED;
    }
    shared_ptr<AAFwk::Want> userWant(data.ReadParcelable<AAFwk::Want>());
    if (userWant == nullptr) {
        HILOGW("DistributedSchedStub:: START_ABILITY userWant readParcelable failed!");
        return ERR_NULL_OBJECT;
    }
    unique_ptr<AbilityInfo> spAbilityInfo(data.ReadParcelable<AbilityInfo>());
    if (spAbilityInfo == nullptr) {
        HILOGW("DistributedSchedStub: StartRemoteAbilityInner AbilityInfo readParcelable failed!");
        return ERR_NULL_OBJECT;
    }
    int32_t requestCode = 0;
    PARCEL_READ_HELPER(data, Int32, requestCode);
    int32_t result = StartRemoteAbility(*userWant, *spAbilityInfo, requestCode);
    HILOGI("DistributedSchedStub:: StartRemoteAbilityInner result = %{public}d", result);
    PARCEL_WRITE_REPLY_NOERROR(reply, Int32, result);
}

int32_t DistributedSchedStub::StartAbilityFromRemoteInner(MessageParcel& data, MessageParcel& reply)
{
    if (!EnforceInterfaceToken(data)) {
        HILOGW("DistributedSchedStub:: StartAbilityFromRemoteInner interface token check failed!");
        return DMS_PERMISSION_DENIED;
    }
    shared_ptr<AAFwk::Want> userWant(data.ReadParcelable<AAFwk::Want>());
    if (userWant == nullptr) {
        HILOGW("DistributedSchedStub:: StartAbilityFromRemoteInner userWant readParcelable failed!");
        return ERR_NULL_OBJECT;
    }
    unique_ptr<AbilityInfo> spAbilityInfo(data.ReadParcelable<AbilityInfo>());
    if (spAbilityInfo == nullptr) {
        HILOGW("DistributedSchedStub: StartAbilityFromRemoteInner AbilityInfo readParcelable failed!");
        return ERR_NULL_OBJECT;
    }
    int32_t requestCode = 0;
    CallerInfo callerInfo;
    AccountInfo accountInfo;
    PARCEL_READ_HELPER(data, Int32, requestCode);
    int32_t result = StartAbilityFromRemote(*userWant, *spAbilityInfo, requestCode, callerInfo, accountInfo);
    HILOGI("DistributedSchedStub:: StartAbilityFromRemoteInner result = %{public}d", result);
    PARCEL_WRITE_REPLY_NOERROR(reply, Int32, result);
}

bool DistributedSchedStub::EnforceInterfaceToken(MessageParcel& data)
{
    u16string interfaceToken = data.ReadInterfaceToken();
    return interfaceToken == DMS_STUB_INTERFACE_TOKEN;
}
} // namespace DistributedSchedule
} // namespace OHOS
