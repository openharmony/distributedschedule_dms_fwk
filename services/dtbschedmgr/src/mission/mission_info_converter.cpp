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
int32_t MissionInfoConverter::ConvertToDstbMissionInfos(std::vector<AAFwk::MissionInfo>& missionInfos,
    std::vector<DstbMissionInfo>& dstbMissionInfos)
{
    if (missionInfos.empty()) {
        return INVALID_PARAMETERS_ERR;
    }
    for (auto iter = missionInfos.begin(); iter != missionInfos.end(); iter++) {
        DstbMissionInfo dstbMissionInfo;
        dstbMissionInfo.id  = iter->id;
        dstbMissionInfo.runingState = iter->runningState;
        dstbMissionInfo.lockedState = iter->lockedState;
        dstbMissionInfo.label = iter->label;
        dstbMissionInfo.iconPath = iter->iconPath;
        shared_ptr<AAFwk::Want> spWant = make_shared<AAFwk::Want>(iter->want);
        dstbMissionInfo.baseWant = spWant;
        dstbMissionInfos.push_back(dstbMissionInfo);
    }
    return ERR_OK;
}

int32_t MissionInfoConverter::ConvertToMissionInfos(std::vector<DstbMissionInfo>& dstbMissionInfos,
    std::vector<AAFwk::MissionInfo>& missionInfos)
{
    if (dstbMissionInfos.empty()) {
        return INVALID_PARAMETERS_ERR;
    }
    for (auto iter = dstbMissionInfos.begin(); iter != dstbMissionInfos.end(); iter++) {
        AAFwk::MissionInfo missionInfo;
        missionInfo.id = iter->id;
        missionInfo.runningState = iter->runingState;
        missionInfo.lockedState = iter->lockedState;
        missionInfo.label = iter->label;
        missionInfo.iconPath = iter->iconPath;
        missionInfo.want = *(iter->baseWant);
        missionInfos.push_back(missionInfo);
    }
    return ERR_OK;
}
}
}
