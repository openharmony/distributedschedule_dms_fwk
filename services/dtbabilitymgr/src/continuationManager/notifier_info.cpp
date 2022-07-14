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

#include "continuationManager/notifier_info.h"

namespace OHOS {
namespace DistributedSchedule {
namespace {
const std::string TAG = "NotifierInfo";
}

NotifierInfo::NotifierMap& NotifierInfo::GetNotifierMap()
{
    return notifierMap_;
}

sptr<IRemoteObject> NotifierInfo::GetNotifier(const std::string& cbType)
{
    return notifierMap_[cbType];
}

void NotifierInfo::SetNotifier(const std::string& cbType, const sptr<IRemoteObject>& notifier)
{
    notifierMap_[cbType] = notifier;
}

std::shared_ptr<ConnectStatusInfo> NotifierInfo::GetConnectStatusInfo() const
{
    return connectStatusInfo_;
}

void NotifierInfo::SetConnectStatusInfo(const std::shared_ptr<ConnectStatusInfo>& connectStatusInfo)
{
    connectStatusInfo_ = connectStatusInfo;
}
} // namespace DistributedSchedule
} // namespace OHOS