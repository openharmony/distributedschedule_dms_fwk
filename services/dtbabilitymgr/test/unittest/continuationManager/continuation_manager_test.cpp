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

#include "continuation_manager_test.h"

#include "distributed_ability_manager_client.h"
#include "dtbschedmgr_log.h"
#include "test_log.h"

namespace OHOS {
namespace DistributedSchedule {
using namespace std;
using namespace testing;
using namespace testing::ext;

void ContinuationManagerTest::SetUpTestCase()
{
    DTEST_LOG << "ContinuationManagerTest::SetUpTestCase" << std::endl;
}

void ContinuationManagerTest::TearDownTestCase()
{
    DTEST_LOG << "ContinuationManagerTest::TearDownTestCase" << std::endl;
}

void ContinuationManagerTest::SetUp()
{
    DTEST_LOG << "ContinuationManagerTest::SetUp" << std::endl;
}

void ContinuationManagerTest::TearDown()
{
    DTEST_LOG << "ContinuationManagerTest::TearDown" << std::endl;
}

/**
 * @tc.name: RegisterTest_001
 * @tc.desc: test register token
 * @tc.type: FUNC
 * @tc.require: SR000H34KJ
 */
HWTEST_F(ContinuationManagerTest, RegisterTest_001, TestSize.Level0)
{
    DTEST_LOG << "ContinuationManagerTest RegisterTest_001 start" << std::endl;
    int32_t token1 = -1;
    int32_t result1 = DistributedAbilityManagerClient::GetInstance().Register(nullptr, token1);
    DTEST_LOG << "result1:" << result1 << std::endl;
    int32_t token2 = -1;
    int32_t result2 = DistributedAbilityManagerClient::GetInstance().Register(nullptr, token2);
    DTEST_LOG << "result2:" << result2 << std::endl;
    EXPECT_EQ(ERR_OK, result1);
    EXPECT_EQ(ERR_OK, result2);
    EXPECT_EQ(1, token2 - token1);
    DTEST_LOG << "ContinuationManagerTest RegisterTest_001 end" << std::endl;
}
} // namespace DistributedSchedule
} // namespace OHOS