#!/bin/bash

#******************************************************************************
#
# ACPICA package generation script
#
# Execute this script from the acpica/generate/release directory.
#
# Constructed packages are placed in the acpica/generate/release/current
# directory.
#
# Licensing: ACPICA is distributed under the BSD-3-Clause license or the
# GPL-2.0-only license. The source files carry only the corresponding SPDX
# license identifier, so the full text of both licenses is copied into the
# root of every package.
#
# Usage:
#
#   build <package_type> <target_type>
#
#   where:
#       <package_type> is one of:
#           source  - Build an ACPICA source package (core and all tools)
#           test    - Build an ACPICA test suite package
#
#       <target_type> is one of:
#           unix    - Generate Unix package (both .tar.gz and .zip)
#
#******************************************************************************

# Configuration

ZIP_UTILITY="zip"

# Filenames and paths

TARGET_DIR="generate/release/current"
TEMP_DIR=acpitemp
TEST_PREFIX=acpitests
SOURCE_PREFIX=acpica
PACKAGE_SUFFIX=`date +%Y%m%d`

# Full license text, copied into the root of every package

LICENSE_FILES="LICENSE.BSD-3-Clause LICENSE.GPL-2.0-only"

NPARAM=$#


#******************************************************************************
#
# Miscellaneous utility functions
#
#******************************************************************************

usage()
{
	echo "$1"
	echo
	echo "Low-level build script for ACPICA release packages"
	echo "Usage:"
	echo "    $0 source <unix>"
	echo "    $0 test   <unix>"
}

banner()
{
	echo
	echo "$1"
	echo
}

check_zip_utility_exists()
{
	#
	# Need zip to build the .zip form of each package
	#
	if ! command -v $ZIP_UTILITY > /dev/null; then
		echo "ZIP_UTILITY ($ZIP_UTILITY) does not exist!"
		exit 1
	fi
}

copy_license_files()
{
	#
	# The source files identify the ACPICA license via an SPDX identifier
	# only, so the license text itself must be part of every package
	#
	echo "Copying the ACPICA license files"
	cp $LICENSE_FILES $TEMP_DIR
}

build_unix_package()
{
	#
	# Build release package, in both .tar.gz and .zip form
	#
	rm -r -f $PACKAGE_FILENAME
	mv $TEMP_DIR $PACKAGE_FILENAME
	tar czf $PACKAGE_FILENAME.tar.gz $PACKAGE_FILENAME

	rm -f $PACKAGE_FILENAME.zip
	$ZIP_UTILITY -q -r $PACKAGE_FILENAME.zip $PACKAGE_FILENAME

	#
	# Move the completed packages
	#
	mv $PACKAGE_FILENAME.tar.gz $PACKAGE_FILENAME.zip $TARGET_DIR
	mv $PACKAGE_FILENAME $TEMP_DIR
}


#******************************************************************************
#
# generate_source_package
#
# Generates the ACPICA source code packages (core and all tools)
#
# Arguments:
#   %1  - Target type (unix)
#
#******************************************************************************

