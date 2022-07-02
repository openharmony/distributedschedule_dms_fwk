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

#include "distributed_sched_continuation.h"
#include "dtbschedmgr_log.h"
#include "parcel_helper.h"

using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace DistributedSchedule {
namespace {
constexpr int64_t CONTINUATION_DELAY_TIME = 20000;
const std::string TAG = "DSchedContinuation";
const std::u16string NAPI_MISSION_CENTER_INTERFACE_TOKEN = u"ohos.DistributedSchedule.IMissionCallback";
constexpr int32_t NOTIFY_MISSION_CENTER_RESULT = 4;
}

void DSchedContinuation::Init(const FuncContinuationCallback& contCallback)
{
    auto runner = EventRunner::Create("dsched_continuation");
    continuationHandler_ = std::make_shared<ContinuationHandler>(runner, shared_from_this(), contCallback);
}

bool DSchedContinuation::PushAbilityToken(int32_t sessionId, const sptr<IRemoteObject>& abilityToken)
{
    if (abilityToken == nullptr) {
        HILOGE("PushAbilityToken abilityToken null!");
        return false;
    }

    if (sessionId <= 0) {
        HILOGE("PushAbilityToken sessionId invalid!");
        return false;
    }

    if (continuationHandler_ == nullptr) {
        HILOGE("PushAbilityToken not initialized!");
        return false;
    }

    std::lock_guard<std::mutex> autoLock(continuationLock_);
    bool ret = true;
    ret = continuationHandler_->SendEvent(sessionId, 0, CONTINUATION_DELAY_TIME);
    if (!ret) {
        HILOGE("PushAbilityToken SendEvent failed!");
        return false;
    }

    auto iterSession = continuationMap_.find(sessionId);
    if (iterSession != continuationMap_.end()) {
        HILOGE("PushAbilityToken sessionId:%{public}d exist!", sessionId);
        return false;
    }
    (void)continuationMap_.emplace(sessionId, abilityToken);
    return true;
}

sptr<IRemoteObject> DSchedContinuation::PopAbilityToken(int32_t sessionId)
{
    if (sessionId <= 0) {
        HILOGE("PopAbilityToken sessionId invalid");
        return nullptr;
    }

    std::lock_guard<std::mutex> autoLock(continuationLock_);
    auto iter = continuationMap_.find(sessionId);
    if (iter == continuationMap_.end()) {
        HILOGW("PopAbilityToken not found sessionId:%{public}d", sessionId);
        return nullptr;
    }
    sptr<IRemoteObject> abilityToken = iter->second;
    (void)continuationMap_.erase(iter);
    if (continuationHandler_ != nullptr) {
        continuationHandler_->RemoveEvent(sessionId);
    }
    return abilityToken;
}

int32_t DSchedContinuation::GenerateSessionId()
{
    std::lock_guard<std::mutex> autoLock(continuationLock_);
    int32_t currValue = currSessionId_;
    if (++currSessionId_ <= 0) {
        currSessionId_ = 1;
    }
    return currValue;
}

void DSchedContinuation::SetTimeOut(int32_t missionId, int32_t timeout)
{
    if (continuationHandler_ == nullptr) {
        HILOGE("continuationHandler not initialized!");
        return;
    }
    continuationHandler_->SendEvent(missionId, 0, timeout);
}

void DSchedContinuation::RemoveTimeOut(int32_t missionId)
{
    if (continuationHandler_ == nullptr) {
        HILOGE("continuationHandler not initialized!");
        return;
    }
    continuationHandler_->RemoveEvent(missionId);
}

bool DSchedContinuation::IsFreeInstall(int32_t missionId)
{
    std::lock_guard<std::mutex> autoLock(continuationLock_);
    auto iter = freeInstall_.find(missionId);
    if (iter != freeInstall_.end()) {
        HILOGD("continue free install, missionId:%{public}d exist!", missionId);
        return iter->second;
    }
    return false;
}

bool DSchedContinuation::IsInContinuationProgress(int32_t missionId)
{
    std::lock_guard<std::mutex> autoLock(continuationLock_);
    auto iterSession = callbackMap_.find(missionId);
    if (iterSession != callbackMap_.end()) {
        HILOGE("Continuation in progress, missionId:%{public}d exist!", missionId);
        return true;
    }
    return false;
}

bool DSchedContinuation::PushCallback(int32_t missionId, const sptr<IRemoteObject>& callback, bool isFreeInstall)
{
    HILOGI("DSchedContinuation PushCallback start!");
    if (callback == nullptr) {
        HILOGE("PushCallback callback null!");
        return false;
    }

    if (continuationHandler_ == nullptr) {
        HILOGE("PushCallback not initialized!");
        return false;
    }

    std::lock_guard<std::mutex> autoLock(continuationLock_);
    auto iterSession = callbackMap_.find(missionId);
    if (iterSession != callbackMap_.end()) {
        HILOGE("PushCallback missionId:%{public}d exist!", missionId);
        return false;
    }
    (void)callbackMap_.emplace(missionId, callback);
    if (isFreeInstall) {
        freeInstall_[missionId] = isFreeInstall;
    }
    return true;
}

sptr<IRemoteObject> DSchedContinuation::PopCallback(int32_t missionId)
{
    std::lock_guard<std::mutex> autoLock(continuationLock_);
    auto iter = callbackMap_.find(missionId);
    if (iter == callbackMap_.end()) {
        HILOGW("PopCallback not found missionId:%{public}d", missionId);
        return nullptr;
    }
    sptr<IRemoteObject> callback = iter->second;
    auto it = freeInstall_.find(missionId);
    if (it != freeInstall_.end()) {
        HILOGD("%{public}d need pop from freeInstall_", missionId);
        (void)freeInstall_.erase(it);
    }
    (void)callbackMap_.erase(iter);
    return callback;
}

int32_t DSchedContinuation::NotifyMissionCenterResult(int32_t missionId, int32_t isSuccess)
{
    sptr<IRemoteObject> callback = PopCallback(missionId);
    RemoveTimeOut(missionId);
    if (callback == nullptr) {
        HILOGE("NotifyMissionCenterResult callback is null");
        return INVALID_PARAMETERS_ERR;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(NAPI_MISSION_CENTER_INTERFACE_TOKEN)) {
        HILOGE("NotifyMissionCenterResult write token failed");
        return INVALID_PARAMETERS_ERR;
    }
    PARCEL_WRITE_HELPER_RET(data, Int32, isSuccess, INVALID_PARAMETERS_ERR);
    MessageParcel reply;
    MessageOption option;
    int32_t error = callback->SendRequest(NOTIFY_MISSION_CENTER_RESULT, data, reply, option);
    HILOGI("NotifyMissionCenterResult transact result: %{public}d", error);
    return error;
}

void DSchedContinuation::ContinuationHandler::ProcessEvent(const InnerEvent::Pointer& event)
{
    if (event == nullptr) {
        HILOGE("ProcessEvent event nullptr!");
        return;
    }

    auto eventId = event->GetInnerEventId();
    int32_t sessionId = static_cast<int32_t>(eventId);
    if (sessionId <= 0) {
        HILOGW("ProcessEvent sessionId invalid!");
        return;
    }

    if (contCallback_ != nullptr) {
        contCallback_(sessionId);
    }
}
} // namespace DistributedSchedule
} // namespace OHOS
