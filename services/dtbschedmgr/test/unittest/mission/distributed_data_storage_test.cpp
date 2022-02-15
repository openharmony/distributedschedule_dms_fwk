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

#include "distributed_data_storage_test.h"

#include <thread>
#include "dtbschedmgr_device_info_storage.h"
#include "test_log.h"

namespace OHOS {
namespace DistributedSchedule {
using namespace std;
using namespace testing;
using namespace testing::ext;
using namespace OHOS::DistributedKv;
namespace {
constexpr int32_t TASK_ID_1 = 11;
constexpr int32_t TASK_ID_2 = 12;
constexpr size_t BYTESTREAM_LENGTH = 100;
constexpr uint8_t ONE_BYTE = '6';
}

void DistributedDataStorageTest::SetUpTestCase()
{
    DTEST_LOG << "DistributedDataStorageTest::SetUpTestCase" << std::endl;
}

void DistributedDataStorageTest::TearDownTestCase()
{
    DTEST_LOG << "DistributedDataStorageTest::TearDownTestCase" << std::endl;
}

void DistributedDataStorageTest::SetUp()
{
    distributedDataStorage_ = std::make_shared<DistributedDataStorage>();
    DTEST_LOG << "DistributedDataStorageTest::SetUp" << std::endl;
}

void DistributedDataStorageTest::TearDown()
{
    DTEST_LOG << "DistributedDataStorageTest::TearDown" << std::endl;
}

uint8_t* DistributedDataStorageTest::InitByteStream()
{
    uint8_t* byteStream = new uint8_t[BYTESTREAM_LENGTH];
    for (size_t i = 0; i < BYTESTREAM_LENGTH; ++i) {
        byteStream[i] = ONE_BYTE;
    }
    return byteStream;
}

std::string DistributedDataStorageTest::GetLocalDeviceId() const
{
    std::string localDeviceId;
    if (!DtbschedmgrDeviceInfoStorage::GetInstance().GetLocalDeviceId(localDeviceId)) {
        DTEST_LOG << "DistributedDataStorageTest::GetLocalDeviceId failed!" << std::endl;
    }
    return localDeviceId;
}

/**
 * @tc.name: InitTest_001
 * @tc.desc: test init DistributedDataStorage
 * @tc.type: FUNC
 * @tc.require: SR000GK5MR
 */
HWTEST_F(DistributedDataStorageTest, InitTest_001, TestSize.Level0)
{
    DTEST_LOG << "DistributedDataStorageTest InitTest_001 start" << std::endl;
    bool ret = distributedDataStorage_->Init();
    EXPECT_EQ(true, ret);
    this_thread::sleep_for(1s);
    distributedDataStorage_->Stop();
    DTEST_LOG << "DistributedDataStorageTest InitTest_001 end" << std::endl;
}

/**
 * @tc.name: InsertTest_001
 * @tc.desc: test insert DistributedDataStorage
 * @tc.type: FUNC
 * @tc.require: AR000GK5MS
 */
HWTEST_F(DistributedDataStorageTest, InsertTest_001, TestSize.Level1)
{
    DTEST_LOG << "DistributedDataStorageTest InsertTest_001 start" << std::endl;
    distributedDataStorage_->Init();
    this_thread::sleep_for(1s);
    std::string deviceId = GetLocalDeviceId();
    uint8_t* byteStream = InitByteStream();
    bool ret = distributedDataStorage_->Insert(deviceId, TASK_ID_1, byteStream, BYTESTREAM_LENGTH);
    EXPECT_EQ(true, ret);
    distributedDataStorage_->Stop();
    DTEST_LOG << "DistributedDataStorageTest InsertTest_001 end" << std::endl;
}

/**
 * @tc.name: DeleteTest_001
 * @tc.desc: test delete DistributedDataStorage
 * @tc.type: FUNC
 * @tc.require: SR000GK66E
 */
HWTEST_F(DistributedDataStorageTest, DeleteTest_001, TestSize.Level0)
{
    DTEST_LOG << "DistributedDataStorageTest DeleteTest_001 start" << std::endl;
    distributedDataStorage_->Init();
    this_thread::sleep_for(1s);
    std::string deviceId = GetLocalDeviceId();
    bool ret = distributedDataStorage_->Delete(deviceId, TASK_ID_1);
    EXPECT_EQ(true, ret);
    distributedDataStorage_->Stop();
    DTEST_LOG << "DistributedDataStorageTest DeleteTest_001 end" << std::endl;
}

/**
 * @tc.name: DeleteTest_002
 * @tc.desc: test delete DistributedDataStorage
 * @tc.type: FUNC
 * @tc.require: AR000GK66F
 */
HWTEST_F(DistributedDataStorageTest, DeleteTest_002, TestSize.Level1)
{
    DTEST_LOG << "DistributedDataStorageTest DeleteTest_002 start" << std::endl;
    distributedDataStorage_->Init();
    this_thread::sleep_for(1s);
    std::string deviceId = GetLocalDeviceId();
    uint8_t* byteStream = InitByteStream();
    distributedDataStorage_->Insert(deviceId, TASK_ID_1, byteStream, BYTESTREAM_LENGTH);
    bool ret = distributedDataStorage_->Delete(deviceId, TASK_ID_1);
    EXPECT_EQ(true, ret);
    distributedDataStorage_->Stop();
    DTEST_LOG << "DistributedDataStorageTest DeleteTest_002 end" << std::endl;
}

/**
 * @tc.name: FuzzyDeleteTest_001
 * @tc.desc: test fuzzy delete DistributedDataStorage
 * @tc.type: FUNC
 * @tc.require: SR000GK67F
 */
HWTEST_F(DistributedDataStorageTest, FuzzyDeleteTest_001, TestSize.Level0)
{
    DTEST_LOG << "DistributedDataStorageTest FuzzyDeleteTest_001 start" << std::endl;
    distributedDataStorage_->Init();
    this_thread::sleep_for(1s);
    std::string deviceId = GetLocalDeviceId();
    bool ret = distributedDataStorage_->FuzzyDelete(deviceId);
    EXPECT_EQ(true, ret);
    distributedDataStorage_->Stop();
    DTEST_LOG << "DistributedDataStorageTest FuzzyDeleteTest_001 end" << std::endl;
}

/**
 * @tc.name: FuzzyDeleteTest_002
 * @tc.desc: test fuzzy delete DistributedDataStorage
 * @tc.type: FUNC
 * @tc.require: AR000GK67G
 */
HWTEST_F(DistributedDataStorageTest, FuzzyDeleteTest_002, TestSize.Level1)
{
    DTEST_LOG << "DistributedDataStorageTest FuzzyDeleteTest_002 start" << std::endl;
    distributedDataStorage_->Init();
    this_thread::sleep_for(1s);
    std::string deviceId = GetLocalDeviceId();
    uint8_t* byteStream = InitByteStream();
    distributedDataStorage_->Insert(deviceId, TASK_ID_1, byteStream, BYTESTREAM_LENGTH);
    distributedDataStorage_->Insert(deviceId, TASK_ID_2, byteStream, BYTESTREAM_LENGTH);
    bool ret = distributedDataStorage_->FuzzyDelete(deviceId);
    EXPECT_EQ(true, ret);
    distributedDataStorage_->Stop();
    DTEST_LOG << "DistributedDataStorageTest FuzzyDeleteTest_002 end" << std::endl;
}

/**
 * @tc.name: QueryTest_001
 * @tc.desc: test query DistributedDataStorage
 * @tc.type: FUNC
 * @tc.require: SR000GK67F
 */
HWTEST_F(DistributedDataStorageTest, QueryTest_001, TestSize.Level0)
{
    DTEST_LOG << "DistributedDataStorageTest QueryTest_001 start" << std::endl;
    distributedDataStorage_->Init();
    this_thread::sleep_for(1s);
    std::string deviceId = GetLocalDeviceId();
    Value value;
    bool ret = distributedDataStorage_->Query(deviceId, TASK_ID_1, value);
    EXPECT_EQ(false, ret);
    distributedDataStorage_->Stop();
    DTEST_LOG << "DistributedDataStorageTest QueryTest_001 end" << std::endl;
}

/**
 * @tc.name: QueryTest_002
 * @tc.desc: test query DistributedDataStorage
 * @tc.type: FUNC
 * @tc.require: AR000GK67G
 */
HWTEST_F(DistributedDataStorageTest, QueryTest_002, TestSize.Level1)
{
    DTEST_LOG << "DistributedDataStorageTest QueryTest_002 start" << std::endl;
    distributedDataStorage_->Init();
    this_thread::sleep_for(1s);
    std::string deviceId = GetLocalDeviceId();
    uint8_t* byteStream = InitByteStream();
    distributedDataStorage_->Insert(deviceId, TASK_ID_1, byteStream, BYTESTREAM_LENGTH);
    Value value;
    bool ret = distributedDataStorage_->Query(deviceId, TASK_ID_1, value);
    EXPECT_EQ(true, ret);
    distributedDataStorage_->Stop();
    DTEST_LOG << "DistributedDataStorageTest QueryTest_002 end" << std::endl;
}

/**
 * @tc.name: QueryTest_003
 * @tc.desc: test query DistributedDataStorage
 * @tc.type: FUNC
 * @tc.require: AR000GK67G
 */
HWTEST_F(DistributedDataStorageTest, QueryTest_003, TestSize.Level1)
{
    DTEST_LOG << "DistributedDataStorageTest QueryTest_003 start" << std::endl;
    distributedDataStorage_->Init();
    this_thread::sleep_for(1s);
    std::string deviceId = GetLocalDeviceId();
    uint8_t* byteStream = InitByteStream();
    distributedDataStorage_->Insert(deviceId, TASK_ID_1, byteStream, BYTESTREAM_LENGTH);
    distributedDataStorage_->Delete(deviceId, TASK_ID_1);
    Value value;
    bool ret = distributedDataStorage_->Query(deviceId, TASK_ID_1, value);
    EXPECT_EQ(false, ret);
    distributedDataStorage_->Stop();
    DTEST_LOG << "DistributedDataStorageTest QueryTest_003 end" << std::endl;
}

/**
 * @tc.name: QueryTest_004
 * @tc.desc: test query DistributedDataStorage
 * @tc.type: FUNC
 * @tc.require: AR000GK67G
 */
HWTEST_F(DistributedDataStorageTest, QueryTest_004, TestSize.Level1)
{
    DTEST_LOG << "DistributedDataStorageTest QueryTest_004 start" << std::endl;
    distributedDataStorage_->Init();
    this_thread::sleep_for(1s);
    std::string deviceId = GetLocalDeviceId();
    uint8_t* byteStream = InitByteStream();
    distributedDataStorage_->Insert(deviceId, TASK_ID_1, byteStream, BYTESTREAM_LENGTH);
    distributedDataStorage_->Insert(deviceId, TASK_ID_2, byteStream, BYTESTREAM_LENGTH);
    distributedDataStorage_->Delete(deviceId, TASK_ID_1);
    Value value;
    bool ret = distributedDataStorage_->Query(deviceId, TASK_ID_1, value);
    EXPECT_EQ(false, ret);
    ret = distributedDataStorage_->Query(deviceId, TASK_ID_2, value);
    EXPECT_EQ(true, ret);
    distributedDataStorage_->Stop();
    DTEST_LOG << "DistributedDataStorageTest QueryTest_004 end" << std::endl;
}

/**
 * @tc.name: QueryTest_005
 * @tc.desc: test query DistributedDataStorage
 * @tc.type: FUNC
 * @tc.require: AR000GK67G
 */
HWTEST_F(DistributedDataStorageTest, QueryTest_005, TestSize.Level1)
{
    DTEST_LOG << "DistributedDataStorageTest QueryTest_005 start" << std::endl;
    distributedDataStorage_->Init();
    this_thread::sleep_for(1s);
    std::string deviceId = GetLocalDeviceId();
    uint8_t* byteStream = InitByteStream();
    distributedDataStorage_->Insert(deviceId, TASK_ID_1, byteStream, BYTESTREAM_LENGTH);
    distributedDataStorage_->Insert(deviceId, TASK_ID_2, byteStream, BYTESTREAM_LENGTH);
    distributedDataStorage_->FuzzyDelete(deviceId);
    Value value;
    bool ret = distributedDataStorage_->Query(deviceId, TASK_ID_1, value);
    EXPECT_EQ(true, ret);
    ret = distributedDataStorage_->Query(deviceId, TASK_ID_2, value);
    EXPECT_EQ(true, ret);
    distributedDataStorage_->Stop();
    DTEST_LOG << "DistributedDataStorageTest QueryTest_005 end" << std::endl;
}
} // namespace DistributedSchedule
} // namespace OHOS