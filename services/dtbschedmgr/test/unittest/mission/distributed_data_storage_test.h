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

#ifndef OHOS_DISTRIBUTED_DATA_STORAGE_TEST_H
#define OHOS_DISTRIBUTED_DATA_STORAGE_TEST_H

#include "gtest/gtest.h"

#include "mission/distributed_data_storage.h"

namespace OHOS {
namespace DistributedSchedule {
class DistributedDataStorageTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
protected:
    uint8_t* InitByteStream();
    void CompareByteStream(const uint8_t* byteStream1, const std::vector<uint8_t>& byteStream2);
    std::string GetLocalDeviceId() const;

    std::shared_ptr<DistributedDataStorage> distributedDataStorage_;
};
} // namespace DistributedSchedule
} // namespace OHOS
#endif /* OHOS_DISTRIBUTED_DATA_STORAGE_TEST_H */