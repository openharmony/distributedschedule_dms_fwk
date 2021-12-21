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

#ifndef DISTRIBUTEDSCHED_MISSION_INFO_CONVERTER_H
#define DISTRIBUTEDSCHED_MISSION_INFO_CONVERTER_H

#include <string>
#include <vector>

#include "ability_mission_info.h"
#include "mission/mission_info.h"

using AbilityMissionInfo = OHOS::AAFwk::AbilityMissionInfo;

namespace OHOS {
namespace DistributedSchedule {
class MissionInfoConverter {
public:
    static int32_t ConvertToMissionInfos(std::vector<AbilityMissionInfo>& abilityMissionInfos,
        std::vector<MissionInfo>& missionInfos);
    static int32_t ConvertToAbilityMissionInfos(std::vector<MissionInfo>& missionInfos,
        std::vector<AbilityMissionInfo>& abilityMissionInfos);
};
}
}
#endif // DISTRIBUTEDSCHED_MISSION_INFO_CONVERTER_H
