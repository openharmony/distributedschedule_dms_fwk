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

#include "distributed_sched_proxy.h"
#include "distributed_sched_service.h"

#include "gtest/gtest.h"
#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "test_log.h"

namespace OHOS {
namespace DistributedSchedule {
using namespace testing;
using namespace testing::ext;

namespace {
constexpr int32_t STDOUT_FD = 1;
}

class DistributedSchedConnectTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void DistributedSchedConnectTest::SetUpTestCase()
{
}

void DistributedSchedConnectTest::TearDownTestCase()
{
}

void DistributedSchedConnectTest::SetUp()
{
}

void DistributedSchedConnectTest::TearDown()
{
}

/**
 * @tc.name: DumpConnectInfo_001
 * @tc.desc: dump connect ability info
 * @tc.type: FUNC
 */
HWTEST_F(DistributedSchedConnectTest, DumpConnectInfo_001, TestSize.Level1)
{
    sptr<ISystemAbilityManager> samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        DTEST_LOG << "DistributedSchedServiceTest DumpConnectInfo_001 samgr null" << std::endl;
    } else {
        DTEST_LOG << "DistributedSchedServiceTest DumpConnectInfo_001 avaiable" << std::endl;
    }

    auto dms = samgr->GetSystemAbility(DISTRIBUTED_SCHED_SA_ID);
    std::vector<std::u16string> args;
    args.push_back(u"-connect");
    int32_t result = dms->Dump(STDOUT_FD, args);
    DTEST_LOG << "DistributedSchedServiceTest DumpConnectInfo_001 dump result: " << result << std::endl;
}
}
}