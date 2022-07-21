# 跨端迁移Demo

### 简介
本示例展示了在eTS中如何构建stage模型跨端迁移Ability，包含:

1、配置迁移属性字段：continuable和launchType

2、申请分布式权限：DISTRIBUTED_DATASYNC

3、实现数据保存和恢复：发起端页面输入的文本迁移到远端后能够显示

4、在迁移中使用分布式对象传输数据

效果图

![index](screenshots/device/index.png)  ![](screenshots/device/work.png)


### 使用说明
1、两台设备安装此应用，并组网

2、点击进入不同的page，通过迁移入口，将当前界面迁移到远端设备

3、迁移成功后的远端界面与迁移之前本机页面一致，本机页面消失

4、也可以再次将远端页面迁移到本机

### 约束与限制
1、本示例仅支持标准系统上运行。 2、本示例为Stage模型，仅支持API version 9，IDE版本3.0.1.900以上，系统版本OpenHarmony3.1 release。