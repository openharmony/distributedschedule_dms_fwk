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
}
}