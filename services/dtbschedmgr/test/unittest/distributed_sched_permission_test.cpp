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

#include "distributed_sched_permission_test.h"

#include "distributed_sched_interface.h"
#include "distributed_sched_permission.h"
#include "dtbschedmgr_log.h"
#include "test_log.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace DistributedSchedule {
namespace {
const string GROUP_ID = "TEST_GROUP_ID";
}

void DistributedSchedPermissionTest::SetUpTestCase()
{
    DTEST_LOG << "DistributedSchedPermissionTest::SetUpTestCase" << std::endl;
}

void DistributedSchedPermissionTest::TearDownTestCase()
{
    DTEST_LOG << "DistributedSchedPermissionTest::TearDownTestCase" << std::endl;
}

void DistributedSchedPermissionTest::TearDown()
{
    DTEST_LOG << "DistributedSchedPermissionTest::TearDown" << std::endl;
}

void DistributedSchedPermissionTest::SetUp()
{
    DTEST_LOG << "DistributedSchedPermissionTest::SetUp" << std::endl;
}

/**
 * @tc.name: CheckDPermission_001
 * @tc.desc: input invalid params
 * @tc.type: FUNC
 */
HWTEST_F(DistributedSchedPermissionTest, CheckDPermission_001, TestSize.Level0)
{
    DTEST_LOG << "DistributedSchedPermissionTest CheckDPermission_001 begin" << std::endl;
    AAFwk::Want want;
    CallerInfo callerInfo;
    IDistributedSched::AccountInfo accountInfo;
    accountInfo.accountType = IDistributedSched::DIFF_ACCOUNT_TYPE;
    std::string localDeviceId = "255.255.255.255";
    int32_t ret = DistributedSchedPermission::GetInstance().CheckDPermission(want,
        callerInfo, accountInfo, localDeviceId);
    EXPECT_TRUE(ret != ERR_OK);
    DTEST_LOG << "DistributedSchedPermissionTest CheckDPermission_001 end ret:" << ret << std::endl;
}

/**
 * @tc.name: CheckDPermission_002
 * @tc.desc: input invalid params
 * @tc.type: FUNC
 */
HWTEST_F(DistributedSchedPermissionTest, CheckDPermission_002, TestSize.Level0)
{
    DTEST_LOG << "DistributedSchedPermissionTest CheckDPermission_002 begin" << std::endl;
    AAFwk::Want want;
    CallerInfo callerInfo;
    IDistributedSched::AccountInfo accountInfo;
    accountInfo.accountType = IDistributedSched::DIFF_ACCOUNT_TYPE;
    bool needQueryExtension = true;
    std::string localDeviceId = "255.255.255.255";
    int32_t ret = DistributedSchedPermission::GetInstance().CheckDPermission(want,
        callerInfo, accountInfo, localDeviceId, needQueryExtension);
    EXPECT_TRUE(ret != ERR_OK);
    DTEST_LOG << "DistributedSchedPermissionTest CheckDPermission_002 end ret:" << ret << std::endl;
}

/**
 * @tc.name: CheckDPermission_003
 * @tc.desc: input invalid params
 * @tc.type: FUNC
 */
HWTEST_F(DistributedSchedPermissionTest, CheckDPermission_003, TestSize.Level0)
{
    DTEST_LOG << "DistributedSchedPermissionTest CheckDPermission_003 begin" << std::endl;
    AAFwk::Want want;
    CallerInfo callerInfo;
    IDistributedSched::AccountInfo accountInfo;
    accountInfo.accountType = IDistributedSched::DIFF_ACCOUNT_TYPE;
    std::string localDeviceId = "";
    int32_t ret = DistributedSchedPermission::GetInstance().CheckDPermission(want,
        callerInfo, accountInfo, localDeviceId);
    EXPECT_TRUE(ret == INVALID_PARAMETERS_ERR);
    DTEST_LOG << "DistributedSchedPermissionTest CheckDPermission_003 end ret:" << ret << std::endl;
}

/**
 * @tc.name: CheckDPermission_004
 * @tc.desc: input invalid params
 * @tc.type: FUNC
 */
HWTEST_F(DistributedSchedPermissionTest, CheckDPermission_004, TestSize.Level0)
{
    DTEST_LOG << "DistributedSchedPermissionTest CheckDPermission_004 begin" << std::endl;
    AAFwk::Want want;
    CallerInfo callerInfo;
    IDistributedSched::AccountInfo accountInfo;
    accountInfo.accountType = IDistributedSched::DIFF_ACCOUNT_TYPE;
    bool needQueryExtension = true;
    std::string localDeviceId = "";
    int32_t ret = DistributedSchedPermission::GetInstance().CheckDPermission(want,
        callerInfo, accountInfo, localDeviceId, needQueryExtension);
    EXPECT_TRUE(ret == INVALID_PARAMETERS_ERR);
    DTEST_LOG << "DistributedSchedPermissionTest CheckDPermission_004 end ret:" << ret << std::endl;
}

/**
 * @tc.name: CheckDPermission_005
 * @tc.desc: call CheckDPermission with illegal parameter
 * @tc.type: FUNC
 * @tc.require: SR000GK79C
 */
