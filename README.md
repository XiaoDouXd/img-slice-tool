# 切图工具

#### 一、编译环境：

##### 1. 配置 freetype2

下载 [freetype2](https://github.com/freetype/freetype) 并编译，将编译好的静态库和头文件分别放在编译器的库目录和引用目录中。

##### 2. 配置 vulkan sdk

windows:

1. 前往 [LunarXchange (lunarg.com)](https://vulkan.lunarg.com/sdk/home)
2. 下载最新 vulkan SDK Installer 并安装
3. 安装完成后可运行安装目录下的 `Bin\vkcube.exe` 或在命令行运行 `vulkaninfo` 测试环境

linux:

- 可以类似 windows 在 [LunarXchange (lunarg.com)](https://vulkan.lunarg.com/sdk/home) 上获取 SDK 并安装
- 或者执行如下指令

```sh
#ubuntu16  vulkan版本1.126
wget -qO - http://packages.lunarg.com/lunarg-signing-key-pub.asc | sudo apt-key add -
sudo wget -qO /etc/apt/sources.list.d/lunarg-vulkan-1.1.126-xenial.list http://packages.lunarg.com/vulkan/1.1.126/lunarg-vulkan-1.1.126-xenial.list
sudo apt update
sudo apt install vulkan-sdk
```

#### 二、生成 rc

编辑项目根目录下的 `Config.cmake` 文件，将 `set(CC_RC_REGEN true)` 一行解除注释。重新生成 cmake 项目，若 `./gui/rc/` 目录下出现了 `gen` 文件夹，则可以初步判断生成成功。生成后请在 `Config.cmake` 中将上述行再度注释，否则会占用大量生成时间。
