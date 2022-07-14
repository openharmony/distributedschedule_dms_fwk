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

#include "continuation_extra_params.h"

#include <iosfwd>
#include <new>

#include "dtbschedmgr_log.h"

namespace OHOS {
namespace DistributedSchedule {
namespace {
const std::string TAG = "ContinuationExtraParams";
}

bool ContinuationExtraParams::ReadFromParcel(Parcel &parcel)
{
    std::vector<std::u16string> deviceTypeVec;
    if (!parcel.ReadString16Vector(&deviceTypeVec)) {
        HILOGE("read device type failed.");
        return false;
    }
    SetDeviceType(ContinationManagerUtils::Str16VecToStr8Vec(deviceTypeVec));
    SetTargetBundle(Str16ToStr8(parcel.ReadString16()));
    SetDescription(Str16ToStr8(parcel.ReadString16()));
    SetFilter(Str16ToStr8(parcel.ReadString16()));
    SetContinuationMode(static_cast<ContinuationMode>(parcel.ReadInt32()));
    SetAuthInfo(Str16ToStr8(parcel.ReadString16()));
    return true;
}

ContinuationExtraParams *ContinuationExtraParams::Unmarshalling(Parcel &parcel)
{
    ContinuationExtraParams *continuationExtraParams = new (std::nothrow) ContinuationExtraParams();
    if (continuationExtraParams == nullptr) {
        return nullptr;
    }

    if (!continuationExtraParams->ReadFromParcel(parcel)) {
        delete continuationExtraParams;
        continuationExtraParams = nullptr;
    }

    return continuationExtraParams;
}

bool ContinuationExtraParams::Marshalling(Parcel &parcel) const
{
    parcel.WriteString16Vector(ContinationManagerUtils::Str8VecToStr16Vec(GetDeviceType()));
    parcel.WriteString16(Str8ToStr16(GetTargetBundle()));
    parcel.WriteString16(Str8ToStr16(GetDescription()));
    parcel.WriteString16(Str8ToStr16(GetFilter()));
    parcel.WriteInt32(static_cast<int32_t>(GetContinuationMode()));
    parcel.WriteString16(Str8ToStr16(GetAuthInfo()));
    return true;
}

void ContinuationExtraParams::SetDeviceType(std::vector<std::string> deviceTypeVec)
{
    deviceTypeVec_ = deviceTypeVec;
}

std::vector<std::string> ContinuationExtraParams::GetDeviceType() const
{
    return deviceTypeVec_;
}

void ContinuationExtraParams::SetTargetBundle(std::string targetBundle)
{
    targetBundle_ = targetBundle;
}

std::string ContinuationExtraParams::GetTargetBundle() const
{
    return targetBundle_;
}

void ContinuationExtraParams::SetDescription(std::string description)
{
    description_ = description;
}

std::string ContinuationExtraParams::GetDescription() const
{
    return description_;
}

void ContinuationExtraParams::SetFilter(std::string filter)
{
    filter_ = filter;
}

std::string ContinuationExtraParams::GetFilter() const
{
    return filter_;
}

void ContinuationExtraParams::SetContinuationMode(ContinuationMode continuationMode)
{
    continuationMode_ = continuationMode;
}

ContinuationMode ContinuationExtraParams::GetContinuationMode() const
{
    return continuationMode_;
}

void ContinuationExtraParams::SetAuthInfo(std::string authInfo)
{
    authInfo_ = authInfo;
}

std::string ContinuationExtraParams::GetAuthInfo() const
{
    return authInfo_;
}

std::vector<std::u16string> ContinationManagerUtils::Str8VecToStr16Vec(const std::vector<std::string>& input)
{
    std::vector<std::u16string> output;
    for (auto iter = input.begin(); iter != input.end(); iter++) {
        output.emplace_back(Str8ToStr16(*iter));
    }
    return output;
}

std::vector<std::string> ContinationManagerUtils::Str16VecToStr8Vec(const std::vector<std::u16string>& input)
{
    std::vector<std::string> output;
    for (auto iter = input.begin(); iter != input.end(); iter++) {
        output.emplace_back(Str16ToStr8(*iter));
    }
    return output;
}
}  // namespace DistributedSchedule
}  // namespace OHOS