HWTEST_F(DistributedSchedPermissionTest, CheckDPermission_005, TestSize.Level0)
{
    DTEST_LOG << "DistributedSchedPermissionTest CheckDPermission_005 begin" << std::endl;
    AAFwk::Want want;
    CallerInfo callerInfo;
    callerInfo.accessToken = 0;
    IDistributedSched::AccountInfo accountInfo;
    std::string localDeviceId = "255.255.255.255";
    int32_t ret = DistributedSchedPermission::GetInstance().CheckDPermission(want,
        callerInfo, accountInfo, localDeviceId);
    EXPECT_TRUE(ret != ERR_OK);
    DTEST_LOG << "DistributedSchedPermissionTest CheckDPermission_005 end result:" << ret << std::endl;
}

/**
 * @tc.name: CheckDPermission_006
 * @tc.desc: call CheckDPermission with different account type
 * @tc.type: FUNC
 * @tc.require: AR000GK79E
 */
HWTEST_F(DistributedSchedPermissionTest, CheckDPermission_006, TestSize.Level1)
{
    DTEST_LOG << "DistributedSchedPermissionTest CheckDPermission_006 begin" << std::endl;
    AAFwk::Want want;
    CallerInfo callerInfo;
    callerInfo.accessToken = 0;
    IDistributedSched::AccountInfo accountInfo;
    accountInfo.accountType = IDistributedSched::DIFF_ACCOUNT_TYPE;
    string localDeviceId = "255.255.255.255";
    int32_t ret = DistributedSchedPermission::GetInstance().CheckDPermission(want,
        callerInfo, accountInfo, localDeviceId);
    EXPECT_TRUE(ret != ERR_OK);
    DTEST_LOG << "DistributedSchedPermissionTest CheckDPermission_006 end result:" << ret << std::endl;
}

/**
 * @tc.name: CheckDPermission_007
 * @tc.desc: call CheckDPermission with illegal ability info
 * @tc.type: FUNC
 * @tc.require: AR000GK79E
 */
HWTEST_F(DistributedSchedPermissionTest, CheckDPermission_007, TestSize.Level1)
{
    DTEST_LOG << "DistributedSchedPermissionTest CheckDPermission_007 begin" << std::endl;
    AAFwk::Want want;
    CallerInfo callerInfo;
    callerInfo.accessToken = 0;
    IDistributedSched::AccountInfo accountInfo;
    string localDeviceId = "255.255.255.255";
    int32_t ret = DistributedSchedPermission::GetInstance().CheckDPermission(want,
        callerInfo, accountInfo, localDeviceId);
    EXPECT_TRUE(ret != ERR_OK);
    DTEST_LOG << "DistributedSchedPermissionTest CheckDPermission_007 end result:" << ret << std::endl;
}

/**
 * @tc.name: CheckDPermission_008
 * @tc.desc: call CheckDPermission with same account type
 * @tc.type: FUNC
 * @tc.require: SR000H1FJV
 */
HWTEST_F(DistributedSchedPermissionTest, CheckDPermission_008, TestSize.Level0)
{
    DTEST_LOG << "DistributedSchedPermissionTest CheckDPermission_008 begin" << std::endl;
    AAFwk::Want want;
    CallerInfo callerInfo;
    callerInfo.accessToken = 0;
    IDistributedSched::AccountInfo accountInfo;
    accountInfo.accountType = IDistributedSched::SAME_ACCOUNT_TYPE;
    string localDeviceId = "255.255.255.255";
    int32_t ret = DistributedSchedPermission::GetInstance().CheckDPermission(want,
        callerInfo, accountInfo, localDeviceId);
    EXPECT_TRUE(ret != ERR_OK);
    DTEST_LOG << "DistributedSchedPermissionTest CheckDPermission_008 end result:" << ret << std::endl;
}

/**
 * @tc.name: CheckDPermission_009
 * @tc.desc: call CheckDPermission with illegal account info
 * @tc.type: FUNC
 * @tc.require: AR000H1RID
 */
HWTEST_F(DistributedSchedPermissionTest, CheckDPermission_009, TestSize.Level1)
{
    DTEST_LOG << "DistributedSchedPermissionTest CheckDPermission_009 begin" << std::endl;
    AAFwk::Want want;
    CallerInfo callerInfo;
    callerInfo.accessToken = 0;
    IDistributedSched::AccountInfo accountInfo;
    accountInfo.accountType = IDistributedSched::DIFF_ACCOUNT_TYPE;
    std::string groupId = GROUP_ID;
    accountInfo.groupIdList.push_back(groupId);
    string localDeviceId = "255.255.255.255";
    int32_t ret = DistributedSchedPermission::GetInstance().CheckDPermission(want,
        callerInfo, accountInfo, localDeviceId);
    EXPECT_TRUE(ret != ERR_OK);
    DTEST_LOG << "DistributedSchedPermissionTest CheckDPermission_009 end result:" << ret << std::endl;
}
} // namespace DistributedSchedule
} // namespace OHOS