#!/bin/bash

APP_DIR="app"
SRC_DIR="$APP_DIR/jni/src"
MAIN_DIR="$APP_DIR/src/main"

BUILD_FILE="$APP_DIR/build.gradle"
BUILD_TMPL="$BUILD_FILE.tmpl"

MANIFEST_FILE="$MAIN_DIR/AndroidManifest.xml"
MANIFEST_TMPL="$MANIFEST_FILE.tmpl"

MAIN_H="$SRC_DIR/main.h"

VERSION_SUPER=`grep "#define PROGRAM_VERSION_SUPER[[:space:]]" $MAIN_H | awk '{print $3}'`
VERSION_MAJOR=`grep "#define PROGRAM_VERSION_MAJOR[[:space:]]" $MAIN_H | awk '{print $3}'`
VERSION_MINOR=`grep "#define PROGRAM_VERSION_MINOR[[:space:]]" $MAIN_H | awk '{print $3}'`
VERSION_PATCH=`grep "#define PROGRAM_VERSION_PATCH[[:space:]]" $MAIN_H | awk '{print $3}'`
VERSION_EXTRA=`grep "#define PROGRAM_VERSION_EXTRA[[:space:]]" $MAIN_H | awk '{print $3}'`
VERSION_EXTRA_TEXT=`grep "#define PROGRAM_VERSION_EXTRA_TEXT[[:space:]]" $MAIN_H	\
    | awk -F\" '{print $2}'								\
    | tr '[A-Z ]' '[a-z-]'`

UNIQUE_VERSION=`echo "$VERSION_SUPER" | wc -l | awk '{ print $1 }'`
if [ "$UNIQUE_VERSION" != "1" ]; then
    echo "ERROR: program super version number ('PROGRAM_VERSION_SUPER') not unique!"
    exit 10
fi

UNIQUE_VERSION=`echo "$VERSION_EXTRA" | wc -l | awk '{ print $1 }'`
if [ "$UNIQUE_VERSION" != "1" ]; then
    echo "ERROR: program extra version number ('PROGRAM_VERSION_EXTRA') not unique!"
    exit 10
fi

VERSION_NAME="$VERSION_SUPER.$VERSION_MAJOR.$VERSION_MINOR.$VERSION_PATCH"
VERSION_CODE=$(printf '%d%02d%02d%02d'	\
    "$VERSION_SUPER"			\
    "$VERSION_MAJOR"			\
    "$VERSION_MINOR"			\
    "$VERSION_PATCH")

if [ "$VERSION_EXTRA" != "0" ]; then
    VERSION_NAME="$VERSION_NAME-$VERSION_EXTRA_TEXT-$VERSION_EXTRA"
fi

# echo "::: VERSION_NAME == '$VERSION_NAME'"
# echo "::: VERSION_CODE == '$VERSION_CODE'"

cat "$BUILD_TMPL"					\
    | sed -e "s/__VERSION_NAME__/$VERSION_NAME/"	\
    | sed -e "s/__VERSION_CODE__/$VERSION_CODE/"	\
    > "$BUILD_FILE"

cat "$MANIFEST_TMPL"					\
    | sed -e "s/__VERSION_NAME__/$VERSION_NAME/"	\
    | sed -e "s/__VERSION_CODE__/$VERSION_CODE/"	\
    > "$MANIFEST_FILE"

exit 0
