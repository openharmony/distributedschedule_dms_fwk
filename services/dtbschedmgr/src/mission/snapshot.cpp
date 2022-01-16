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

#include "mission/snapshot.h"

#include <sstream>
#include "datetime_ex.h"
#include "dtbschedmgr_log.h"
#include "image_source.h"
#include "image_packer.h"
#include "parcel_helper.h"
#include "pixel_map_parcel.h"
#include "string_ex.h"

using namespace std;
using namespace OHOS::Media;

namespace OHOS {
namespace DistributedSchedule {
namespace {
const std::string TAG = "Snapshot";
constexpr int32_t COMPRESS_QUALITY = 85;
constexpr size_t PIXEL_MAP_MAX_BUFFER_SIZE = 600 * 1024;
constexpr size_t INT_BYTE = 4;
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
    PARCEL_WRITE_HELPER_RET(data, String16, appLabel_, false);
    PARCEL_WRITE_HELPER_RET(data, String16, abilityLabel_, false);
    PARCEL_WRITE_HELPER_RET(data, UInt8Vector, icon_, false);
    PARCEL_WRITE_HELPER_RET(data, String16, secAppLabel_, false);
    PARCEL_WRITE_HELPER_RET(data, String16, secAbilityLabel_, false);
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
    std::u16string appLabel = data.ReadString16();
    std::u16string abilityLabel = data.ReadString16();
    std::vector<uint8_t> icon;
    PARCEL_READ_HELPER_RET(data, UInt8Vector, &icon, nullptr);
    std::u16string secAppLabel = data.ReadString16();
    std::u16string secAbilityLabel = data.ReadString16();
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
    snapShot->appLabel_ = appLabel;
    snapShot->abilityLabel_ = abilityLabel;
    snapShot->icon_ = icon;
    snapShot->secAppLabel_ = secAppLabel;
    snapShot->secAbilityLabel_ = secAbilityLabel;
    snapShot->secIcon_ = secIcon;
    snapShot->sourceDeviceTips_ = sourceDeviceTips;
    return snapShot;
}

unique_ptr<PixelMap> Snapshot::CreatePixelMap(const uint8_t* buffer, uint32_t bufferSize)
{
    if (buffer == nullptr || bufferSize == 0) {
        HILOGE("Snapshot CreatePixelMap invalid params!");
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
    decodeOpt.desiredPixelFormat = PixelFormat::RGB_888;
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

bool Snapshot::WriteSnapshotInfo(MessageParcel& data) const
{
    MessageParcel parcel;
    PARCEL_WRITE_HELPER_RET(parcel, Int32, version_, false); // for dms version
    PARCEL_WRITE_HELPER_RET(parcel, Int32, orientation_, false); // for orientation
    PARCEL_WRITE_HELPER_RET(parcel, Parcelable, rect_.get(), false); // for contentInsets
    PARCEL_WRITE_HELPER_RET(parcel, Bool, reducedResolution_, false); // for reduceResolution
    PARCEL_WRITE_HELPER_RET(parcel, Float, scale_, false); // for scale
    PARCEL_WRITE_HELPER_RET(parcel, Bool, isRealSnapshot_, false); // for isRealSnapshot
    PARCEL_WRITE_HELPER_RET(parcel, Int32, windowingMode_, false); // for windowingMode
    PARCEL_WRITE_HELPER_RET(parcel, Int32, systemUiVisibility_, false); // for systemUiVisibility
    PARCEL_WRITE_HELPER_RET(parcel, Bool, isTranslucent_, false); // for isTranslucent
    PARCEL_WRITE_HELPER_RET(parcel, Parcelable, windowBounds_.get(), false); // for windowBounds
    PARCEL_WRITE_HELPER_RET(parcel, String16, appLabel_, false); // for appLabel
    PARCEL_WRITE_HELPER_RET(parcel, String16, abilityLabel_, false); // for abilityLabel
    PARCEL_WRITE_HELPER_RET(parcel, UInt8Vector, icon_, false); // for icon
    PARCEL_WRITE_HELPER_RET(parcel, String16, secAppLabel_, false); // for secAppLabel
    PARCEL_WRITE_HELPER_RET(parcel, String16, secAbilityLabel_, false); // for secAbilityLabel
    PARCEL_WRITE_HELPER_RET(parcel, UInt8Vector, secIcon_, false); // for secIcon
    PARCEL_WRITE_HELPER_RET(parcel, String16, sourceDeviceTips_, false); // for sourceDeviceTips
    size_t infoSize = parcel.GetReadableBytes();
    const uint8_t* infoBuffer = parcel.ReadBuffer(infoSize);
    PARCEL_WRITE_HELPER_RET(data, Uint32, infoSize, false); // for snapshot info size
    bool ret = data.WriteBuffer(infoBuffer, infoSize); // for snapshot info buffer
    if (!ret) {
        HILOGE("snapshot info write parcel failed!");
        return false;
    }
    return true;
}

bool Snapshot::WritePixelMap(MessageParcel& data) const
{
    ImagePacker imagePacker;
    PackOption option;
    option.format = "image/jpeg";
    option.quality = COMPRESS_QUALITY;
    option.numberHint = 1;
    stringstream ss;
    std::ostream outputStream(ss.rdbuf());
    imagePacker.StartPacking(outputStream, option);
    imagePacker.AddImage(*pixelMap_);
    imagePacker.FinalizePacking();
    std::istream inputStream(outputStream.rdbuf());
    inputStream.seekg(0, inputStream.end);
    size_t len = inputStream.tellg();
    inputStream.seekg(0);
    HILOGD("pixelMap compress size:%{public}zu", len);
    if (len > PIXEL_MAP_MAX_BUFFER_SIZE) {
        HILOGD("pixelMap size is too big.");
        return false;
    }
    std::unique_ptr<char[]> pixelMapBuffer = make_unique<char[]>(len);
    inputStream.read(pixelMapBuffer.get(), len);
    uint8_t* byteStream = reinterpret_cast<uint8_t*>(pixelMapBuffer.get());
    size_t minCapacity = data.GetReadableBytes() + len + INT_BYTE;
    if (minCapacity % INT_BYTE != 0) {
        HILOGI("bytes are not aligned!");
        minCapacity += INT_BYTE;
    }
    if (minCapacity > data.GetDataCapacity() && !data.SetDataCapacity(minCapacity)) {
        HILOGE("setCapacity failed! length = %{public}zu.", minCapacity);
        return false;
    }
    PARCEL_WRITE_HELPER_RET(data, Uint32, len, false); // for pixel map size
    bool ret = data.WriteBuffer(byteStream, len); // for pixel map buffer
    if (!ret) {
        HILOGE("pixel map write parcel failed!");
        return false;
    }
    return true;
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
