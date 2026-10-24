#!/bin/bash

#******************************************************************************
#
# ACPICA release generation script
#
# front end for build.sh
#
# Copies any existing packages to the archive directory.
#
# Generates 2 types of package, each in both .tar.gz and .zip form:
#   1) Standard ACPICA source, everything except test suites
#   2) ACPICA test suites (very large)
#
# On completion, the release directory holds every file that makes up a
# release: the two packages in both forms, the ACPICA utilities, and the
# release notes. These are the files to be attached to the GitHub release.
#
# Note: The ACPICA utilities are not built by this script - build them first
# with "make clean; make" in the acpica/generate/unix directory.
#
# Note: ACPICA is distributed under the BSD-3-Clause license or the
# GPL-2.0-only license. Both packages include the full text of both licenses,
# since the source files only carry the SPDX license identifier.
#
#******************************************************************************

# Configuration

NPARAM=$#
BUILD_TESTS=1

# Filenames and paths

ARCHIVE_DIR=archive
RELEASE_DIR=current
BINARY_DIR=../unix/bin
CHANGELOG=../../documents/changelogs.md

# The ACPICA utilities, released as-is alongside the source packages

BINARY_FILES="acpibin acpidump acpiexamples acpiexec acpihelp acpisrc \
acpixtract iasl"


#******************************************************************************
#
# Miscellaneous utility functions
#
#******************************************************************************

usage()
{
	echo "$1"
	echo
	echo "Master script to create ACPICA release packages"
	echo "Usage:"
	echo "    $0 [notest]"
}

move_all_files_to_archive()
{
	cd $RELEASE_DIR

	for file in *
	do
		if [ -d $file ]; then
			rm -r -f ../$ARCHIVE_DIR/$file
			mv -f $file ../$ARCHIVE_DIR
			echo "Moved directory $file to $ARCHIVE_DIR directory"
		else
			cp $file ../$ARCHIVE_DIR
			echo "Moved $file ($(ls -al $file | awk '{print $5}') bytes) to $ARCHIVE_DIR directory"
			rm $file
		fi
	done

	cd ..
}

check_binaries_exist()
{
	#
	# The utilities are built separately - do not start a release without them
	#
	for file in $BINARY_FILES
	do
		if [ ! -e $BINARY_DIR/$file ]; then
			echo "$BINARY_DIR/$file does not exist!"
			echo "Build the ACPICA utilities first:"
			echo "    (cd ../unix; make clean; make)"
			exit 1
		fi
	done
}

copy_binaries_to_release()
{
	#
	# The utilities are released as individual executables, not as a package
	#
	for file in $BINARY_FILES
	do
		cp $BINARY_DIR/$file $RELEASE_DIR
		echo "Copied $file to $RELEASE_DIR directory"
	done
}

copy_changelog_to_release()
{
	#
	# The release notes are attached to the release as a separate file
	#
	cp $CHANGELOG $RELEASE_DIR
	echo "Copied $(basename $CHANGELOG) to $RELEASE_DIR directory"
}


#******************************************************************************
#
# main
#
# Arguments:
#    $1 (optional) notest - do not generate the ACPICA test suite packages
#
#******************************************************************************

set -e		# Abort on any error

#
# Parameter evaluation
#
if [ $NPARAM -gt 1 ]; then
	usage "Wrong argument count ($NPARAM)"
	exit 1

elif [ $NPARAM -eq 1 ]; then
	if [ $1 == notest ]; then
		BUILD_TESTS=0
	else
		usage "Invalid argument ($1)"
		exit 1
	fi
fi

#
# Fail before anything is moved or generated if the utilities are missing
#
check_binaries_exist

#
# Move and preserve any previous versions of the various release packages
#
if [ -e $RELEASE_DIR ]; then

	# Create archive directory if necessary

	mkdir -p $ARCHIVE_DIR

	#
	# Save any older versions of the release packages
	#
	if [ "$(ls -A $RELEASE_DIR)" ]; then
		echo "Moving previous packages to $ARCHIVE_DIR directory"

		move_all_files_to_archive
		echo "Completed move of previous packages to $ARCHIVE_DIR directory"
	fi

else
	# Just create the release directory
	mkdir -p $RELEASE_DIR
fi

# ACPICA source code (core subsystem and all tools/utilities)

bash build.sh source unix

# Optionally build the test suite packages (built by default)

if [ $BUILD_TESTS -eq 1 ]; then

	# ACPICA test suites

	bash build.sh test unix

else
	echo "**** Test suites not built because the notest option was used"
fi

# ACPICA utilities and release notes

echo
copy_binaries_to_release
copy_changelog_to_release

echo
echo "ACPICA - Summary of release files:"
echo
ls $RELEASE_DIR -g -G -t
