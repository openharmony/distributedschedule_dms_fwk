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

#include "mission/distributed_mission_info.h"

#include "adapter/adapter_constant.h"
#include "dtbschedmgr_log.h"
#include "parcel_helper.h"
#include "string_ex.h"

namespace OHOS {
namespace DistributedSchedule {
using namespace Constants::Adapter;
namespace {
const std::string TAG = "DstbMissionInfo";
}

bool DstbMissionInfo::ReadFromParcel(Parcel& parcel)
{
    id = parcel.ReadInt32();
    runingState = parcel.ReadInt32();
    userId = parcel.ReadInt32();
    missionStackId = parcel.ReadInt32();
    if (!parcel.ReadInt32Vector(&combinedMissionIds)) {
        return false;
    }
    windowMode = parcel.ReadInt32();
    boundsLeft = parcel.ReadInt32();
    boundsTop = parcel.ReadInt32();
    boundsRight = parcel.ReadInt32();
    boundsBottom = parcel.ReadInt32();
    baseWant.reset(parcel.ReadParcelable<AAFwk::Want>());
    topAbility.reset(parcel.ReadParcelable<AppExecFwk::ElementName>());
    baseAbility.reset(parcel.ReadParcelable<AppExecFwk::ElementName>());
    reservedAbility.reset(parcel.ReadParcelable<AppExecFwk::ElementName>());
    lastMissionActiveTime = parcel.ReadInt64();
    displayId = parcel.ReadInt32();
    label = Str16ToStr8(parcel.ReadString16());
    size = parcel.ReadInt32();
    iconPath = Str16ToStr8(parcel.ReadString16());
    color = parcel.ReadInt32();
    windowType = parcel.ReadInt32();
    lockedState = parcel.ReadBool();
    missionType = parcel.ReadInt32();
    windowTypeMode = parcel.ReadInt32();

    return true;
}

DstbMissionInfo* DstbMissionInfo::Unmarshalling(Parcel& parcel)
{
    DstbMissionInfo* info = new DstbMissionInfo();
    if (info && !info->ReadFromParcel(parcel)) {
        HILOGE("read from parcel failed!");
        delete info;
        info = nullptr;
    }
    return info;
}

bool DstbMissionInfo::Marshalling(Parcel& parcel) const
{
    PARCEL_WRITE_HELPER_RET(parcel, Int32, id, false);
    PARCEL_WRITE_HELPER_RET(parcel, Int32, runingState, false);
    PARCEL_WRITE_HELPER_RET(parcel, Int32, userId, false);
    PARCEL_WRITE_HELPER_RET(parcel, Int32, missionStackId, false);
    PARCEL_WRITE_HELPER_RET(parcel, Int32Vector, combinedMissionIds, false);
    PARCEL_WRITE_HELPER_RET(parcel, Int32, windowMode, false);
    PARCEL_WRITE_HELPER_RET(parcel, Int32, boundsLeft, false);
    PARCEL_WRITE_HELPER_RET(parcel, Int32, boundsTop, false);
    PARCEL_WRITE_HELPER_RET(parcel, Int32, boundsRight, false);
    PARCEL_WRITE_HELPER_RET(parcel, Int32, boundsBottom, false);
    PARCEL_WRITE_HELPER_RET(parcel, Parcelable, baseWant.get(), false);
    PARCEL_WRITE_HELPER_RET(parcel, Parcelable, topAbility.get(), false);
    PARCEL_WRITE_HELPER_RET(parcel, Parcelable, baseAbility.get(), false);
    PARCEL_WRITE_HELPER_RET(parcel, Parcelable, reservedAbility.get(), false);
    PARCEL_WRITE_HELPER_RET(parcel, Int64, lastMissionActiveTime, false);
    PARCEL_WRITE_HELPER_RET(parcel, Int32, displayId, false);
    PARCEL_WRITE_HELPER_RET(parcel, String16, Str8ToStr16(label), false);
    PARCEL_WRITE_HELPER_RET(parcel, Int32, size, false);
    PARCEL_WRITE_HELPER_RET(parcel, String16, Str8ToStr16(iconPath), false);
    PARCEL_WRITE_HELPER_RET(parcel, Int32, color, false);
    PARCEL_WRITE_HELPER_RET(parcel, Int32, windowType, false);
    PARCEL_WRITE_HELPER_RET(parcel, Bool, lockedState, false);
    PARCEL_WRITE_HELPER_RET(parcel, Int32, missionType, false);
    PARCEL_WRITE_HELPER_RET(parcel, Int32, windowTypeMode, false);

    return true;
}

bool DstbMissionInfo::ReadDstbMissionInfosFromParcel(Parcel& parcel,
    std::vector<DstbMissionInfo>& missionInfos)
{
    int32_t empty = parcel.ReadInt32();
    if (empty == VALUE_OBJECT) {
        int32_t len = parcel.ReadInt32();
        HILOGD("ReadDstbMissionInfosFromParcel::readLength is:%{public}d", len);
        if (len < 0) {
            return false;
        }
        size_t size = static_cast<size_t>(len);
        if ((size > parcel.GetReadableBytes()) || (missionInfos.max_size() < size)) {
            HILOGE("Failed to read DstbMissionInfo vector, size = %{public}zu", size);
            return false;
        }
        missionInfos.clear();
        for (size_t i = 0; i < size; i++) {
            DstbMissionInfo *ptr = parcel.ReadParcelable<DstbMissionInfo>();
            if (ptr == nullptr) {
                return false;
            }
            HILOGD("read DstbMissionInfo is:%{private}s", ptr->ToString().c_str());
            missionInfos.emplace_back(*ptr);
            delete ptr;
        }
    }

    HILOGI("read ReadDstbMissionInfosFromParcel end. info size is:%{public}zu", missionInfos.size());
    return true;
}

bool DstbMissionInfo::WriteDstbMissionInfosToParcel(Parcel& parcel,
    const std::vector<DstbMissionInfo>& missionInfos)
{
    size_t size = missionInfos.size();
    if (size == 0) {
        PARCEL_WRITE_HELPER_RET(parcel, Int32, VALUE_NULL, false);
        return true;
    }

    PARCEL_WRITE_HELPER_RET(parcel, Int32, VALUE_OBJECT, false);
    PARCEL_WRITE_HELPER_RET(parcel, Int32, size, false);
    for (auto& info : missionInfos) {
        PARCEL_WRITE_HELPER_RET(parcel, Parcelable, &info, false);
    }
    return true;
}

std::string DstbMissionInfo::ToString() const
{
    std::string str = "id: " + std::to_string(id);
    str += " runingState: " + std::to_string(runingState);
    str += " userId: " + std::to_string(userId);
    str += " missionStackId: " + std::to_string(missionStackId);
    str += " windowMode: " + std::to_string(windowMode);
    str += " boundsLeft: " + std::to_string(boundsLeft);
    str += " boundsTop: " + std::to_string(boundsTop);
    str += " boundsRight: " + std::to_string(boundsRight);
    str += " boundsBottom: " + std::to_string(boundsBottom);
    if (baseWant == nullptr) {
        str += " baseWant: nullptr";
    } else {
        str += " baseWant: " + baseWant->GetBundle();
    }
    if (topAbility == nullptr) {
        str += " topAbility: nullptr";
    } else {
        str += " topAbility: " + topAbility->GetBundleName();
    }
    if (baseAbility == nullptr) {
        str += " baseAbility: nullptr";
    } else {
        str += " baseAbility: " + baseAbility->GetBundleName();
    }
    if (reservedAbility == nullptr) {
        str += " reservedAbility: nullptr";
    } else {
        str += " reservedAbility: " + reservedAbility->GetBundleName();
    }
    str += " lastMissionActiveTime: " + std::to_string(lastMissionActiveTime);
    str += " displayId: " + std::to_string(displayId);
    str += " label: " + label;
    str += " size: " + std::to_string(size);
    str += " iconPath: " + iconPath;
    str += " color: " + std::to_string(color);
    str += " windowType: " + std::to_string(windowType);
    str += " lockedState: ";
    str += lockedState ? "true" : "false";
    str += " missionType: " + std::to_string(missionType);
    str += " windowTypeMode: " + std::to_string(windowTypeMode);
    return str;
}
} // namespace DistributedSchedule
} // namespace OHOS