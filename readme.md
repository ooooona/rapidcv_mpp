# RapidCV_MPP项目

[![BK Pipelines Status](https://api.bkdevops.qq.com/process/api/external/pipelines/projects/rapidcv/p-4b4ba5e8d5d54c4597ffcb0e485d5318/badge?X-DEVOPS-PROJECT-ID=rapidcv)](http://v2.devops.oa.com/ms/process/api-html/user/builds/projects/rapidcv/pipelines/p-4b4ba5e8d5d54c4597ffcb0e485d5318/latestFinished?X-DEVOPS-PROJECT-ID=rapidcv)


## 项目描述
基于**海思(HISI)MPP媒体库**封装基于处理YUV基础组件。

目前支持：
- JPEG编码，包括缩放、裁剪
- YUV缩放
- YUV裁剪
- YUV硬件加速拷贝
- YUV文件保存和读取
- MMZ内存申请，包括cache和不可cache


## 使用手册
1. 蓝盾编译
点击bar进入蓝盾平台流水线，手动点击**执行**，等待流水线构建完成后，在**查看构建**可以看到构建完成的库文件。
2. 源代码编译
首先准备docker：
```
docker search docker.oa.com/demo/ai-camera:v2
docker login docker.oa.com/demo/ai-camera:v2
docker pull docker.oa.com/demo/ai-camera:v2 # 用户名密码分别是：rtx英文名、6位pin码+token

docker run -it --net=host -v /data/workspace/:/home/ docker.oa.com/demo/ai-camera:v1 /bin/bash # -v后跟的参数请自行选择本机的工作目录
```

进入docker后，请进入工作目录，之后执行如下命令：
```shell
git clone http://git.code.oa.com/oonamao/yt-hisi-mpp.git
cd yt-hisi-mpp
./rebuild.sh -t ${3516|3519} // 3516将得到HI3516DV300平台的APP包，同理，3519得到HI3519AV100的APP包
```
编译结束之后，将得到构建好的库文件`rapidcv_mpp`


## TODO
- 数据格式
    目前仅支持YUV420SP，即nv21格式；后期将扩展支持更多格式。
- 硬件拷贝
    超过1920x1080的YUV数据，尚未完整验证。
- 能力说明
    **YUV缩放**和**YUV裁剪**的实际输出最大能力取决于COMMON VB POOL的最大能力。

## 能力说明

| 平台 | 能力 | 输入范围 | 输出范围 | 输入对齐要求 | 输出对齐要求 | 备注 | 
| :---: | :---: | :---: | :---: | :---: | :---: | :---: |
| HI3516DV300 | **YUV缩放** | 宽度[64, 2688], 高度[64, 2688] |  宽度[64, 8192], 高度[64, 8192] | 高度、宽度2对齐；跨度16对齐 | 高度、宽度2对齐；跨度16对齐 | -- |
| HI3516DV300 | **YUV裁剪** | 宽度[64, 2688], 高度[64, 2688] |  宽度[64, 8192], 高度[64, 8192] | 高度、宽度2对齐；跨度16对齐 | 高度、宽度2对齐；跨度16对齐 | -- |
| HI3516DV300 | **JPEG编码** | 宽度[32, 8192], 高度[32, 8192] | 无 | 高度、宽度2对齐；跨度16对齐 | 无 | -- |
| HI3519AV100 | **YUV缩放** | 宽度[64, 8192], 高度[64, 8192] |  宽度[64, 8192], 高度[64, 8192] | 高度、宽度2对齐；跨度16对齐 | 高度、宽度2对齐；跨度16对齐 | -- |
| HI3519AV100 | **YUV裁剪** | 宽度[64, 8192], 高度[64, 8192] |  宽度[64, 8192], 高度[64, 8192] | 高度、宽度2对齐；跨度16对齐 | 高度、宽度2对齐；跨度16对齐 | -- |
| HI3519AV100 | **JPEG编码** | 宽度[32, 8192], 高度[32, 8192] | 无 | 高度、宽度2对齐；跨度16对齐 | 无 | -- |
