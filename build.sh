#!/bin/bash

# Copyright (c) 2011, Intel Corporation.
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
#

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
MAJOR=`awk 'FNR == 26' version.h`
MINOR=`awk 'FNR == 29' version.h`
rm -rf $RPMCOMPILEDIR
mkdir -p $RPMCOMPILEDIR/{BUILDROOT,BUILD,RPMS,S{OURCE,PEC,RPM}S}
cp -p $RPMSRCFILE.tar.gz  $RPMCOMPILEDIR
cp -p $RPMSRCFILE.tar.gz  $RPMCOMPILEDIR/SOURCES
cp -p ${RPMSPECFILE} $RPMCOMPILEDIR/SPECS
cd $RPMCOMPILEDIR/SPECS
rpmbuild --define "_topdir ${RPMCOMPILEDIR}" --define "_major $MAJOR" --define "_minor $MINOR" -ba ${RPMSPECFILE}

exit
