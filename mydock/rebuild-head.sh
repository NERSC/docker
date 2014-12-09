#!/bin/sh

VERS=$(cat ../VERSION)
BUILDDIR=builddir
echo $VERSION

echo "Creating tar file"
cd ../../
mkdir $BUILDDIR
mkdir $BUILDDIR/SOURCES
mkdir $BUILDDIR/SPECS
tar czf $BUILDDIR/SOURCES/docker-head.tar.gz docker
cd docker/mydock

cp docker.sysconfig docker-storage.sysconfig docker.sysvinit workarounds.diff ../../$BUILDDIR/SOURCES/

rpmbuild --define '_topdir '`cd ../../;pwd`/${BUILDDIR} -ba ./docker-io-head.spec || exit
#rpm -Uhv --force ~/rpmbuild/RPMS/x86_64/docker-io-$(cat ../VERSION|sed 's/-//')-*nersc*rpm || exit
#/etc/init.d/docker restart


