#!/bin/sh

. ./t/functions
echo -n "No Op test: "
/usr/bin/mydock 2>&1|grep -c Usage > /dev/null 
passfail $?

echo -n "Run test: "
/usr/bin/mydock run -i -t ubuntu /bin/echo true|grep -c true > /dev/null
passfail $?

# Legacy mode
echo -n "Legacy test: "
/usr/bin/mydock -i -t ubuntu /bin/echo true|grep -c true > /dev/null
passfail $?

echo -n "Bad flag test: "
/usr/bin/mydock --privileged -i -t ubuntu /bin/true 2>&1|grep -c Unallowed > /dev/null 
passfail $?
