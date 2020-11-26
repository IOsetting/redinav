# RediNav
RediNav is a GUI management tool for Redis, forked from [RedisDesktopManager](https://github.com/uglide/RedisDesktopManager) and developed by [SCAVASOFT](https://www.scavasoft.com/)  

RediNav是一个图形化的Redis客户端软件, 从代码看是从[RedisDesktopManager](https://github.com/uglide/RedisDesktopManager) 早期版本中fork出来的,
开发者是[SCAVASOFT](https://www.scavasoft.com/). 因为RedisDesktopManager现在给Linux只发布臃肿的snap版本(我个人很不喜欢snap, 因为它非常吃资源),
而其源码的2019, 2020分支在Ubuntu20.04下均无法成功编译, 所以一直在寻找其替代品, RediNav是最接近我需求的. 需要安装文件的可以直接在 https://www.redinav.com/ 上下载.


## What Is This
The original code of this repository was downloaded from [RediNav](https://www.redinav.com/), the version is v1.0.24, several versions behind the latest release. I just did some work to make it compilable on Ubuntu20.04 with Qt5.14.2.

这个仓库对源代码进行了一些调整, 便于在Ubuntu/Debian发行版下编译. 当前通过的环境为Ubuntu20.04和Qt5.14.2.

## How To
**Install Qt5.14.2 from a Qt mirror**

Download qt-opensource-linux-x64-5.14.2.run and run it, install all componments except the Android one, in my case I install it in /opt/qt/Qt5.14.2/

**Install dependencies**

```shell
sudo apt install automake libtool libssl-dev g++ libgl1-mesa-dev zlib1g-dev cmake
```

**Checkout the code and compile dependencies**

```shell
git clone https://github.com/IOsetting/redinav.git
cd redinav
./redinav/configure
```

**Compile**

Set the QT_BASE_DIR in src/build-project-linux.sh
```
QT_BASE_DIR=/opt/qt/Qt5.14.2/5.14.2/gcc_64
```

Then compile

```shell
./redinav/build-project-linux.sh
```

**Run**

The runnable is located at bin/linux/release/redinav.

## License

The code of this repository is licensed under GPL v3.
