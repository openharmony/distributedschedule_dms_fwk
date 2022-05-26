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

#ifndef OHOS_DMS_CALLBACK_TASK_H
#define OHOS_DMS_CALLBACK_TASK_H

#include <atomic>
#include <cstdint>
#include <map>
#include <mutex>

#include "event_handler.h"
#include "iremote_object.h"
#include "refbase.h"
#include "want.h"

namespace OHOS {
namespace DistributedSchedule {
using DmsCallbackTaskInitCallbackFunc = std::function<void(int64_t taskId)>;

enum class LaunchType {
    FREEINSTALL_START = 0,
    FREEINSTALL_CONTINUE = 1
};

struct CallbackTaskItem {
    sptr<IRemoteObject> callback = nullptr;
    int64_t taskId = -1;
    LaunchType launchType = LaunchType::FREEINSTALL_START;
    std::string deviceId = "";
    OHOS::AAFwk::Want want;
};

class DmsCallbackTask : public std::enable_shared_from_this<DmsCallbackTask> {
public:
    void Init(const DmsCallbackTaskInitCallbackFunc& callback);
    int64_t GenerateTaskId();
    int32_t PushCallback(int64_t taskId, const sptr<IRemoteObject>& callback, const std::string& deviceId,
        LaunchType launchType, const OHOS::AAFwk::Want& want);
    CallbackTaskItem PopCallback(int64_t taskId);
    void SetContinuationMissionMap(int64_t taskId, int32_t missionId);
    LaunchType GetLaunchType(int64_t taskId);
    void PopContinuationMissionMap(int64_t taskId);
    int64_t GetContinuaionMissionId(int64_t taskId);
    void NotifyDeviceOffline(const std::string& deviceId);

private:
    class DmsCallbackHandler : public AppExecFwk::EventHandler {
    public:
        DmsCallbackHandler(const std::shared_ptr<AppExecFwk::EventRunner>& runner,
            const std::shared_ptr<DmsCallbackTask>& callbackTask, const DmsCallbackTaskInitCallbackFunc& callback)
            : AppExecFwk::EventHandler(runner), callbackTask_(callbackTask), callback_(callback)
        {
        }
        ~DmsCallbackHandler() = default;

        void ProcessEvent(const OHOS::AppExecFwk::InnerEvent::Pointer& event) override;
    private:
        std::weak_ptr<DmsCallbackTask> callbackTask_;
        DmsCallbackTaskInitCallbackFunc callback_;
    };

    std::shared_ptr<DmsCallbackHandler> dmsCallbackHandler_;
    std::mutex callbackMapMutex_;
    std::mutex taskMutex_;
    std::atomic<int64_t> currTaskId_ {1};
    std::map<int64_t, CallbackTaskItem> callbackMap_;
    std::map<int64_t, int64_t> continuationMissionMap_;
};
}  // namespace DistributedSchedule
}  // namespace OHOS
#endif  // OHOS_DMS_CALLBACK_TASK_H
