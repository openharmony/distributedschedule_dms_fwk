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

import router from '@system.router';
import pageName from '../MainAbility/stringConst';
import Logger from '../MainAbility/Logger';

async function routePage(pageIndex) {
  Logger.info("routePage");
  let options = {
    uri: pageIndex
  }
  try {
    await router.push(options)
  } catch (err) {
    Logger.error(`fail callback, code: ${err.code}, msg: ${err.msg}`)
  }
}

@Entry
@Component
struct Index {
  @State message: string = '记事本(免安装)'

  build() {
    Flex({ direction: FlexDirection.Column, alignItems: ItemAlign.Center, justifyContent: FlexAlign.Center }) {
      Text(this.message)
        .fontSize(50)
        .fontWeight(FontWeight.Bold)
      Button() {
        Text(pageName.page1)
          .fontSize(35)
          .fontWeight(FontWeight.Bold)
      }
      .type(ButtonType.Normal)
      .margin({
        top: 30
      })
      .backgroundColor('#2cb3cb')
      .onClick(() => routePage('pages/work'))

      Button() {
        Text(pageName.page2)
          .fontSize(35)
          .fontWeight(FontWeight.Bold)
      }
      .type(ButtonType.Normal)
      .margin({
        top: 60
      })
      .backgroundColor('#2cb3cb')
      .onClick(() => routePage('pages/study'))
    }
    .width('100%')
    .height('100%')
  }
}