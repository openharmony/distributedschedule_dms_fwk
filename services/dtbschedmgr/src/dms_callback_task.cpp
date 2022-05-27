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

#include "dms_callback_task.h"

#include "dtbschedmgr_log.h"
#include "distributed_sched_service.h"
#include "parcel_helper.h"

namespace OHOS {
namespace DistributedSchedule {
using namespace OHOS::AppExecFwk;

namespace {
constexpr int64_t CALLBACK_DELAY_TIME = 30000;
const std::string TAG = "DmsCallbackTask";
}

void DmsCallbackTask::Init(const DmsCallbackTaskInitCallbackFunc& callback)
{
    auto runner = EventRunner::Create("dsched_callback");
    dmsCallbackHandler_ = std::make_shared<DmsCallbackHandler>(runner, shared_from_this(), callback);
}

int64_t DmsCallbackTask::GenerateTaskId()
{
    std::lock_guard<std::mutex> autoLock(taskMutex_);
    int64_t currValue = currTaskId_.load(std::memory_order_relaxed);
    if (++currTaskId_ <= 0) {
        currTaskId_ = 1;
    }
    return currValue;
}

int32_t DmsCallbackTask::PushCallback(int64_t taskId, const sptr<IRemoteObject>& callback,
    const std::string& deviceId, LaunchType launchType, const OHOS::AAFwk::Want& want)
{
    HILOGI("PushCallback taskId:%{public}" PRId64, taskId);
    if (taskId <= 0) {
        HILOGE("PushCallback taskId invalid!");
        return INVALID_PARAMETERS_ERR;
    }

    if (launchType == LaunchType::FREEINSTALL_START) {
        if (callback == nullptr) {
            HILOGE("PushCallback callback null!");
            return INVALID_PARAMETERS_ERR;
        }
    }

    if (dmsCallbackHandler_ == nullptr) {
        HILOGE("PushCallback not initialized!");
        return INVALID_PARAMETERS_ERR;
    }

    std::lock_guard<std::mutex> autoLock(callbackMapMutex_);
    auto iterTask = callbackMap_.find(taskId);
    if (iterTask != callbackMap_.end()) {
        HILOGE("PushCallback taskId:%{public}" PRId64 " exist!", taskId);
        return INVALID_PARAMETERS_ERR;
    }

    bool ret = dmsCallbackHandler_->SendEvent(taskId, 0, CALLBACK_DELAY_TIME);
    if (!ret) {
        HILOGE("PushCallback SendEvent failed!");
        return INVALID_PARAMETERS_ERR;
    }

    CallbackTaskItem item {
        .callback = callback,
        .taskId = taskId,
        .launchType = launchType,
        .deviceId = deviceId,
        .want = want,
    };
    (void)callbackMap_.emplace(taskId, item);
    return ERR_OK;
}

CallbackTaskItem DmsCallbackTask::PopCallback(int64_t taskId)
{
    std::lock_guard<std::mutex> autoLock(callbackMapMutex_);
    auto iter = callbackMap_.find(taskId);
    CallbackTaskItem item = {};
    if (iter == callbackMap_.end()) {
        HILOGW("PopCallback not found taskId:%{public}" PRId64 "!", taskId);
        return item;
    }
    item = iter->second;
    (void)callbackMap_.erase(iter);
    if (dmsCallbackHandler_ != nullptr) {
        dmsCallbackHandler_->RemoveEvent(taskId);
    }
    return item;
}

void DmsCallbackTask::PopContinuationMissionMap(int64_t taskId)
{
    std::lock_guard<std::mutex> autoLock(callbackMapMutex_);
    int32_t ret = continuationMissionMap_.erase(taskId);
    if (ret != 1) {
        HILOGW("GetContinuaionMissionId not found taskId:%{public}" PRId64 "!", taskId);
    }
}

int64_t DmsCallbackTask::GetContinuaionMissionId(int64_t taskId)
{
    if (taskId <= 0) {
        return INVALID_PARAMETERS_ERR;
    }

    std::lock_guard<std::mutex> autoLock(callbackMapMutex_);
    auto iter = continuationMissionMap_.find(taskId);
    if (iter == continuationMissionMap_.end()) {
        HILOGW("GetContinuaionMissionId not found taskId:%{public}" PRId64 "!", taskId);
        return INVALID_PARAMETERS_ERR;
    }
    HILOGI("GetContinuaionMissionId end, taskId :%{public}" PRId64 ".", taskId);
    return iter->second;
}

void DmsCallbackTask::SetContinuationMissionMap(int64_t taskId, int32_t missionId)
{
    HILOGI("taskId = %{public}" PRId64 ", missionId = %{public}d.", taskId, missionId);
    if (taskId <= 0 || missionId <= 0) {
        HILOGD("param invalid");
        return;
    }
    std::lock_guard<std::mutex> autoLock(callbackMapMutex_);
    auto itreator = continuationMissionMap_.find(taskId);
    if (itreator != continuationMissionMap_.end()) {
        HILOGD("taskId = %{public}" PRId64 ".", taskId);
        return;
    }
    continuationMissionMap_[taskId] = missionId;
}

LaunchType DmsCallbackTask::GetLaunchType(int64_t taskId)
{
    if (taskId <= 0) {
        HILOGD("GetLaunchType param taskId invalid");
        return LaunchType::FREEINSTALL_START;
    }
    std::lock_guard<std::mutex> autoLock(callbackMapMutex_);
    auto iterTask = callbackMap_.find(taskId);
    if (iterTask == callbackMap_.end()) {
        HILOGE("GetLaunchType not found taskId : %{public}" PRId64 "!", taskId);
        return LaunchType::FREEINSTALL_START;
    }
    CallbackTaskItem item = iterTask->second;
    return item.launchType;
}

void DmsCallbackTask::NotifyDeviceOffline(const std::string& deviceId)
{
    HILOGI("%{public}s begin.", __func__);
    std::lock_guard<std::mutex> autoLock(callbackMapMutex_);
    for (auto it = callbackMap_.begin(); it != callbackMap_.end();) {
        if (it->second.deviceId == deviceId) {
            if (dmsCallbackHandler_ != nullptr) {
                dmsCallbackHandler_->RemoveEvent(it->second.taskId);
            }
            DistributedSchedService::GetInstance().NotifyFreeInstallResult(it->second, DEVICE_OFFLINE_ERR);
            (void)callbackMap_.erase(it++);
        } else {
            it++;
        }
    }
}

void DmsCallbackTask::DmsCallbackHandler::ProcessEvent(const InnerEvent::Pointer& event)
{
    if (event == nullptr) {
        HILOGE("ProcessEvent event nullptr!");
        return;
    }

    auto eventId = event->GetInnerEventId();
    int64_t taskId = static_cast<int64_t>(eventId);
    if (taskId <= 0) {
        HILOGW("ProcessEvent taskId invalid!");
        return;
    }

    if (callback_ != nullptr) {
        callback_(taskId);
    }
}
}  // namespace DistributedSchedule
}  // namespace OHOS
