#!/bin/bash

TNVME_CMD_LINE=$1

Usage() {
echo "usage...."
echo "  $0 <tnvme cmd line options>"
echo ""
}

if [ -z $TNVME_CMD_LINE ]; then
  Usage
  exit
fi

./tnvme -k skipqemu.cfg $TNVME_CMD_LINE 2>&1 1>/dev/null | tee tnvme.out
