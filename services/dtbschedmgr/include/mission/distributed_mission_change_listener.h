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

#ifndef OHOS_DISTRIBUTED_MISSION_CHANGE_LISTENER_H
#define OHOS_DISTRIBUTED_MISSION_CHANGE_LISTENER_H

#include "event_handler.h"
#include "mission_listener_stub.h"

namespace OHOS {
namespace DistributedSchedule {
class DistributedMissionChangeListener : public AAFwk::MissionListenerStub {
public:
    DistributedMissionChangeListener() = default;
    virtual ~DistributedMissionChangeListener() = default;

    /**
     * @brief When a mission is created, AbilityMs notifies the listener of the mission id
     *
     * @param missionId, mission Id.
     */
    void OnMissionCreated(int32_t missionId) override;

    /**
     * @brief When a mission is destroyed, AbilityMs notifies the listener of the mission id
     *
     * @param missionId, mission Id.
     */
    void OnMissionDestroyed(int32_t missionId) override;

    /**
     * @brief When the snapshot of a mission changes, AbilityMs notifies the listener of the mission id
     *
     * @param missionId, mission Id.
     */
    void OnMissionSnapshotChanged(int32_t missionId) override;

    /**
     * @brief When a mission is moved to front, AbilityMs notifies the listener of the mission id
     *
     * @param missionId, mission Id.
     */
    void OnMissionMovedToFront(int32_t missionId) override;
};
} // namespace DistributedSchedule
} // namespace OHOS
#endif // OHOS_DISTRIBUTED_MISSION_CHANGE_LISTENER_H

