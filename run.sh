#!/bin/bash

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
echo ./tnvme -k skip.cfg $TNVME_CMD_LINE 2>&1 | tee ./Logs/tnvme.out
./tnvme -k skip.cfg $TNVME_CMD_LINE 2>&1 | tee ./Logs/tnvme.out
