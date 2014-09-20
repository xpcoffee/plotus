#!/bin/bash
if [ -e plotus_0.5.orig.tar.xz  ]
then
	echo "packages found. deleting..."
	rm plotus_*
else
	echo "no packages found. proceeding..."
fi

cd plotus-0.5

echo "cleaning..."
cd src
make clean
cd ..
dh clean

echo "making..."
qmake -nodepend
dh_make --createorig -s

echo "building..."
debuild

echo "done."

echo "Package contents:"

cd ..
./pkgcontents.sh
