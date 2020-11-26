#!/bin/bash
#
# A simple, somewhat clumzy, but working script to run qmake and then make the RediNav (Linux)
#
#   [] Manually Install Qt 5.11.1 (or other version, but check below QT_BASE_DIR) Open Source in $HOME directory (https://www.qt.io/download)
#   [] cd redinav && ./configure
#   [] Run THIS script. Options:
#       -d   Build in 'debug' mode
#       -r   Build in 'release' mode
#       -l   Build WITH license management support (if you know what you are doing!)
#       -n   Build WITHOUT license management support (recommended)
#       -p   Build in production mode
#       -s   Build in Staging mode
#
#   One of [-d or -r] AND one of [-l or -n] AND one of [-p or -s] must be specified (to force developer knowing what he is doing)


# You can adjust the value according to your locally installe Qt version
QT_BASE_DIR=/opt/qt/Qt5.14.2/5.14.2/gcc_64


################## DO NOT CHANGE BELOW

OPT_CONFIG_BUILD=""
OPT_CONFIG_LICENSE=""
OPT_CONFIG_PSMODE=""

usage() {
    echo "Usage: $0 -d|-r -l|-n" 1>&2
    echo "    -d   Build in 'debug' mode"
    echo "    -r   Build in 'release' mode"
    echo "    -l   Build WITH license management support (if you know what you are doing!)"
    echo "    -n   Build WITHOUT license management support (recommended, creates license free application)"
    echo "    -p   Build in 'production' mode"
    echo "    -s   Build in 'staging' mode"
    echo "    -h   This help"
    exit 1
}

## Get options
while getopts ":drlnpsh" o; do
    case "${o}" in
    d)
        OPT_CONFIG_BUILD="CONFIG+=debug CONFIG-=release"
        ;;
    r)
        OPT_CONFIG_BUILD="CONFIG-=debug CONFIG+=release"
        ;;
    l)
        OPT_CONFIG_LICENSE="CONFIG+=license"
        ;;
    n)
        OPT_CONFIG_LICENSE="CONFIG-=license"
        ;;
    p)
        OPT_CONFIG_PSMODE="CONFIG+=production"
        ;;
    s)
        OPT_CONFIG_PSMODE="CONFIG-=production"
        ;;
    h)
        usage
        ;;
    *)
        ;;
    esac
done
shift $((OPTIND-1))

if [ "x$OPT_CONFIG_BUILD" == "x" ]; then
    usage
fi

if [ "x$OPT_CONFIG_LICENSE" == "x" ]; then
    usage
fi

if [ "x$OPT_CONFIG_PSMODE" == "x" ]; then
    usage
fi

REDINAV_VERSION=$(sed -n '/REDINAV_VERSION/p' modules/version.h| awk '{print $3}' | sed 's/"//g')


if [ ! -d "$QT_BASE_DIR" ]; then
    echo "Qt installation not found in $QT_BASE_DIR"
    exit 1
fi

QTDIR=$QT_BASE_DIR
PATH=$QT_BASE_DIR/bin:$PATH
LD_LIBRARY_PATH=$QT_BASE_DIR/lib/:$QT_BASE_DIR/lib:$LD_LIBRARY_PATH

DIR=$(dirname "$(readlink -f "$0")")
REDINAV_BASE_DIR=$DIR/..
SRC_DIR=$REDINAV_BASE_DIR/redinav

cd $SRC_DIR
$QT_BASE_DIR/bin/qmake redinav.pro -spec linux-g++ $OPT_CONFIG_BUILD $OPT_CONFIG_LICENSE $OPT_CONFIG_PSMODE
/usr/bin/make clean && /usr/bin/make -j4

