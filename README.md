# RediNav
RediNav is a GUI management tool for Redis, forked from [RedisDesktopManager](https://github.com/uglide/RedisDesktopManager) and developed by [SCAVASOFT](https://www.scavasoft.com/)  

RediNav是[RedisDesktopManager](https://github.com/uglide/RedisDesktopManager)的一个fork版本, 开发者是[SCAVASOFT](https://www.scavasoft.com/).
因为RedisDesktopManager现在给Linux只发布snap版本, 而自己编译的话, 用到的qml版本2.15高于Ubuntu20.04的qt版本5.12, 运行需要带上高版本的Qt库, 无论怎样都会比较吃资源.
RediNav相对而言比较精简, 可以考虑在其基础上根据自己需求定制.

如果只是希望直接安装使用的, 可以在 https://www.redinav.com/ 上下载安装版本.

## What Is This
The original code of this repository was downloaded from [RediNav](https://www.redinav.com/), the version is v1.0.24, several versions behind the latest release. I just did some work to make it compilable on Ubuntu20.04 with Qt5.14.2.

这个仓库对源代码进行了一些调整和精简, 以便于在Ubuntu/Debian系统下编译. 当前使用的环境为Ubuntu20.04.

## How To

**Install dependencies**

```shell
sudo apt install automake libtool libssl-dev g++ libgl1-mesa-dev zlib1g-dev cmake build-essential qt5-default libssl-dev qt5keychain-dev
```

**Checkout the code and compile**

```shell
git clone --recursive https://github.com/IOsetting/redinav.git
cd redinav
./redinav/build-project-linux.sh -r -p
```

**Run**

The runnable is located at bin/linux/release/redinav.

## License

The code of this repository is licensed under GPL v3.
