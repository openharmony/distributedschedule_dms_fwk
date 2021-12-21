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

#include "mission/mission_info_converter.h"

#include "dtbschedmgr_log.h"

using namespace std;

namespace OHOS {
namespace DistributedSchedule {
int32_t MissionInfoConverter::ConvertToMissionInfos(std::vector<AbilityMissionInfo>& abilityMissionInfos,
    std::vector<MissionInfo>& missionInfos)
{
    if (abilityMissionInfos.empty()) {
        return INVALID_PARAMETERS_ERR;
    }
    for (auto iter = abilityMissionInfos.begin(); iter != abilityMissionInfos.end(); iter++) {
        MissionInfo missionInfo;
        missionInfo.id  = iter->id;
        missionInfo.runingState = iter->runingState;
        missionInfo.missionStackId = iter->missionStackId;
        shared_ptr<AAFwk::Want> spWant = make_shared<AAFwk::Want>(iter->baseWant);
        missionInfo.baseWant = spWant;
        shared_ptr<AppExecFwk::ElementName> spTopAbility = make_shared<AppExecFwk::ElementName>(iter->topAbility);
        missionInfo.topAbility = spTopAbility;
        shared_ptr<AppExecFwk::ElementName> spBaseAbility = make_shared<AppExecFwk::ElementName>(iter->baseAbility);
        missionInfo.baseAbility = spBaseAbility;
        missionInfo.size = iter->size;
        missionInfo.label = iter->missionDescription.label;
        missionInfo.iconPath = iter->missionDescription.iconPath;
        missionInfos.push_back(missionInfo);
    }
    return ERR_OK;
}

int32_t MissionInfoConverter::ConvertToAbilityMissionInfos(std::vector<MissionInfo>& missionInfos,
    std::vector<AbilityMissionInfo>& abilityMissionInfos)
{
    if (missionInfos.empty()) {
        return INVALID_PARAMETERS_ERR;
    }
    for (auto iter = missionInfos.begin(); iter != missionInfos.end(); iter++) {
        AbilityMissionInfo missionInfo;
        missionInfo.id = iter->id;
        missionInfo.runingState = iter->runingState;
        missionInfo.missionStackId = iter->missionStackId;
        missionInfo.baseWant = *(iter->baseWant);
        missionInfo.topAbility = *(iter->topAbility);
        missionInfo.baseAbility = *(iter->baseAbility);
        missionInfo.size = iter->size;
        missionInfo.missionDescription.label = iter->label;
        missionInfo.missionDescription.iconPath = iter->iconPath;
        abilityMissionInfos.push_back(missionInfo);
    }
    return ERR_OK;
}
}
}
