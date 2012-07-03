mdLine/gCmdLine/g

# Copyright (c) 2011, Intel Corporation.
#
mdLine/gCmdLine/g
mdLine/gCmdLine/g
mdLine/gCmdLine/g
#
mdLine/gCmdLine/g
#
mdLine/gCmdLine/g
mdLine/gCmdLine/g
mdLine/gCmdLine/g
mdLine/gCmdLine/g
mdLine/gCmdLine/g
#

RPMCOMPILEDIR=$1
RPMSPECFILE=$2
RPMSRCFILE=$3

mdLine/gCmdLine/g
mdLine/gCmdLine/g
mdLine/gCmdLine/g
mdLine/gCmdLine/g
mdLine/gCmdLine/g
mdLine/gCmdLine/g
echo ""
}

mdLine/gCmdLine/g
if [[ $EUID -eq 0 ]]; then
mdLine/gCmdLine/g
  exit
fi

if [ -z $RPMCOMPILEDIR ] || [ -z $RPMSPECFILE ] || [ -z $RPMSRCFILE ]; then
mdLine/gCmdLine/g
  exit
fi

mdLine/gCmdLine/g
mdLine/gCmdLine/g
mdLine/gCmdLine/g
rm -rf $RPMCOMPILEDIR
mkdir -p $RPMCOMPILEDIR/{BUILDROOT,BUILD,RPMS,S{OURCE,PEC,RPM}S}
cp -p $RPMSRCFILE.tar.gz  $RPMCOMPILEDIR
cp -p $RPMSRCFILE.tar.gz  $RPMCOMPILEDIR/SOURCES
cp -p ${RPMSPECFILE} $RPMCOMPILEDIR/SPECS
cd $RPMCOMPILEDIR/SPECS
rpmbuild --define "_topdir ${RPMCOMPILEDIR}" --define "_major $MAJOR" --define "_minor $MINOR" -ba ${RPMSPECFILE}

exit
