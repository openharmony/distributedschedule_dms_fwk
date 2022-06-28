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

#include "continuation_result.h"

#include "dtbschedmgr_log.h"
#include "string_ex.h"

namespace OHOS {
namespace DistributedSchedule {
namespace {
const std::string TAG = "ContinuationResult";
constexpr int32_t VALUE_NULL = -1; // no object in parcel
constexpr int32_t VALUE_OBJECT = 1; // object exist in parcel
}

bool ContinuationResult::ReadFromParcel(Parcel &parcel)
{
    SetDeviceId(Str16ToStr8(parcel.ReadString16()));
    SetDeviceType(Str16ToStr8(parcel.ReadString16()));
    SetDeviceName(Str16ToStr8(parcel.ReadString16()));
    return true;
}

ContinuationResult *ContinuationResult::Unmarshalling(Parcel &parcel)
{
    ContinuationResult *continuationResult = new (std::nothrow) ContinuationResult();
    if (continuationResult == nullptr) {
        return nullptr;
    }

    if (!continuationResult->ReadFromParcel(parcel)) {
        delete continuationResult;
        continuationResult = nullptr;
    }

    return continuationResult;
}

bool ContinuationResult::Marshalling(Parcel &parcel) const
{
    parcel.WriteString16(Str8ToStr16(GetDeviceId()));
    parcel.WriteString16(Str8ToStr16(GetDeviceType()));
    parcel.WriteString16(Str8ToStr16(GetDeviceName()));
    return true;
}

void ContinuationResult::SetDeviceId(std::string deviceId)
{
    deviceId_ = deviceId;
}

std::string ContinuationResult::GetDeviceId() const
{
    return deviceId_;
}

void ContinuationResult::SetDeviceType(std::string deviceType)
{
    deviceType_ = deviceType;
}

std::string ContinuationResult::GetDeviceType() const
{
    return deviceType_;
}

void ContinuationResult::SetDeviceName(std::string deviceName)
{
    deviceName_ = deviceName;
}

std::string ContinuationResult::GetDeviceName() const
{
    return deviceName_;
}

bool ContinuationResult::ReadContinuationResultsFromParcel(Parcel& parcel,
    std::vector<ContinuationResult>& continuationResults)
{
    continuationResults.clear();
    int32_t empty = parcel.ReadInt32();
    if (empty == VALUE_OBJECT) {
        int32_t len = parcel.ReadInt32();
        HILOGD("read size: %{public}d", len);
        if (len < 0) {
            HILOGE("size Unmarshalling failed");
            return false;
        }
        size_t size = static_cast<size_t>(len);
        if ((size > parcel.GetReadableBytes()) || (continuationResults.max_size() < size)) {
            HILOGE("size convert failed, size = %{public}zu", size);
            return false;
        }
        for (size_t i = 0; i < size; i++) {
            ContinuationResult* continuationResult = parcel.ReadParcelable<ContinuationResult>();
            if (continuationResult == nullptr) {
                HILOGE("ContinuationResult Unmarshalling failed");
                return false;
            }
            continuationResults.emplace_back(*continuationResult);
            delete continuationResult;
        }
    }
    return true;
}

bool ContinuationResult::WriteContinuationResultsToParcel(Parcel& parcel,
    const std::vector<ContinuationResult>& continuationResults)
{
    size_t size = continuationResults.size();
    if (size == 0) {
        if (!parcel.WriteInt32(VALUE_NULL)) {
            return false;
        }
        return true;
    }
    if (!parcel.WriteInt32(VALUE_OBJECT)) {
        return false;
    }
    if (!parcel.WriteInt32(static_cast<int32_t>(size))) {
        return false;
    }
    for (auto& continuationResult : continuationResults) {
        if (!parcel.WriteParcelable(&continuationResult)) {
            return false;
        }
    }
    return true;
}
}  // namespace DistributedSchedule
}  // namespace OHOS