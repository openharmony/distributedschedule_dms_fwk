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

#ifndef OHOS_DISTRIBUTED_MISSION_INFO_H
#define OHOS_DISTRIBUTED_MISSION_INFO_H

#include <string>

#include "parcel.h"

#include "application_info.h"
#include "element_name.h"
#include "ohos/aafwk/content/want.h"

namespace OHOS {
namespace DistributedSchedule {
struct MissionInfo : public Parcelable {
    int32_t id = -1;
    int32_t runingState = -1;
    int32_t userId = 0;
    int32_t missionStackId = -1;
    std::vector<int32_t> combinedMissionIds;
    int32_t windowMode = 0;
    int32_t boundsLeft = 0;
    int32_t boundsTop = 0;
    int32_t boundsRight = 0;
    int32_t boundsBottom = 0;
    std::shared_ptr<AAFwk::Want> baseWant;
    std::shared_ptr<AppExecFwk::ElementName> topAbility;
    std::shared_ptr<AppExecFwk::ElementName> baseAbility;
    std::shared_ptr<AppExecFwk::ElementName> reservedAbility;
    int64_t lastMissionActiveTime = 0;
    int32_t displayId = 0;
    std::string label;
    int32_t size = 0;
    std::string iconPath;
    int32_t color = 0;
    int32_t windowType = 0;
    bool supportsMultiWindow = false;
    int32_t missionType = INT_MAX;
    int32_t windowTypeMode = 0;

    bool ReadFromParcel(Parcel& parcel);
    virtual bool Marshalling(Parcel& parcel) const override;
    static MissionInfo* Unmarshalling(Parcel& parcel);
    std::string ToString() const;

    static bool ReadMissionInfoVectorFromParcel(Parcel& parcel, std::vector<MissionInfo> &missionInfos);
    static bool WriteMissionInfoVectorFromParcel(Parcel& parcel, const std::vector<MissionInfo> &missionInfos);
};
} // namespace DistributedSchedule
} // namespace OHOS
#endif // OHOS_DISTRIBUTED_MISSION_INFO_H
