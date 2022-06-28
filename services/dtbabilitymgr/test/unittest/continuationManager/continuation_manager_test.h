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

#ifndef OHOS_DISTRIBUTED_SCHED_CONTINUATION_MANAGER_TEST_H
#define OHOS_DISTRIBUTED_SCHED_CONTINUATION_MANAGER_TEST_H

#include "gtest/gtest.h"

#include "device_selection_notifier_stub.h"
#include "distributed_sched_proxy.h"

namespace OHOS {
namespace DistributedSchedule {
class ContinuationManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    sptr<IDistributedSched> GetDms();
    void InitContinuationHandler();
private:
    sptr<IDistributedSched> proxy_;
};

class DeviceSelectionNotifierTest : public DeviceSelectionNotifierStub {
public:
    DeviceSelectionNotifierTest() = default;
    ~DeviceSelectionNotifierTest() = default;

    void OnDeviceConnect(const std::vector<ContinuationResult>& continuationResults) override;
    void OnDeviceDisconnect(const std::vector<std::string>& deviceIds) override;
};
} // namespace DistributedSchedule
} // namespace OHOS
#endif // OHOS_DISTRIBUTED_SCHED_CONTINUATION_MANAGER_TEST_H