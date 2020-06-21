# rapidcv_mpp

### 背景介绍
`rapidcv_mpp`是基于海思MPP实现一套对YUV处理的软件框架/工具，旨在适配海思Hi35xx系列芯片。

已经适配的平台：
- Hi3519AV100
- Hi3516DV300

已经完成的功能：
- YUV JPEG编码
- YUV SCALE
- YUV CROP

### 编译和使用
1. 用户自己准备编译环境（交叉编译链工具）
2. git clone本项目
3. 执行`./rebuild.sh -t [3516|3519]`即可。
