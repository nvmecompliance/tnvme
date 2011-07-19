#!/bin/bash

RPMCOMPILEDIR=$1
RPMSPECFILE=$2
RPMSRCFILE=$3

Usage() {
echo "usage...."
echo "  $0 <RPM_compile> <RPM_spec> <RPM_src>"
echo "    <RPM_compile>  Specify full path to the base RPM compilation dir"
echo "    <RPM_spec>     Specify filename only of the RPM spec file (*.spec)"
echo "    <RPM_src>      Specify full path to the RPM source file (*.tar.gz)"
echo ""
}

# RPM build errors under ROOT a potentially catastrophic
if [[ $EUID -eq 0 ]]; then
  echo Running as 'root' is not supported
  exit
fi

if [ -z $RPMCOMPILEDIR ] || [ -z $RPMSPECFILE ] || [ -z $RPMSRCFILE ]; then
  Usage
  exit
fi

# Setup a fresh RPM build environment, and then build
MAJOR=`awk 'FNR == 5' version.h`
MINOR=`awk 'FNR == 8' version.h`
rm -rf $RPMCOMPILEDIR
mkdir -p $RPMCOMPILEDIR/{BUILDROOT,BUILD,RPMS,S{OURCE,PEC,RPM}S}
cp -p $RPMSRCFILE.tar.gz  $RPMCOMPILEDIR
cp -p $RPMSRCFILE.tar.gz  $RPMCOMPILEDIR/SOURCES
cp -p ${RPMSPECFILE} $RPMCOMPILEDIR/SPECS
cd $RPMCOMPILEDIR/SPECS
rpmbuild --define "_topdir ${RPMCOMPILEDIR}" --define "_major $MAJOR" --define "_minor $MINOR" -ba ${RPMSPECFILE}

exit
