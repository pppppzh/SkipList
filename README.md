# SkipLislt
跳表是一种非关系型数据库中，存储引擎常用的数据结构，如redis。本项目使用C++编程，实现了基于跳表的轻量级的键值型存储引擎。项目包含数据的插入、删除、查询、打印、转储、文件加载等操作。

# 项目中文件

* main.cpp 包含skiplist.h使用跳表进行数据操作
* skiplist.h 跳表核心实现
* README.md 项目介绍        
* bin 生成可执行文件目录 
* makefile 编译脚本
* stress_test.cpp 压力测试代码
* start_stress_test.sh 压力测试脚本

# 提供接口

* insertElement（插入数据）
* deleteElement（删除数据）
* searchElement（查询数据）
* displayList（展示已存数据）
* dumpFile（数据落盘）
* loadFile（加载数据）
* size（返回数据规模）


# 存储引擎数据表现
跳表树高：18 

采用随机插入数据测试：


|插入数据规模（万条） |耗时（秒） | 
|---|---|
|10 |0.259944 |
|50 |1.17261 |
|100 |3.1025 |


每秒可处理写请求数（QPS）: 32.23w


# 项目运行方式

```
make                      // 编译
./bin/main                // 运行
sh start_stress_test.sh   //压力测试
```



