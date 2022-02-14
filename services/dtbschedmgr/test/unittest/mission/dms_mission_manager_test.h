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

#ifndef SERVICES_DTBSCHEDMGR_TEST_UNITTEST_MISSION_DMS_MISSION_MANAGER_TEST_H
#define SERVICES_DTBSCHEDMGR_TEST_UNITTEST_MISSION_DMS_MISSION_MANAGER_TEST_H

#include "gtest/gtest.h"

#include "distributed_sched_interface.h"
#include "remote_mission_listener_stub.h"

namespace OHOS {
namespace DistributedSchedule {
class DMSMissionManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
protected:
    std::string localDeviceId_;
    std::u16string u16localDeviceId_;
    sptr<IDistributedSched> proxy_;
    sptr<IDistributedSched> GetDms();
};

class RemoteMissionListenerTest : public AAFwk::RemoteMissionListenerStub {
public:
    void NotifyMissionsChanged(const std::string& deviceId) override;
    void NotifySnapshot(const std::string& deviceId, int32_t missionId) override;
    void NotifyNetDisconnect(const std::string& deviceId, int32_t state) override;
};
} // namespace DistributedSchedule
} // namespace OHOS
#endif /* SERVICES_DTBSCHEDMGR_TEST_UNITTEST_MISSION_DMS_MISSION_MANAGER_TEST_H */
