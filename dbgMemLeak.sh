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

echo ./tnvme -k skipqemu.cfg $TNVME_CMD_LINE 2>&1 | tee tnvme.out
valgrind --tool=memcheck ./tnvme -k skipqemu.cfg $TNVME_CMD_LINE 2>&1 | tee tnvme.out
#valgrind --tool=memcheck --leak-check=full --track-origins=yes -v --show-reachable=yes ./tnvme -k skipqemu.cfg $TNVME_CMD_LINE 2>&1 | tee tnvme.out