generate_source_package ()
{
	#
	# Parameter evaluation
	#
	if [ $1 == unix ]; then
		PACKAGE_NAME="Unix"

	else
		usage "Invalid argument ($1)"
		exit 1
	fi

	PACKAGE_FILENAME=$SOURCE_PREFIX-$1-$PACKAGE_SUFFIX
	banner "ACPICA - Generating $PACKAGE_NAME source code package ($PACKAGE_FILENAME)"

	#
	# Make directories common to all source packages
	#
	mkdir $TEMP_DIR
	mkdir $TEMP_DIR/generate
	mkdir $TEMP_DIR/generate/lint
	mkdir $TEMP_DIR/generate/release
	mkdir $TEMP_DIR/generate/unix
	mkdir $TEMP_DIR/generate/unix/acpibin
	mkdir $TEMP_DIR/generate/unix/acpidump
	mkdir $TEMP_DIR/generate/unix/acpiexamples
	mkdir $TEMP_DIR/generate/unix/acpiexec
	mkdir $TEMP_DIR/generate/unix/acpihelp
	mkdir $TEMP_DIR/generate/unix/acpisrc
	mkdir $TEMP_DIR/generate/unix/acpixtract
	mkdir $TEMP_DIR/generate/unix/iasl
	mkdir $TEMP_DIR/tests
	mkdir $TEMP_DIR/tests/misc
	mkdir $TEMP_DIR/tests/templates
	mkdir -p $TEMP_DIR/source/os_specific/service_layers

	#
	# Copy ACPICA subsystem source code
	#
	cp documents/changelogs.md              $TEMP_DIR/changelogs.md
	cp -r source/common                     $TEMP_DIR/source/common
	cp -r source/components                 $TEMP_DIR/source/
	cp -r source/include                    $TEMP_DIR/source/include
	cp -r generate/release/*.sh             $TEMP_DIR/generate/release

	#
	# Copy iASL compiler and tools source
	#
	cp -r source/compiler                   $TEMP_DIR/source/compiler
	cp -r source/tools                      $TEMP_DIR/source/tools

	#
	# Copy iASL/ACPICA miscellaneous tests (not full test suites)
	#
	cp -r tests/misc/*.asl                  $TEMP_DIR/tests/misc
	cp -r tests/templates/Makefile          $TEMP_DIR/tests/templates
	cp -r tests/templates/templates.sh      $TEMP_DIR/tests/templates

	#
	# Copy all OS-specific interfaces
	#
	cp source/os_specific/service_layers/*.c $TEMP_DIR/source/os_specific/service_layers

	#
	# Copy generic UNIX makefiles
	#
	cp Makefile                             $TEMP_DIR
	cp generate/unix/readme.txt             $TEMP_DIR/generate/unix/readme.txt
	cp generate/unix/Makefile*              $TEMP_DIR/generate/unix
	cp generate/unix/acpibin/Makefile       $TEMP_DIR/generate/unix/acpibin
	cp generate/unix/acpidump/Makefile      $TEMP_DIR/generate/unix/acpidump
	cp generate/unix/acpiexamples/Makefile  $TEMP_DIR/generate/unix/acpiexamples
	cp generate/unix/acpiexec/Makefile      $TEMP_DIR/generate/unix/acpiexec
	cp generate/unix/acpihelp/Makefile      $TEMP_DIR/generate/unix/acpihelp
	cp generate/unix/acpisrc/Makefile       $TEMP_DIR/generate/unix/acpisrc
	cp generate/unix/acpixtract/Makefile    $TEMP_DIR/generate/unix/acpixtract
	cp generate/unix/iasl/Makefile          $TEMP_DIR/generate/unix/iasl

	#
	# Copy Lint directory
	#
	cp -r generate/lint $TEMP_DIR/generate
	rm -f $TEMP_DIR/generate/lint/co*
	rm -f $TEMP_DIR/generate/lint/env*
	rm -f $TEMP_DIR/generate/lint/lib*
	rm -f $TEMP_DIR/generate/lint/LintOut.txt

	copy_license_files
	build_unix_package

	banner "ACPICA - Completed $PACKAGE_NAME source code package ($PACKAGE_FILENAME)"
}


#******************************************************************************
#
# generate_test_package
#
# Generates the ACPICA test suite packages
#
# Arguments:
#   %1  - Target type (unix)
#
#******************************************************************************

generate_test_package()
{
	#
	# Parameter evaluation
	#
	if [ $1 == unix ]; then
		PACKAGE_NAME="Unix"

	else
		usage "Invalid argument ($1)"
		exit 1
	fi

	PACKAGE_FILENAME=$TEST_PREFIX-$1-$PACKAGE_SUFFIX
	banner "ACPICA - Generating $PACKAGE_NAME test suite package ($PACKAGE_FILENAME)"

	#
	# Copy the ASL Test source
	#
	mkdir $TEMP_DIR
	cp -r tests $TEMP_DIR/tests

	#
	# Delete extraneous files
	#
	cd $TEMP_DIR
	find . -name "tmp" | xargs rm -r -f
	find . -name "aml" | xargs rm -r -f
	find . -name "CVS" | xargs rm -r -f
	cd ..

	copy_license_files
	build_unix_package

	banner "ACPICA - Completed $PACKAGE_NAME test suite package ($PACKAGE_FILENAME)"
}


#******************************************************************************
#
# main
#
# Arguments:
#       $1 (package_type) is one of:
#           source  - Build an ACPICA source package (core and all tools)
#           test    - Build an ACPICA test suite package
#
#       $2 (target_type) is one of:
#           unix    - Generate Unix package (both .tar.gz and .zip)
#
#******************************************************************************

set -e		# Abort on any error

if [ $NPARAM -ne 2 ]; then
	usage "Wrong argument count ($NPARAM)"
	exit 1
fi

check_zip_utility_exists

#
# cd from acpica/generate/release to acpica
#
cd ../..

#
# Ensure that the temporary directory is created fresh
#
rm -rf $TEMP_DIR

#
# Parameter evaluation
#
if [ $1 == source ]; then
	generate_source_package $2

elif [ $1 == test ]; then
	generate_test_package $2

else
	usage "Invalid argument ($1)"
	exit 1
fi

#
# Remove temporary directory
#
rm -rf $TEMP_DIR
