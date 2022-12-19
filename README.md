**NOTICE**

The distributedschedule_dms_fwk repository is renamed ability_dmsfwk since July 2022. The distributedschedule_dms_fwk repository is archived and no longer maintained.

To obtain the latest code, go to the [**ability\_dmsfwk**](https://gitee.com/openharmony/ability_dmsfwk) repository.

# Distributed Scheduler<a name="EN-US_TOPIC_0000001115719369"></a>

## Introduction<a name="section11660541593"></a>

Distributed Scheduler is used for cross-device component management. It allows the local device to access or control remote components, and enables app collaboration in distributed scenarios. Its main functions are as follows:

-   Remote ability startup: Starts an ability on a remote device.
-   Remote ability migration: Migrates an ability to a remote device.
-   Remote ability binding: Binds an ability on a remote device.

## Architecture<a name="section13587185873516"></a>

**Figure 1**  Distributed Scheduler architecture<a name="fig4460722185514"></a>


![](figures/dms-architecture.png)

## Directory Structure<a name="section161941989596"></a>

```
/foundation/ability
├── dmsfwk              # Distributed Manager Service
│  ├── bundle.json      # Description and build script of dmsfwk
│  ├── etc              # Configuration files
│  ├── interfaces       # APIs exposed externally
│  ├── services         # Service implementation
│  ├── sa_profile       # SA profiles
│  ├── utils            # Utils
```

## Repositories Involved<a name="section1371113476307"></a>

**Distributed Scheduler**

[**ability\_dmsfwk**](https://gitee.com/openharmony/ability_dmsfwk)

[ability\_dmsfwk\_lite](https://gitee.com/openharmony/ability_dmsfwk_lite)
