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

#include "distributed_sched_continuation_test.h"

using namespace std;
using namespace testing;
using namespace testing::ext;
using namespace OHOS;
using namespace AAFwk;
using namespace AppExecFwk;

namespace OHOS {
namespace DistributedSchedule {
namespace {
}

void DSchedContinuationTest::SetUpTestCase()
{
}

void DSchedContinuationTest::TearDownTestCase()
{
}

void DSchedContinuationTest::SetUp()
{
    dschedContinuation_ = make_shared<DSchedContinuation>();
}

void DSchedContinuationTest::TearDown()
{
    dschedContinuation_ = nullptr;
}

sptr<IRemoteObject> DSchedContinuationTest::GetDSchedService() const
{
    sptr<IRemoteObject> dsched;
    return dsched;
}

int32_t DSchedContinuationTest::PushAbilityToken()
{
    FuncContinuationCallback continuationCallback = [this] (const sptr<IRemoteObject>& abilityToken) {
        if (abilityToken == nullptr) {
            return;
        }
        timeoutFlag_ = true;
    };
    dschedContinuation_->Init(continuationCallback);
    int32_t sessionId = dschedContinuation_->GenerateSessionId();
    dschedContinuation_->PushAbilityToken(sessionId, GetDSchedService());
    return sessionId;
}

std::shared_ptr<Want> DSchedContinuationTest::MockWant(const string& bundleName, const string& ability, int32_t flags)
{
    ElementName element("", bundleName, ability);
    shared_ptr<Want> spWant = make_shared<Want>();
    spWant->SetElement(element);
    spWant->SetFlags(flags);
    return spWant;
}

std::shared_ptr<AbilityInfo> DSchedContinuationTest::MockAbilityInfo(const string& bundleName, const string& ability,
    const string& devId)
{
    shared_ptr<AbilityInfo> spAbility = make_shared<AbilityInfo>();
    spAbility->bundleName = bundleName;
    spAbility->deviceId = devId;
    return spAbility;
}

int32_t DSchedContinuationTest::StartContinuation(const sptr<IRemoteObject>& abilityToken, int32_t flags)
{
    string bundleName = "bundleName";
    string abilityName = "abilityName";
    string devId = "devId";
    shared_ptr<Want> spWant = MockWant(bundleName, abilityName, flags);
    shared_ptr<AbilityInfo> spAbility = MockAbilityInfo(bundleName, abilityName, devId);
    return DistributedSchedService::GetInstance().StartContinuation(*spWant, *spAbility, abilityToken);
}

/**
 * @tc.name: StartContinuation_001
 * @tc.desc: input invalid params
 * @tc.type: FUNC
 */
HWTEST_F(DSchedContinuationTest, StartContinuation_001, TestSize.Level0)
{
    DTEST_LOG << "DSchedContinuationTest StartContinuation_001 start" << std::endl;
    /**
     * @tc.steps: step1. input invalid abilityToken
     * @tc.expected: step1. return false.
     */
    int32_t ret = StartContinuation(nullptr, Want::FLAG_ABILITY_CONTINUATION);
    EXPECT_TRUE(ret != ERR_OK);
    DTEST_LOG << "DSchedContinuationTest StartContinuation001 end" << std::endl;
}
}
} // namespace OHOS
