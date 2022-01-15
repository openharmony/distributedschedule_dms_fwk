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

#ifndef SERVICES_DTBSCHEDMGR_INCLUDE_SNAPSHOT_SNAP_SHOT_H
#define SERVICES_DTBSCHEDMGR_INCLUDE_SNAPSHOT_SNAP_SHOT_H

#include <memory>

#include "dtbschedmgr_log.h"
#include "message_parcel.h"
#include "pixel_map.h"

namespace OHOS {
namespace DistributedSchedule {
struct Rect;
class Snapshot {
public:
    int32_t version_ = 0;
    int32_t orientation_ = 0;
    std::unique_ptr<Rect> rect_;
    bool reducedResolution_ = true;
    float scale_ = 0.0;
    bool isRealSnapshot_ = true;
    int32_t windowingMode_ = 0;
    int32_t systemUiVisibility_ = 0;
    bool isTranslucent_ = true;
    std::unique_ptr<Rect> windowBounds_;
    std::u16string appLabel_;
    std::u16string abilityLabel_;
    std::vector<uint8_t> icon_;
    std::u16string secAppLabel_;
    std::u16string secAbilityLabel_;
    std::vector<uint8_t> secIcon_;
    std::u16string sourceDeviceTips_;
    std::shared_ptr<Media::PixelMap> pixelMap_;

    ~Snapshot();
    bool WriteToParcel(MessageParcel& data) const;
    static std::unique_ptr<Snapshot> Create(const std::vector<uint8_t>& data);
    bool WriteSnapshotInfo(MessageParcel& data) const;
    bool WritePixelMap(MessageParcel& data) const;
    int64_t GetCreatedTime() const;
    int64_t GetLastAccessTime() const;
    void UpdateLastAccessTime(int64_t accessTime);
private:
    static std::unique_ptr<Media::PixelMap> CreatePixelMap(const uint8_t* buffer, uint32_t bufferSize);
    static std::unique_ptr<Snapshot> FillSnapShot(MessageParcel& data);

    // inner used
    int64_t createdTime_ = 0;
    int64_t lastAccessTime_ = 0;
};

struct Rect : public Parcelable {
    Rect(int32_t left, int32_t top, int32_t right, int32_t bottom)
    {
        this->left = left;
        this->top = top;
        this->right = right;
        this->bottom = bottom;
    }

    bool Marshalling(Parcel &parcel) const override
    {
        if (!parcel.WriteInt32(left)) {
            return false;
        }
        if (!parcel.WriteInt32(top)) {
            return false;
        }
        if (!parcel.WriteInt32(right)) {
            return false;
        }
        if (!parcel.WriteInt32(bottom)) {
            return false;
        }
        return true;
    }

    static Rect* Unmarshalling(Parcel &parcel)
    {
        int32_t left = parcel.ReadInt32();
        int32_t top = parcel.ReadInt32();
        int32_t right = parcel.ReadInt32();
        int32_t bottom = parcel.ReadInt32();
        auto rectPtr = new Rect(left, top, right, bottom);
        return rectPtr;
    }

    int32_t left = 0;
    int32_t top = 0;
    int32_t right = 0;
    int32_t bottom = 0;
};
} // DistributedSchedule
} // OHOS
#endif /* SERVICES_DTBSCHEDMGR_INCLUDE_SNAPSHOT_SNAP_SHOT_H */
