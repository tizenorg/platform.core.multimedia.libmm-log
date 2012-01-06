#!/bin/sh

### WARNING: DO NOT CHANGE CODES from HERE !!! ###
#import setup
cd `dirname $0`
_PWD=`pwd`
pushd ./ > /dev/null
while [ ! -f "./xo-setup.conf" ]
do
	cd ../
	SRCROOT=`pwd`
	if [ "$SRCROOT" == "/" ]; then
		echo "Cannot find xo-setup.conf !!"
		exit 1
	fi
done
popd > /dev/null
. ${SRCROOT}/xo-setup.conf
cd ${_PWD}
### WARNING: DO NOT CHANGE CODES until HERE!!! ###

export VERSION=1.0
# logmanager -- setting SECTION .. hyunil46.par@samsung.com
# reference : logmanager/logmessage.conf.in
# [setting]
export direction=3
export ownermask=0xFFFF
export classmask=0x0C
export filename=/var/log/logmessage.log
#[color]
export classinfomation=default
export classwarning=BLUE
export classerror=RED
export clsascritical=RED
export classassert=RED

# ARCH=arm/i686
# MACHINE=protector/floater
# DISTRO=vodafone/openmarket
echo "################################"
echo "setup=[$ARCH][$MACHINE][$DISTRO]"
echo "################################"

CFLAGS="-DEXPORT_API=\"__attribute__((visibility(\\\"default\\\")))\""
if [ $1 ];
then
	run make $1 || exit 1
else
	if [ -z "$USE_AUTOGEN" ]; then
		run ./autogen.sh || exit 1
		run ./configure --prefix=$PREFIX || exit 1
	fi
	run make || exit 1
	run make check || exit 1
	run make install || exit 1
	run make_pkg.sh || exit 1
fi
