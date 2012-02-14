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

TNVME_CMD_LINE=$@

Usage() {
echo "usage...."
echo "  $0 <tnvme cmd line options>"
echo ""
}

if [ -z $TNVME_CMD_LINE ]; then
  Usage
  exit
fi

rm -rf ./Logs
mkdir -m 0777 ./Logs
echo ./tnvme -k skiptest.cfg $TNVME_CMD_LINE 2>&1 | tee ./Logs/tnvme.out
#valgrind --tool=memcheck ./tnvme -k skiptest.cfg $TNVME_CMD_LINE 2>&1 | tee ./Logs/tnvme.out
valgrind --tool=memcheck --leak-check=full --track-origins=yes -v --show-reachable=yes ./tnvme -k skiptest.cfg $TNVME_CMD_LINE 2>&1 | tee ./Logs/tnvme.out
