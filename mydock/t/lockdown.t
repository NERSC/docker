#!/bin/sh

. ./t/functions

echo -n "Setuid Test: "
docker run --rm -i -t --user 501 ubuntu ping -c 1 8.8.8.8|grep -c Operation > /dev/null
passfail $?


echo -n "Privileged mode test: "
echo "mknod /tmp/mem c 1 1;dd if=/tmp/mem count=1"| docker run --rm --privileged=true -i ubuntu /bin/bash 2>&1|grep -c Operation > /dev/null 
passfail $?
