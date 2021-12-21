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

#include "datetime_ex.h"
#include "dtbschedmgr_log.h"
#include "image_source.h"
#include "mission/snapshot.h"
#include "parcel_helper.h"
#include "pixel_map_parcel.h"
#include "string_ex.h"

using namespace std;
using namespace OHOS::Media;

namespace OHOS {
namespace DistributedSchedule {
namespace {
const std::string TAG = "Snapshot";
}

Snapshot::~Snapshot()
{
    HILOGD("Snapshot destruct!");
}

bool Snapshot::WriteToParcel(MessageParcel& data) const
{
    PARCEL_WRITE_HELPER_RET(data, Int32, 1, false);
    PARCEL_WRITE_HELPER_RET(data, Int32, orientation_, false);
    if (rect_ != nullptr) {
        PARCEL_WRITE_HELPER_RET(data, Parcelable, rect_.get(), false);
    } else {
        PARCEL_WRITE_HELPER_RET(data, Parcelable, nullptr, false);
    }
    PARCEL_WRITE_HELPER_RET(data, Bool, reducedResolution_, false);
    PARCEL_WRITE_HELPER_RET(data, Float, scale_, false);
    PARCEL_WRITE_HELPER_RET(data, Bool, isRealSnapshot_, false);
    PARCEL_WRITE_HELPER_RET(data, Int32, windowingMode_, false);
    PARCEL_WRITE_HELPER_RET(data, Int32, systemUiVisibility_, false);
    PARCEL_WRITE_HELPER_RET(data, Bool, isTranslucent_, false);
    if (windowBounds_ != nullptr) {
        PARCEL_WRITE_HELPER_RET(data, Parcelable, windowBounds_.get(), false);
    } else {
        PARCEL_WRITE_HELPER_RET(data, Parcelable, nullptr, false);
    }
    PARCEL_WRITE_HELPER_RET(data, String16, applicationLabel_, false);
    PARCEL_WRITE_HELPER_RET(data, String16, activityLabel_, false);
    PARCEL_WRITE_HELPER_RET(data, UInt8Vector, icon_, false);
    PARCEL_WRITE_HELPER_RET(data, String16, secApplicationLabel_, false);
    PARCEL_WRITE_HELPER_RET(data, String16, secActivityLabel_, false);
    PARCEL_WRITE_HELPER_RET(data, UInt8Vector, secIcon_, false);
    PARCEL_WRITE_HELPER_RET(data, String16, sourceDeviceTips_, false);
    if (pixelMap_ != nullptr) {
        PARCEL_WRITE_HELPER_RET(data, Int32, 1, false);
        bool ret = PixelMapParcel::WriteToParcel(pixelMap_.get(), data);
        if (!ret) {
            HILOGE("Snapshot WriteToParcel pixelMap failed!");
            return false;
        }
    } else {
        PARCEL_WRITE_HELPER_RET(data, Parcelable, nullptr, false);
    }
    return true;
}

unique_ptr<Snapshot> Snapshot::FillSnapShot(MessageParcel& data)
{
    int32_t version = 0;
    PARCEL_READ_HELPER_RET(data, Int32, version, nullptr);
    int32_t orientation = 0;
    PARCEL_READ_HELPER_RET(data, Int32, orientation, nullptr);
    unique_ptr<Rect> rect(data.ReadParcelable<Rect>());
    bool reduced = false;
    PARCEL_READ_HELPER_RET(data, Bool, reduced, nullptr);
    float scale = 0.0;
    PARCEL_READ_HELPER_RET(data, Float, scale, nullptr);
    bool isRealSnapshot = false;
    PARCEL_READ_HELPER_RET(data, Bool, isRealSnapshot, nullptr);
    int32_t windowingMode = 0;
    PARCEL_READ_HELPER_RET(data, Int32, windowingMode, nullptr);
    int32_t systemUiVisibility = 0;
    PARCEL_READ_HELPER_RET(data, Int32, systemUiVisibility, nullptr);
    bool isTranslucent = false;
    PARCEL_READ_HELPER_RET(data, Bool, isTranslucent, nullptr);
    unique_ptr<Rect> windowBounds(data.ReadParcelable<Rect>());
    std::u16string applicationLabel = data.ReadString16();
    std::u16string activityLabel = data.ReadString16();
    std::vector<uint8_t> icon;
    PARCEL_READ_HELPER_RET(data, UInt8Vector, &icon, nullptr);
    std::u16string secApplicationLabel = data.ReadString16();
    std::u16string secActivityLabel = data.ReadString16();
    std::vector<uint8_t> secIcon;
    PARCEL_READ_HELPER_RET(data, UInt8Vector, &secIcon, nullptr);
    std::u16string sourceDeviceTips = data.ReadString16();
    unique_ptr<Snapshot> snapShot = make_unique<Snapshot>();
    snapShot->version_ = version;
    snapShot->orientation_ = orientation;
    snapShot->rect_ = std::move(rect);
    snapShot->reducedResolution_ = reduced;
    snapShot->scale_ = scale;
    snapShot->isRealSnapshot_ = isRealSnapshot;
    snapShot->windowingMode_ = windowingMode;
    snapShot->systemUiVisibility_ = systemUiVisibility;
    snapShot->isTranslucent_ = isTranslucent;
    snapShot->windowBounds_ = std::move(windowBounds);
    snapShot->applicationLabel_ = applicationLabel;
    snapShot->activityLabel_ = activityLabel;
    snapShot->icon_ = icon;
    snapShot->secApplicationLabel_ = secApplicationLabel;
    snapShot->secActivityLabel_ = secActivityLabel;
    snapShot->secIcon_ = secIcon;
    snapShot->sourceDeviceTips_ = sourceDeviceTips;
    return snapShot;
}

unique_ptr<PixelMap> Snapshot::CreatePixelMap(const uint8_t* buffer, uint32_t bufferSize)
{
    if (buffer == nullptr || bufferSize == 0) {
        HILOGE("Snapshot CreatePixelMap invalid params !");
        return nullptr;
    }
    SourceOptions opts;
    uint32_t errCode = 0;
    opts.formatHint = "image/jpeg";
    unique_ptr<ImageSource> imageSource = ImageSource::CreateImageSource(buffer, bufferSize, opts, errCode);
    if (imageSource == nullptr) {
        HILOGE("Snapshot CreatePixelMap create image source failed!");
        return nullptr;
    }
    DecodeOptions decodeOpt;
    decodeOpt.allocatorType = AllocatorType::SHARE_MEM_ALLOC;
    decodeOpt.desiredPixelFormat = PixelFormat::RGB_565;
    int64_t begin = GetTickCount();
    unique_ptr<PixelMap> pixelMap = imageSource->CreatePixelMap(decodeOpt, errCode);
    HILOGI("[PerformanceTest] Create PixelMap spend %{public}" PRId64 " ms", GetTickCount() - begin);
    if (pixelMap == nullptr || errCode != 0) {
        HILOGW("Snapshot: CreatePixelMap failed, errCode:%{public}d!", errCode);
        return nullptr;
    }
    return pixelMap;
}

unique_ptr<Snapshot> Snapshot::Create(const vector<uint8_t>& data)
{
    auto dataBuffer = data.data();
    auto totalSize = data.size();
    if (totalSize < sizeof(int32_t)) {
        HILOGE("Snapshot invalid data !");
        return nullptr;
    }
    uint32_t msgSize = *reinterpret_cast<const uint32_t *>(dataBuffer);
    if (msgSize >= totalSize || msgSize + sizeof(uint32_t) >= totalSize) {
        HILOGE("Snapshot dataParcel invalid data !");
        return nullptr;
    }
    dataBuffer += sizeof(uint32_t);
    MessageParcel dataParcel;
    bool ret = dataParcel.WriteBuffer(dataBuffer, msgSize);
    if (!ret) {
        HILOGE("Snapshot dataParcel write failed!");
        return nullptr;
    }
    unique_ptr<Snapshot> snapShot = FillSnapShot(dataParcel);
    if (snapShot == nullptr) {
        HILOGW("Snapshot: FillSnapShot failed!");
        return nullptr;
    }
    dataBuffer += msgSize;
    uint32_t remainSize = totalSize - msgSize - sizeof(uint32_t);
    if (remainSize < sizeof(uint32_t)) {
        HILOGE("Snapshot dataParcel remainSize invalid!");
        return nullptr;
    }
    uint32_t pixelmapLen = *reinterpret_cast<const uint32_t *>(dataBuffer);
    if (pixelmapLen >= remainSize || pixelmapLen + sizeof(uint32_t) > remainSize) {
        HILOGE("Snapshot dataParcel buffer broken, should not happens!");
        return nullptr;
    }
    dataBuffer += sizeof(uint32_t);
    unique_ptr<PixelMap> pixelMap = CreatePixelMap(dataBuffer, pixelmapLen);
    if (pixelMap != nullptr) {
        HILOGD("Snapshot: dataParcel pixelMap width:%d, height:%d, byteCount:%d, pixelformat:%d",
            pixelMap->GetWidth(), pixelMap->GetHeight(), pixelMap->GetByteCount(),
            static_cast<int32_t>(pixelMap->GetPixelFormat()));
        snapShot->pixelMap_ = std::move(pixelMap);
    }
    snapShot->createdTime_ = GetTickCount();
    snapShot->lastAccessTime_ = snapShot->createdTime_;
    return snapShot;
}

int64_t Snapshot::GetCreatedTime() const
{
    return createdTime_;
}

int64_t Snapshot::GetLastAccessTime() const
{
    return lastAccessTime_;
}

void Snapshot::UpdateLastAccessTime(int64_t accessTime)
{
    lastAccessTime_ = accessTime;
}
}
}
