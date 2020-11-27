#!/bin/bash
#
# A simple, somewhat clumzy, but working script to run qmake and then make the RediNav (Linux)
#
#   [] cd redinav
#   [] Run THIS script. Options:
#       -d   Build in 'debug' mode
#       -r   Build in 'release' mode
#       -p   Build in production mode
#       -s   Build in Staging mode
#
#   One of [-d or -r] AND one of [-p or -s] must be specified (to force developer knowing what he is doing)


################## DO NOT CHANGE BELOW

OPT_CONFIG_BUILD=""
OPT_CONFIG_LICENSE=""
OPT_CONFIG_PSMODE=""

usage() {
    echo "Usage: $0 -d|-r -l|-n" 1>&2
    echo "    -d   Build in 'debug' mode"
    echo "    -r   Build in 'release' mode"
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

if [ "x$OPT_CONFIG_PSMODE" == "x" ]; then
    usage
fi


DIR=$(dirname "$(readlink -f "$0")")
REDINAV_BASE_DIR=$DIR/..
SRC_DIR=$REDINAV_BASE_DIR/redinav
REDINAV_VERSION=$(sed -n '/REDINAV_VERSION/p' $SRC_DIR/modules/version.h| awk '{print $3}' | sed 's/"//g'):

cd $SRC_DIR
qmake redinav.pro -spec linux-g++ $OPT_CONFIG_BUILD $OPT_CONFIG_PSMODE
/usr/bin/make clean && /usr/bin/make -j4

