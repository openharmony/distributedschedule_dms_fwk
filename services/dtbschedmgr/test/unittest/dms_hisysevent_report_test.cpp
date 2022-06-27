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

#include "dms_hisysevent_report_test.h"

#include "dfx/dms_hisysevent_report.h"
#include "test_log.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace DistributedSchedule {
namespace {
    const string GROUP_ID = "TEST_GROUP_ID";
    const string bundleName = "ohos.hisysevent.test";
    const string abilityName = "testAbility";
    const int32_t eventResult = 0;
    const int32_t callingAppUid = 0;
}

void DmsHiSysEventReportTest::SetUpTestCase()
{
    DTEST_LOG << "DmsHiSysEventReportTest::SetUpTestCase" << std::endl;
}

void DmsHiSysEventReportTest::TearDownTestCase()
{
    DTEST_LOG << "DmsHiSysEventReportTest::TearDownTestCase" << std::endl;
}

void DmsHiSysEventReportTest::TearDown()
{
    DTEST_LOG << "DmsHiSysEventReportTest::TearDown" << std::endl;
}

void DmsHiSysEventReportTest::SetUp()
{
    DTEST_LOG << "DmsHiSysEventReportTest::SetUp" << std::endl;
}

/**
 * @tc.name: DmsHiSysEventReportTest_001
 * @tc.desc: report start remote ability
 * @tc.type: FUNC
 */
HWTEST_F(DmsHiSysEventReportTest, DmsHiSysEventReportTest_001, TestSize.Level1)
{
    DTEST_LOG << "DmsHiSysEventReportTest DmsHiSysEventReportTest_001 begin" << std::endl;

    int32_t result = -1;
    BehaviorEventParam param = { EventCallingType::LOCAL, BehaviorEvent::START_REMOTE_ABILITY, eventResult,
        bundleName, abilityName, callingAppUid };
    result = DmsHiSysEventReport::ReportBehaviorEvent(param);
    EXPECT_TRUE(result == 0);
    DTEST_LOG << "DmsHiSysEventReportTest DmsHiSysEventReportTest_001 end ret:" << result << std::endl;
}

/**
 * @tc.name: DmsHiSysEventReportTest_002
 * @tc.desc: report connect remote ability
 * @tc.type: FUNC
 */
HWTEST_F(DmsHiSysEventReportTest, DmsHiSysEventReportTest_002, TestSize.Level1)
{
    DTEST_LOG << "DmsHiSysEventReportTest DmsHiSysEventReportTest_002 begin" << std::endl;

    int32_t result = -1;
    BehaviorEventParam param = { EventCallingType::LOCAL, BehaviorEvent::CONNECT_REMOTE_ABILITY, eventResult,
        bundleName, abilityName, callingAppUid };
    result = DmsHiSysEventReport::ReportBehaviorEvent(param);
    EXPECT_TRUE(result == 0);
    DTEST_LOG << "DmsHiSysEventReportTest DmsHiSysEventReportTest_002 end ret:" << result << std::endl;
}

/**
 * @tc.name: DmsHiSysEventReportTest_003
 * @tc.desc: report continue remote ability
 * @tc.type: FUNC
 */
HWTEST_F(DmsHiSysEventReportTest, DmsHiSysEventReportTest_003, TestSize.Level1)
{
    DTEST_LOG << "DmsHiSysEventReportTest DmsHiSysEventReportTest_003 begin" << std::endl;

    int32_t result = -1;
    BehaviorEventParam param = { EventCallingType::LOCAL, BehaviorEvent::START_CONTINUATION, eventResult,
        bundleName, abilityName, callingAppUid };
    result = DmsHiSysEventReport::ReportBehaviorEvent(param);
    EXPECT_TRUE(result == 0);
    DTEST_LOG << "DmsHiSysEventReportTest DmsHiSysEventReportTest_003 end ret:" << result << std::endl;
}

/**
 * @tc.name: DmsHiSysEventReportTest_004
 * @tc.desc: report start remote ability by call
 * @tc.type: FUNC
 */
HWTEST_F(DmsHiSysEventReportTest, DmsHiSysEventReportTest_004, TestSize.Level1)
{
    DTEST_LOG << "DmsHiSysEventReportTest DmsHiSysEventReportTest_004 begin" << std::endl;

    int32_t result = -1;
    BehaviorEventParam param = { EventCallingType::LOCAL, BehaviorEvent::START_REMOTE_ABILITY_BYCALL, eventResult,
        bundleName, abilityName, callingAppUid };
    result = DmsHiSysEventReport::ReportBehaviorEvent(param);
    EXPECT_TRUE(result == 0);
    DTEST_LOG << "DmsHiSysEventReportTest DmsHiSysEventReportTest_004 end "<< std::endl;
}

/**
 * @tc.name: DmsHiSysEventReportTest_005
 * @tc.desc: report disconnect remote ability
 * @tc.type: FUNC
 */
HWTEST_F(DmsHiSysEventReportTest, DmsHiSysEventReportTest_005, TestSize.Level1)
{
    DTEST_LOG << "DmsHiSysEventReportTest DmsHiSysEventReportTest_005 begin" << std::endl;

    int32_t result = -1;
    BehaviorEventParam param = { EventCallingType::LOCAL, BehaviorEvent::DISCONNECT_REMOTE_ABILITY, eventResult,
        bundleName, abilityName, callingAppUid };
    result = DmsHiSysEventReport::ReportBehaviorEvent(param);
    EXPECT_TRUE(result == 0);
    DTEST_LOG << "DmsHiSysEventReportTest DmsHiSysEventReportTest_005 end "<< std::endl;
}

