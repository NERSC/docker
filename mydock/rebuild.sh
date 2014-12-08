#!/bin/sh
git diff v1.3.2 -- > ~/rpmbuild/SOURCES/nersc.diff 

cd ~/rpmbuild
rpmbuild -ba SPECS/docker-io.spec || exit
rpm -Uhv --force RPMS/x86_64/docker-io-1*nersc*rpm || exit

/etc/init.d/docker restart


