升级脚本
--------

* gen_sdupdate: 工厂sd卡升级文件组生成脚本。
* gen_otaupdate: 产品OTA升级包生成脚本。

升级包源文件
------------

- source/update_sys/zImage-ramfs: 带ramdisk的内核，命名定死为`zImage-ramfs`。
- source/image: 升级包文件目录，升级对象目录。
