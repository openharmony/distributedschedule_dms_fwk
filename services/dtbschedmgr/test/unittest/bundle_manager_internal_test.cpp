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

#include "bundle_manager_internal_test.h"
#include "bundle/bundle_manager_internal.h"

#define private public
#include "bundle/bundle_manager_callback_stub.h"
#undef private

#include "dtbschedmgr_log.h"
#include "test_log.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace DistributedSchedule {
namespace {
const string GROUP_ID = "TEST_GROUP_ID";
}

void BundleManagerInternalTest::SetUpTestCase()
{
    DTEST_LOG << "BundleManagerInternalTest::SetUpTestCase" << std::endl;
}

void BundleManagerInternalTest::TearDownTestCase()
{
    DTEST_LOG << "BundleManagerInternalTest::TearDownTestCase" << std::endl;
}

void BundleManagerInternalTest::TearDown()
{
    DTEST_LOG << "BundleManagerInternalTest::TearDown" << std::endl;
}

void BundleManagerInternalTest::SetUp()
{
    DTEST_LOG << "BundleManagerInternalTest::SetUp" << std::endl;
}

/**
 * @tc.name: BundleManagerInternalTest_001
 * @tc.desc: input invalid params
 * @tc.type: FUNC
 * @tc.require: AR000H3FD0
 */
HWTEST_F(BundleManagerInternalTest, BundleManagerInternalTest_001, TestSize.Level0)
{
    DTEST_LOG << "BundleManagerInternalTest BundleManagerInternalTest_001 begin" << std::endl;
    AAFwk::Want want;
    AppExecFwk::ExtensionAbilityInfo extensionInfo;

    bool ret = BundleManagerInternal::QueryExtensionAbilityInfo(want, extensionInfo);
    EXPECT_TRUE(!ret);
    EXPECT_TRUE(extensionInfo.name.empty());
    DTEST_LOG << "BundleManagerInternalTest BundleManagerInternalTest_001 end ret:" << ret << std::endl;
}

/**
 * @tc.name: BundleManagerInternalTest_002
 * @tc.desc: test ability info convert
 * @tc.type: FUNC
 * @tc.require: AR000H3FD0
 */
HWTEST_F(BundleManagerInternalTest, BundleManagerInternalTest_002, TestSize.Level0)
{
    DTEST_LOG << "BundleManagerInternalTest BundleManagerInternalTest_002 begin" << std::endl;
    AppExecFwk::AbilityInfo abilityInfo;
    AppExecFwk::ExtensionAbilityInfo extensionInfo;
    extensionInfo.bundleName.append("bundleName");
    extensionInfo.name.append("name");
    vector<string> permissions;
    extensionInfo.permissions = permissions;
    extensionInfo.visible = true;
    
    BundleManagerInternal::InitAbilityInfoFromExtension(extensionInfo, abilityInfo);
    EXPECT_TRUE(abilityInfo.bundleName.compare("bundleName") == 0);
    EXPECT_TRUE(abilityInfo.name.compare("name") == 0);
    EXPECT_TRUE(abilityInfo.permissions == permissions);
    EXPECT_TRUE(abilityInfo.visible);
    DTEST_LOG << "BundleManagerInternalTest BundleManagerInternalTest_002 end "<< std::endl;
}

/**
 * @tc.name: BundleManagerInternalTest_003
 * @tc.desc: test CheckRemoteBundleInfo with empty bundleName
 * @tc.type: FUNC
 */
HWTEST_F(BundleManagerInternalTest, BundleManagerInternalTest_003, TestSize.Level1)
{
    DTEST_LOG << "BundleManagerInternalTest BundleManagerInternalTest_003 begin" << std::endl;
    string deviceId = "123456";
    string bundleName = "";
    AppExecFwk::DistributedBundleInfo remoteBundleInfo;
    int ret = BundleManagerInternal::CheckRemoteBundleInfoForContinuation(deviceId, bundleName, remoteBundleInfo);
    EXPECT_TRUE(INVALID_PARAMETERS_ERR == ret);

    DTEST_LOG << "BundleManagerInternalTest BundleManagerInternalTest_003 end "<< std::endl;
}

/**
 * @tc.name: BundleManagerInternalTest_004
 * @tc.desc: test CheckRemoteBundleInfo with unexist bundle
 * @tc.type: FUNC
 */
HWTEST_F(BundleManagerInternalTest, BundleManagerInternalTest_004, TestSize.Level1)
{
    DTEST_LOG << "BundleManagerInternalTest BundleManagerInternalTest_004 begin" << std::endl;
    string deviceId = "123456";
    string bundleName = "ohos.samples.testApp";
    AppExecFwk::DistributedBundleInfo remoteBundleInfo;
    int ret = BundleManagerInternal::CheckRemoteBundleInfoForContinuation(deviceId, bundleName, remoteBundleInfo);
    EXPECT_TRUE(INVALID_PARAMETERS_ERR == ret);

    DTEST_LOG << "BundleManagerInternalTest BundleManagerInternalTest_004 end "<< std::endl;
}

/**
 * @tc.name: BundleManagerInternalTest_005
 * @tc.desc: test CheckRemoteBundleInfo with valid param
 * @tc.type: FUNC
 */
HWTEST_F(BundleManagerInternalTest, BundleManagerInternalTest_005, TestSize.Level1)
{
    DTEST_LOG << "BundleManagerInternalTest BundleManagerInternalTest_005 begin" << std::endl;
    string deviceId = "123456";
    string bundleName = "ohos.samples.distributedcalc";
    AppExecFwk::DistributedBundleInfo remoteBundleInfo;
    int ret = BundleManagerInternal::CheckRemoteBundleInfoForContinuation(deviceId, bundleName, remoteBundleInfo);
    EXPECT_TRUE(CONTINUE_REMOTE_UNINSTALLED_UNSUPPORT_FREEINSTALL == ret);

    DTEST_LOG << "BundleManagerInternalTest BundleManagerInternalTest_005 end "<< std::endl;
}

