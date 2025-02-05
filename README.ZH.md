# 准备
1. 确保你已经有本项目对应的硬件并正确安装 [指南](Hardware/README.EN.md).
2. 在树莓派上编译本项目. [指南](Software/README.EN.md).
# 使用
1. 确定自己位于编译生成的可执行文件的目录下
2. 运行程序
    ```bash
    ./keyboard_latency_test
    ```
3. 按照程序中出现的指示测试屏幕
## 运行参数
| Prameter |    Value    | Description                      |
| :------: | :---------: | :------------------------------- |
|    -a    |    none     | 负载阻抗模式                     |
|    -b    | millisecond | 设定每次触发测试本身的时间间隔   |
|    -h    |    none     | 显示帮助信息与可用命令/选项      |
|    -l    |    none     | 使用负载阻抗模式测试             |
|    -n    |   number    | 设置测试次数                     |
|    -s    |  filename   | 保存本次测试结果（文件名可选填） |
|    -t    | millisecond | 设定两次测试间的时间间隔         |
|    -v    |    none     | 显示当前程序版本                 |
### 使用示例
以下命令会执行一次触发时间为50ms，两次触发间隔为100ms的测试，结果将会被保存至test_result文件

```bash
./keyboard_latency_test -t 50 -b 100 -s test_result
```
