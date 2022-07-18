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

#include "native_engine/native_engine.h"
#include "js_continuation_manager.h"

__attribute__((constructor)) static void NAPI_application_continuationmanager_AutoRegister()
{
    auto moduleManager = NativeModuleManager::GetInstance();
    NativeModule newModuleInfo = {
        .name = "continuation.continuationManager",
        .fileName = "continuation/libcontinuationmanager_napi.so/continuationmanager.js",
        .registerCallback = OHOS::DistributedSchedule::JsContinuationManagerInit,
    };

    moduleManager->Register(&newModuleInfo);
}