/**
 * @tc.name: DmsHiSysEventReportTest_006
 * @tc.desc: report release remote ability
 * @tc.type: FUNC
 */
HWTEST_F(DmsHiSysEventReportTest, DmsHiSysEventReportTest_006, TestSize.Level1)
{
    DTEST_LOG << "DmsHiSysEventReportTest DmsHiSysEventReportTest_006 begin" << std::endl;

    int32_t result = -1;
    BehaviorEventParam param = { EventCallingType::LOCAL, BehaviorEvent::RELEASE_REMOTE_ABILITY, eventResult,
        bundleName, abilityName, callingAppUid };
    result = DmsHiSysEventReport::ReportBehaviorEvent(param);
    EXPECT_TRUE(result == 0);
    DTEST_LOG << "DmsHiSysEventReportTest DmsHiSysEventReportTest_006 end "<< std::endl;
}

/**
 * @tc.name: DmsHiSysEventReportTest_007
 * @tc.desc: report start remote ability
 * @tc.type: FUNC
 */
HWTEST_F(DmsHiSysEventReportTest, DmsHiSysEventReportTest_007, TestSize.Level1)
{
    DTEST_LOG << "DmsHiSysEventReportTest DmsHiSysEventReportTest_007 begin" << std::endl;

    int32_t result = -1;
    result = DmsHiSysEventReport::ReportFaultEvent(FaultEvent::START_REMOTE_ABILITY,
        EventErrorType::GET_ABILITY_MGR_FAILED);
    EXPECT_TRUE(result == 0);
    DTEST_LOG << "DmsHiSysEventReportTest DmsHiSysEventReportTest_007 end ret:" << result << std::endl;
}

/**
 * @tc.name: DmsHiSysEventReportTest_008
 * @tc.desc: report connect remote ability
 * @tc.type: FUNC
 */
HWTEST_F(DmsHiSysEventReportTest, DmsHiSysEventReportTest_008, TestSize.Level1)
{
    DTEST_LOG << "DmsHiSysEventReportTest DmsHiSysEventReportTest_008 begin" << std::endl;

    int32_t result = -1;
    result = DmsHiSysEventReport::ReportFaultEvent(FaultEvent::CONNECT_REMOTE_ABILITY,
        EventErrorType::GET_ABILITY_MGR_FAILED);
    EXPECT_TRUE(result == 0);
    DTEST_LOG << "DmsHiSysEventReportTest DmsHiSysEventReportTest_008 end ret:" << result << std::endl;
}

/**
 * @tc.name: DmsHiSysEventReportTest_009
 * @tc.desc: report continue remote ability
 * @tc.type: FUNC
 */
HWTEST_F(DmsHiSysEventReportTest, DmsHiSysEventReportTest_009, TestSize.Level1)
{
    DTEST_LOG << "DmsHiSysEventReportTest DmsHiSysEventReportTest_009 begin" << std::endl;

    int32_t result = -1;
    result = DmsHiSysEventReport::ReportFaultEvent(FaultEvent::START_CONTINUATION,
        EventErrorType::GET_ABILITY_MGR_FAILED);
    EXPECT_TRUE(result == 0);
    DTEST_LOG << "DmsHiSysEventReportTest DmsHiSysEventReportTest_009 end ret:" << result << std::endl;
}

/**
 * @tc.name: HiSysEventReportTest_010
 * @tc.desc: report start remote ability by call
 * @tc.type: FUNC
 */
HWTEST_F(DmsHiSysEventReportTest, DmsHiSysEventReportTest_010, TestSize.Level1)
{
    DTEST_LOG << "DmsHiSysEventReportTest DmsHiSysEventReportTest_010 begin" << std::endl;

    int32_t result = -1;
    result = DmsHiSysEventReport::ReportFaultEvent(FaultEvent::START_REMOTE_ABILITY_BYCALL,
        EventErrorType::GET_ABILITY_MGR_FAILED);
    EXPECT_TRUE(result == 0);
    DTEST_LOG << "DmsHiSysEventReportTest DmsHiSysEventReportTest_010 end "<< std::endl;
}

/**
 * @tc.name: DmsHiSysEventReportTest_011
 * @tc.desc: report disconnect remote ability
 * @tc.type: FUNC
 */
HWTEST_F(DmsHiSysEventReportTest, DmsHiSysEventReportTest_011, TestSize.Level1)
{
    DTEST_LOG << "DmsHiSysEventReportTest DmsHiSysEventReportTest_011 begin" << std::endl;

    int32_t result = -1;
    result = DmsHiSysEventReport::ReportFaultEvent(FaultEvent::DISCONNECT_REMOTE_ABILITY,
        EventErrorType::GET_ABILITY_MGR_FAILED);
    EXPECT_TRUE(result == 0);
    DTEST_LOG << "DmsHiSysEventReportTest DmsHiSysEventReportTest_011 end "<< std::endl;
}

/**
 * @tc.name: DmsHiSysEventReportTest_012
 * @tc.desc: report release remote ability
 * @tc.type: FUNC
 */
HWTEST_F(DmsHiSysEventReportTest, DmsHiSysEventReportTest_012, TestSize.Level1)
{
    DTEST_LOG << "DmsHiSysEventReportTest DmsHiSysEventReportTest_012 begin" << std::endl;

    int32_t result = -1;
    result = DmsHiSysEventReport::ReportFaultEvent(FaultEvent::RELEASE_REMOTE_ABILITY,
        EventErrorType::GET_ABILITY_MGR_FAILED);
    EXPECT_TRUE(result == 0);
    DTEST_LOG << "DmsHiSysEventReportTest DmsHiSysEventReportTest_012 end "<< std::endl;
}
}
}