/**
 * @tc.name: BundleManagerInternalTest_006
 * @tc.desc: test CheckIfRemoteCanInstall with unexist device
 * @tc.type: FUNC
 */
HWTEST_F(BundleManagerInternalTest, BundleManagerInternalTest_006, TestSize.Level1)
{
    DTEST_LOG << "BundleManagerInternalTest BundleManagerInternalTest_006 begin" << std::endl;
    string deviceId = "123456";
    string bundleName = "ohos.samples.distributedcalc";
    string moduleName = "entry";
    string abilityName = "MainAbility";
    AAFwk::Want want;
    want.SetElementName(deviceId, bundleName, abilityName, moduleName);
    int32_t missionId = 0;
    bool ret = BundleManagerInternal::CheckIfRemoteCanInstall(want, missionId);
    EXPECT_TRUE(false == ret);
    DTEST_LOG << "BundleManagerInternalTest BundleManagerInternalTest_006 end "<< std::endl;
}

/**
 * @tc.name: BundleManagerInternalTest_007
 * @tc.desc: test CheckIfRemoteCanInstall with unexist bundle
 * @tc.type: FUNC
 */
HWTEST_F(BundleManagerInternalTest, BundleManagerInternalTest_007, TestSize.Level1)
{
    DTEST_LOG << "BundleManagerInternalTest BundleManagerInternalTest_007 begin" << std::endl;
    string deviceId = "123456";
    string bundleName = "ohos.samples.testApp";
    string moduleName = "entry";
    string abilityName = "MainAbility";
    AAFwk::Want want;
    want.SetElementName(deviceId, bundleName, abilityName, moduleName);
    int32_t missionId = 0;
    bool ret = BundleManagerInternal::CheckIfRemoteCanInstall(want, missionId);
    EXPECT_TRUE(false == ret);
    DTEST_LOG << "BundleManagerInternalTest BundleManagerInternalTest_007 end "<< std::endl;
}

/**
 * @tc.name: BundleManagerInternalTest_008
 * @tc.desc: test CheckIfRemoteCanInstall with invalid param
 * @tc.type: FUNC
 */
HWTEST_F(BundleManagerInternalTest, BundleManagerInternalTest_008, TestSize.Level1)
{
    DTEST_LOG << "BundleManagerInternalTest BundleManagerInternalTest_008 begin" << std::endl;
    string deviceId = "";
    string bundleName = "ohos.samples.testApp";
    string moduleName = "entry";
    string abilityName = "MainAbility";
    AAFwk::Want want;
    want.SetElementName(deviceId, bundleName, abilityName, moduleName);
    int32_t missionId = 0;
    bool ret = BundleManagerInternal::CheckIfRemoteCanInstall(want, missionId);
    EXPECT_TRUE(false == ret);

    deviceId = "123456";
    bundleName = "";
    want.SetElementName(deviceId, bundleName, abilityName, moduleName);
    ret = BundleManagerInternal::CheckIfRemoteCanInstall(want, missionId);
    EXPECT_TRUE(false == ret);

    bundleName = "ohos.samples.testApp";
    moduleName = "";
    want.SetElementName(deviceId, bundleName, abilityName, moduleName);
    ret = BundleManagerInternal::CheckIfRemoteCanInstall(want, missionId);
    EXPECT_TRUE(false == ret);

    moduleName = "entry";
    abilityName = "";
    want.SetElementName(deviceId, bundleName, abilityName, moduleName);
    ret = BundleManagerInternal::CheckIfRemoteCanInstall(want, missionId);
    EXPECT_TRUE(false == ret);

    DTEST_LOG << "BundleManagerInternalTest BundleManagerInternalTest_008 end "<< std::endl;
}

/**
 * @tc.name: BundleManagerCallBackTest_001
 * @tc.desc: test OnQueryInstallationFinished with failed result
 * @tc.type: FUNC
 */
HWTEST_F(BundleManagerInternalTest, BundleManagerCallBackTest_001, TestSize.Level1)
{
    DTEST_LOG << "BundleManagerCallBackTest BundleManagerCallBackTest_001 begin" << std::endl;
    int32_t resultCode = -1;
    string deviceId = "123456";
    int32_t missionId = 0;
    int32_t versionCode = 10000;
    auto callback = new DmsBundleManagerCallbackStub();
    int32_t ret = callback->OnQueryInstallationFinished(resultCode, deviceId, missionId, versionCode);
    EXPECT_TRUE(CONTINUE_REMOTE_UNINSTALLED_CANNOT_FREEINSTALL == ret);
    DTEST_LOG << "BundleManagerCallBackTest BundleManagerCallBackTest_001 end "<< std::endl;
}

/**
 * @tc.name: BundleManagerCallBackTest_002
 * @tc.desc: test OnQueryInstallationFinished
 * @tc.type: FUNC
 */
HWTEST_F(BundleManagerInternalTest, BundleManagerCallBackTest_002, TestSize.Level1)
{
    DTEST_LOG << "BundleManagerCallBackTest BundleManagerCallBackTest_002 begin" << std::endl;
    int32_t resultCode = 0;
    string deviceId = "123456";
    int32_t missionId = 0;
    int32_t versionCode = 10000;
    auto callback = new DmsBundleManagerCallbackStub();
    int32_t ret = callback->OnQueryInstallationFinished(resultCode, deviceId, missionId, versionCode);
    EXPECT_TRUE(ERR_OK != ret);
    DTEST_LOG << "BundleManagerCallBackTest BundleManagerCallBackTest_002 end "<< std::endl;
}
}
}