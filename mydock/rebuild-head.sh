#!/bin/sh

VERS=$(cat ../VERSION)
echo $VERSION

echo "Creating tar file"
cd ../../
tar czf ~/rpmbuild/SOURCES/docker-head.tar.gz docker
cd docker/mydock

cp workarounds.diff ~/rpmbuild/SOURCES/

rpmbuild -ba ./docker-io-head.spec || exit
rpm -Uhv --force ~/rpmbuild/RPMS/x86_64/docker-io-$(cat ../VERSION|sed 's/-//')-*nersc*rpm || exit

/etc/init.d/docker restart


