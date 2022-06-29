/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef SERVICES_DTBSCHEDMGR_INCLUDE_IMISSION_CHANGED_LISTENER_H
#define SERVICES_DTBSCHEDMGR_INCLUDE_IMISSION_CHANGED_LISTENER_H

#include <list>

#include "iremote_broker.h"
#include "mission/snapshot.h"

namespace OHOS {
namespace DistributedSchedule {
class MissionChangedNotify {
public:
    static void NotifySnapshot(const sptr<IRemoteObject>& remoteObject, const std::u16string& deviceId,
        int32_t missionId);
    static void NotifyMissionsChanged(const sptr<IRemoteObject>& remoteObject, const std::u16string& deviceId);
    static void NotifyNetDisconnect(const sptr<IRemoteObject>& remoteObject, const std::u16string& deviceId,
        int32_t state);
};
} // namespace DistributedSchedule
} // namespace OHOS
#endif /* SERVICES_DTBSCHEDMGR_INCLUDE_IMISSION_CHANGED_LISTENER_H */
