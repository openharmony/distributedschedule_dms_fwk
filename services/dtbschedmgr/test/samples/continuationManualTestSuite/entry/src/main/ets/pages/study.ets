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

@Entry
@Component
struct Study {
  private content: string = pageName.page2
  @StorageLink('ContinueStudy') inputTxt2: string = '';
  build() {
    Flex({ direction: FlexDirection.Column, alignItems: ItemAlign.Center, justifyContent: FlexAlign.Center }) {
      Text(`${this.content}`)
        .fontSize(50)
        .fontWeight(FontWeight.Bold)
      Column() {
        TextArea({ placeholder: 'please input', text: this.inputTxt2 }) // restore user input data on ui
          .textAlign(TextAlign.Start)
          .onChange((value) => {
            Logger.info(`TextInput onChange: ${value}`);
            this.inputTxt2 = value;
            AppStorage.Set('ContinueStudy', value);
          })
          .width('50%')
          .height(50)
      }

      Button() {
        Text(pageName.backKey)
          .fontSize(20)
          .fontWeight(FontWeight.Bold)
      }
      .type(ButtonType.Capsule)
      .margin({
        top: 20
      })
      .backgroundColor('#0D9FFB')
      .onClick(() => {
        router.back()
      })
    }
    .width('100%')
    .height('100%')
  }
}