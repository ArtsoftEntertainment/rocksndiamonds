#!/bin/bash

# =============================================================================
# make_dist.sh
# -----------------------------------------------------------------------------
# create Rocks'n'Diamonds distribution packages for various platforms
#
# 2022-10-25 Holger Schemel <info@artsoft.org>
# =============================================================================

ACTION=$1
PLATFORM=$2

LOGFILE=$3


# -----------------------------------------------------------------------------
# configuration values
# -----------------------------------------------------------------------------

DEBUG_MODE_INITIAL="0"		# set to "1" to activate initial debug output


# -----------------------------------------------------------------------------
# generic definitions
# -----------------------------------------------------------------------------

HOME_BASEDIR_DEFAULT="/home"	# default: the package will be build on Linux
HOME_BASEDIR_MAC="/Users"	# special: Mac package will be build on a Mac

# list of potential directories for GNU binaries on Mac
BIN_DIR_LIST_MAC="/opt/homebrew/bin /usr/local/bin"

SSH_CMD="ssh -A"
SCP_CMD="scp -p"
RSYNC="rsync -aL"	# this is "rsync -a" without preserving symbolic links
MAKE="make"
SHA256_CMD="sha256sum"
CP="cp -aL"		# this is "cp -a" without preserving symbolic links
CP_ARCHIVE="cp -a"
SED_CMD="sed"

DIST_DATE=`date +%Y%m%d`
TIMESTAMP=`date "+%Y%m%d-%H%M%S"`

CMD_FILENAME=$0
CMD_BASENAME=`basename "$CMD_FILENAME"`
CONF_FILENAME=`echo "$0" | sed -e "s/\.sh$/.conf/"`

if [ "$ACTION" = "package" -a "$PLATFORM" != "" -a "$LOGFILE" = "" ]; then
    LOG_FILENAME=`echo "$0" | sed -e "s/\.sh$/-$TIMESTAMP-$PLATFORM.log/"`
    FILTER_LOG="tee -a $LOG_FILENAME"

    # restart script with logging
    $0 "$ACTION" "$PLATFORM" "$LOG_FILENAME" 2>&1 | $FILTER_LOG

    exit ${PIPESTATUS[0]}
fi


# !!! TEST ONLY !!!
#
#CMD_BASENAME=`basename "$CMD_FILENAME" .sh`
#CMD_BASEPATH=`dirname "$CMD_FILENAME"`
#
#LOG_DIR="$CMD_BASEPATH/logs"
#CONF_DIR="$CMD_BASEPATH"
#
#LOG_FILENAME="$LOG_DIR/$CMD_BASENAME-$TIMESTAMP.log"
#CONF_FILENAME="$CONF_DIR/$CMD_BASENAME.conf"


# ---------- values for ANSI colors ----------

C0="\033[0m"
CI="\033[1;34m"
CE="\033[1;31m"


# -----------------------------------------------------------------------------
# generic functions
# -----------------------------------------------------------------------------

info ()
{
    echo -e "${CI}INFO:${C0} $1"
}

warn ()
{
    echo ""
    echo -e "${CE}WARNING:${C0} $1"
    echo ""
}

debug ()
{
    if [ "$DEBUG_MODE" = "1" ]; then
	echo "DEBUG: $1"
    fi
}

error ()
{
    echo ""
    echo -e "${CE}ERROR:${C0} $1"
    echo ""
}

fail ()
{
    error "$1"

    exit 10
}

ctrl_c()
{
    echo ""

    warn "Aborted."

    exit 20
}

line ()
{
    SEP=$1

    echo -n -e "${CI}"

    for i in {1..80}; do
	echo -n "$SEP"
    done

    echo -n -e "${C0}"
    echo ""
}

headline ()
{
    MSG=$1
    SEP=$2

    if [ "$SEP" = "" ]; then
	SEP="-"
    fi

    echo ""
    line "$SEP"
    echo -e "${CI}$MSG${C0}"
    line "$SEP"
    echo ""
}

cp_txt ()
{
    FILENAME_FROM=$1
    FILENAME_TO=$2
    FILENAME_EXT=$3

    BASENAME_FROM=`basename "$FILENAME_FROM" "$FILENAME_EXT"`
    if [ -d "$FILENAME_TO" ]; then
	FILENAME_TO="$FILENAME_TO/$BASENAME_FROM$FILENAME_EXT"
    fi

    if [ "$PLATFORM_BASE" = "win" ]; then
	perl -pe 's/$/\r/' < "$FILENAME_FROM" > "$FILENAME_TO"
    else
	$CP                  "$FILENAME_FROM"   "$FILENAME_TO"
    fi
}

read_config_value ()
{
    TOKEN=$1

    debug "- reading token '$TOKEN' from config file ..."

    CHECK=`grep "^$TOKEN\s" "$CONF_FILENAME"`

    if [ "$CHECK" = "" ]; then
	fail "Missing token '$token' in config file -- aborting!"
    fi

    CHECK=`grep "^$TOKEN\s" "$CONF_FILENAME" | awk '{ print $2 }'`

    if [ "$CHECK" != "=" ]; then
	fail "Malformed config file for '$token' -- aborting!"
    fi

    VALUE=`grep "^$TOKEN\s" "$CONF_FILENAME" | $SED_CMD -e "s/^$TOKEN\s*=\s*//"`

    if [ "$VALUE" = "" ]; then
	fail "Empty value for '$token' in config file -- aborting!"
    fi

    CONFIG[$TOKEN]=$VALUE

    debug "  * got value '$VALUE'"
}

read_config_file ()
{
    debug "Reading configuration file ..."

    for token in "${CONFIG_LIST[@]}"; do
	read_config_value "$token"
    done

    debug "Done."
    debug ""
}

cleanup ()
{
    TMP_DIR=$1

    if [ "$TMP_DIR" != "" ]; then
	if [ ! -d "$TMP_DIR" ]; then
	    fail "Temporary directory '$TMP_DIR' not found -- aborting!"
	fi

	info "Removing temporary directory ..."

	rm -rf "$TMP_DIR"
    fi

    info "Done."
    info ""
}


# -----------------------------------------------------------------------------
# usage and arguments checking
# -----------------------------------------------------------------------------

print_usage_and_exit ()
{
    echo ""
    echo "Usage: $CMD_BASENAME <action> <platform>"
    echo ""
    echo "Arguments:"
    echo ""
    echo "- <action> (mandatory) can be one of the following:"
    echo "  * build        - compile and package on local host"
    echo "  * package      - prepare and execute packaging on remote host"
    echo "  * copy-package - copy package from remote host to local host"
    echo "  * upload       - upload package to remote host"
    echo "  * deploy       - deploy package on remote host"
    echo ""
    echo "- <platform> (mandatory) can be one of the following:"
    echo "  * linux"
    echo "  * win32"
    echo "  * win64"
    echo "  * mac"
    echo "  * android"
    echo "  * emscripten"
    echo ""

    exit 10
}

check_arguments ()
{
    debug "Checking arguments ..."

    ACTION_TYPE=${ACTION_TYPE_LIST[$ACTION]}
    PLATFORM_NAME=${PLATFORM_NAME_LIST[$PLATFORM]}

    if [ "$ACTION_TYPE" = "" -o "$PLATFORM_NAME" = "" ]; then
	print_usage_and_exit
    fi

    PLATFORM_BASE=${PLATFORM_BASE_LIST[$PLATFORM]}

    if [ "$PLATFORM_BASE" = "" ]; then
	PLATFORM_BASE=$PLATFORM
    fi

    PLATFORM_PROJECT=${PLATFORM_PROJECT_LIST[$PLATFORM]}

    if [ "$PLATFORM_PROJECT" = "" ]; then
	PLATFORM_PROJECT=$PLATFORM
    fi

    debug "Arguments:"
    debug "- ACTION = '$ACTION'"
    debug "- PLATFORM = '$PLATFORM'"
    debug "Derived values from arguments:"
    debug "- ACTION_TYPE = '$ACTION_TYPE'"
    debug "- PLATFORM_BASE = '$PLATFORM_BASE'"
    debug "- PLATFORM_NAME = '$PLATFORM_NAME'"

    debug "Done."
    debug ""
}


# -----------------------------------------------------------------------------
# prepare remote packaging
# -----------------------------------------------------------------------------

check_repository_is_up_to_date ()
{
    info "Checking that repository is up-to-date ..."

    git status | grep -q "On branch $GIT_BRANCH$"

    if [ "$?" != "0" ]; then
	fail "Not on branch '$GIT_BRANCH' -- aborting!"
    fi

    git status | grep -q "Your branch is ahead of"

    if [ "$?" = "0" ]; then
	warn "Branch '$GIT_BRANCH' not up-to-date!"
	info "Continue anyway? [N/y]"
	read x

	if [ "$x" != "y" ]; then
	    fail "Aborting."
	fi
    fi
}

check_custom_edition_directory ()
{
    if [ "$IS_CUSTOM_EDITION" != "1" ]; then
	return
    fi

    info "Checking directory for custom edition ..."

    EDITION_SUBDIR="$EDITIONS_SUBDIR/$EDITION_BASENAME"

    if [ ! -d "$EDITION_SUBDIR" ]; then
	fail "Cannot find directory '$EDITION_SUBDIR'!"
    fi
}

copy_custom_edition_files_to_target_host ()
{
    info "Preparing custom files on target host ..."

    # create "BUILD" directory if it does not exist yet
    $REMOTE_CMD "mkdir -p \"$BUILD_DIR\""

    if [ "$?" != "0" ]; then
	fail "Cannot create build directory on target host!"
    fi

    # remove "NonGitStuff" directory if it already exists
    $REMOTE_CMD "rm -rf \"$BUILD_NON_GIT_DIR\""

    info "Copying custom files to target host ..."

    # copy custom files to be packaged with the distribution package
    for i in $CUSTOM_SUBDIRS; do
	SRC_BASE_SUBDIR="$EDITIONS_SUBDIR/$EDITION_BASENAME/$i"
	SRC_PLATFORM_SUBDIR="$SRC_BASE_SUBDIR.$PLATFORM"
	DST_DIR="$BUILD_NON_GIT_DIR/$i"

	COPY_BASE_SUBDIR="0"

	# copy base directory if no platform specific directory exists
	if [ ! -d "$SRC_PLATFORM_SUBDIR" ]; then
	    COPY_BASE_SUBDIR="1"
	fi

	# copy base directory if platform specific directory should be merged
	if [ -f "$SRC_PLATFORM_SUBDIR/$MERGE_CUSTOM_BASENAME" ]; then
	    COPY_BASE_SUBDIR="1"
	fi

	if [ -d "$SRC_BASE_SUBDIR" -a "$COPY_BASE_SUBDIR" = "1" ]; then
	    info "- copying $i files ..."

	    $REMOTE_CMD "mkdir -p \"$DST_DIR\""
	    $RSYNC "$SRC_BASE_SUBDIR/" "$REMOTE_AUTH:$DST_DIR"
	fi

	if [ -d "$SRC_PLATFORM_SUBDIR" ]; then
	    info "- copying $i files ($PLATFORM) ..."

	    $REMOTE_CMD "mkdir -p \"$DST_DIR\""
	    $RSYNC "$SRC_PLATFORM_SUBDIR/" "$REMOTE_AUTH:$DST_DIR"
	fi
    done
}

execute_package_script_on_target_host ()
{
    info "Copying package script to target host ..."

    $RSYNC "$CMD_FILENAME"  "$REMOTE_AUTH:$BUILD_DIR"
    $RSYNC "$CONF_FILENAME" "$REMOTE_AUTH:$BUILD_DIR"

    if [ "$?" != "0" ]; then
	fail "Cannot copy package script to target host!"
    fi

    PKG_COMMAND="$BUILD_DIR/$CMD_BASENAME build $PLATFORM"

    info "Executing package script on target host ..."

    $REMOTE_CMD "$EXTERNAL_EDITION_BASENAME $PKG_COMMAND"

    if [ "$?" != "0" ]; then
	fail "Execution of package script on target host failed!"
    fi
}


# -----------------------------------------------------------------------------
# package
# -----------------------------------------------------------------------------

package ()
{
    headline "Packaging '$EDITION_BASENAME' for $PLATFORM_NAME" "="

    check_repository_is_up_to_date
    check_custom_edition_directory
    copy_custom_edition_files_to_target_host
    execute_package_script_on_target_host
}


# -----------------------------------------------------------------------------
# build
# -----------------------------------------------------------------------------

# -----------------------------------------------------------------------------
# build / prepare
# -----------------------------------------------------------------------------

prepare_repository ()
{
    cd "$BUILD_DIR"

    if [ "$TEST_MODE" = "1" ]; then
	info "::: GIT PULL SKIPPED -- USING TEST DATA IN '$PROGRAM_GIT_SUBDIR'"

	return
    fi

    info "Cloning R'n'D repository ..."

    rm -rf "$PROGRAM_GIT_SUBDIR"
    git clone "$PROGRAM_GIT_URL"

    if [ "$?" != "0" ]; then
	fail "Cannot clone Git repository '$PROGRAM_GIT_URL'!"
    fi

    mv "$PROGRAM_BASENAME" "$PROGRAM_GIT_SUBDIR"

    info "Switching to branch '$GIT_BRANCH' ..."

    cd "$PROGRAM_GIT_SUBDIR"
    git checkout "$GIT_BRANCH"

    if [ "$?" != "0" ]; then
	fail "Cannot check out Git branch '$GIT_BRANCH'!"
    fi

    cd "$BUILD_DIR"
}

create_icon ()
{
    SIZE=$1
    SRC_FILE=$2
    DST_DIR=$3

    SIZEXSIZE="${SIZE}x${SIZE}"

    if [ "$PLATFORM" = "mac" ]; then
	DST_FILE="$DST_DIR/icon_$SIZEXSIZE.png"

	if [ ! -s "$DST_FILE" ]; then
	    info "- creating icon size $SIZEXSIZE ..."

	    sips -z $SIZE $SIZE "$SRC_FILE" --out "$DST_FILE"
	fi

	DST_FILE="$DST_DIR/icon_$SIZEXSIZE@2x.png"

	SIZE=$(($SIZE * 2))
	SIZEXSIZE="${SIZE}x${SIZE}"

	if [ ! -s "$DST_FILE" ]; then
	    info "- creating icon size $SIZEXSIZE (x2)..."

	    sips -z $SIZE $SIZE "$SRC_FILE" --out "$DST_FILE"
	fi
    else
	if [ "$PLATFORM_BASE" = "win" ]; then
	    DST_FILE="$DST_DIR/icon-$SIZEXSIZE.png"
	elif [ "$PLATFORM" = "android" ]; then
	    ANDROID_ICONSIZE=${ANDROID_ICONSIZE_LIST[$SIZE]}
	    DST_FILE="$DST_DIR/mipmap-$ANDROID_ICONSIZE/ic_launcher.png"
	elif [ "$PLATFORM" = "emscripten" ]; then
	    DST_FILE="$DST_DIR/favicon-$SIZEXSIZE.png"
	else
	    fail "Cannot create icon for plaform '$PLATFORM'!"
	fi

	if [ ! -s "$DST_FILE" ]; then
	    info "- creating icon size $SIZEXSIZE ..."

	    convert -resize $SIZEXSIZE "$SRC_FILE" "$DST_FILE"
	fi
    fi
}

build_prepare_mac ()
{
    # ---------- copy package directory for Mac version ----------

    info "Copying Mac application package directory ..."

    APP_SRC_DIR="$DIST_DIR/$BUILD_PROJECTS_DIR/$DEFAULT_PROGRAM_NAME.app"
    APP_DST_DIR="$DIST_DIR/$PROGRAM_NAME.app"

    # copy package directory (and rename it for custom editions)
    $RSYNC "$APP_SRC_DIR/" "$APP_DST_DIR/"

    CONTENTS_DIR="$DIST_DIR/$PROGRAM_NAME.app/Contents"

    PLIST_FILE="$CONTENTS_DIR/Info.plist"
    PLIST_TMPL="$PLIST_FILE.template"
    PLIST_DATE=`date +%Y`

    # replace template values (and rename program for custom editions)
    cat "$PLIST_TMPL"					\
	| sed -e "s/__VERSION__/$ARCHIVE_VERSION/g"	\
	| sed -e "s/__YEAR__/$PLIST_DATE/g"		\
	| sed -e "$SED_EXPR_BASENAME"			\
	| sed -e "$SED_EXPR_NAME"			\
	> "$PLIST_FILE"
    rm -f "$PLIST_TMPL"

    if [ "$CREATE_ICONS" = "1" ]; then
	ICNS_FILE_OLD="$CONTENTS_DIR/Resources/$DEFAULT_PROGRAM_BASENAME.icns"
	ICNS_FILE_NEW="$CONTENTS_DIR/Resources/$PROGRAM_BASENAME.icns"
	ICONSET_DIR="$CONTENTS_DIR/Resources/$PROGRAM_BASENAME.iconset"
	ICON_FILE="$DIST_DIR/$PROGRAM_ICON"

	if [ "$IS_CUSTOM_EDITION" = "1" ]; then
	    rm -f "$ICNS_FILE_OLD"
	fi

	if [ ! -s "$ICNS_FILE_NEW" ]; then
	    info "Creating icons for Mac package ..."

	    rm -f "$ICNS_FILE_OLD"

	    mkdir -p "$ICONSET_DIR"

	    for i in 16 32 128 256 512; do
		create_icon $i "$ICON_FILE" "$ICONSET_DIR"
	    done

	    iconutil -c icns "$ICONSET_DIR"

	    rm -rf "$ICONSET_DIR"
	fi
    fi
}

build_prepare_windows ()
{
    if [ "${CONFIG["edition.with_txt"]}" != "false" ]; then
	for i in $DOC_FILES; do
	    rm -f "$DIST_DIR/$i"
	    cp_txt "$BUILD_GIT_DIR/$i" "$DIST_DIR" $TEXTFILE_EXT
	done
    fi

    URL_SRC_FILE="$DIST_DIR/$BUILD_PROJECTS_DIR/$DEFAULT_PROGRAM_BASENAME.url"
    URL_DST_FILE="$DIST_DIR/$PROGRAM_BASENAME.url"

    # cp_txt "$BUILD_WINDOWS_DIR/$PROGRAM_BASENAME.url" "$DIST_DIR"
    cp_txt "$URL_SRC_FILE" "$URL_DST_FILE"

    if [ "$IS_CUSTOM_EDITION" = "1" ]; then
	sed -i -e "$SED_EXPR_BASENAME" "$URL_DST_FILE"
    fi

    if [ "$CREATE_ICONS" = "1" ]; then
	ICON_FILE="$DIST_DIR/$PROGRAM_ICON"
	ICON_DIR="$DIST_DIR/$BUILD_PROJECTS_DIR/icons"

	info "Creating icons for Windows package ..."

	for i in 16 32 48 128; do
	    create_icon $i "$ICON_FILE" "$ICON_DIR"
	done
    fi
}

build_prepare_android ()
{
    APP_DIR="$DIST_DIR/$BUILD_PROJECTS_DIR/app"

    if [ "$IS_CUSTOM_EDITION" = "1" ]; then
	JAVA_DIR="$APP_DIR/src/main/java"
	JAVA_PKG_DIR_OLD="$JAVA_DIR/org/artsoft/$DEFAULT_PROGRAM_BASENAME"
	JAVA_PKG_DIR_NEW="$JAVA_DIR/org/artsoft/$PROGRAM_BASENAME"
	JAVA_CLASS_FILE_OLD="$JAVA_PKG_DIR_NEW/$DEFAULT_PROGRAM_BASENAME.java"
	JAVA_CLASS_FILE_NEW="$JAVA_PKG_DIR_NEW/$PROGRAM_BASENAME.java"

	mv "$JAVA_PKG_DIR_OLD" "$JAVA_PKG_DIR_NEW"
	mv "$JAVA_CLASS_FILE_OLD" "$JAVA_CLASS_FILE_NEW"

	GRADLE_TMPL="$APP_DIR/build.gradle.tmpl"
	MANIFEST_TMPL="$APP_DIR/src/main/AndroidManifest.xml.tmpl"
	MAKEFILE="$DIST_DIR/$BUILD_PROJECTS_DIR/Makefile"

	for i in "$JAVA_CLASS_FILE_NEW"	\
		 "$GRADLE_TMPL"		\
		 "$MANIFEST_TMPL"	\
		 "$MAKEFILE"; do
	    sed -i -e "$SED_EXPR_BASENAME" "$i"
	done

	STRINGS_XML="$APP_DIR/src/main/res/values/strings.xml"

	sed -i -e "s/$DEFAULT_APP_NAME/$PROGRAM_NAME/g" "$STRINGS_XML"
	sed -i -e "s/'/\\\\'/g"                         "$STRINGS_XML"
    fi

    if [ "$CREATE_ICONS" = "1" ]; then
	RES_DIR="$APP_DIR/src/main/res"
	ICON_FILE="$DIST_DIR/$PROGRAM_ICON"
	ICON_DST="ic_launcher.png"

	info "Creating icons for Android package ..."

	for i in 36 48 72 96 144; do
	    create_icon $i "$ICON_FILE" "$RES_DIR"
	done
    fi
}

build_prepare_emscripten ()
{
    source "$PROJECTS_DIR/emsdk/emsdk_env.sh"
    export PATH="$EMSDK/upstream/emscripten/tools:$PATH"

    $CP "$PROGRAM_GIT_SUBDIR/$BUILD_PROJECTS_DIR"/* "$DIST_DIR"

    if [ "$CREATE_ICONS" = "1" ]; then
	ICON_FILE="$DIST_DIR/$PROGRAM_ICON"

	info "Creating icons for Emscripten package ..."

	for i in 16 32; do
	    create_icon $i "$ICON_FILE" "$DIST_DIR"
	done
    fi
}

build_prepare_libraries ()
{
    if [ ! -d "$SHARED_LIBS_DIR" ]; then
	fail "Cannot find required libraries directory '$SHARED_LIBS_DIR' -- aborting packaging process!"
    fi

    $CP_ARCHIVE "$SHARED_LIBS_DIR"/* "$DIST_DIR/$ARCHIVE_LIB_SUBDIR"
}


# -----------------------------------------------------------------------------
# build / finalize
# -----------------------------------------------------------------------------

build_finalize_mac__set_library_paths ()
{
    PROGRAM_FILE=$1

    SET_LIB_PATH="install_name_tool -add_rpath"
    SET_DEP_NAME="install_name_tool -change"

    LIB_PATH_OLD="/usr/local/lib"
    # LIB_PATH_NEW="@executable_path/lib"
    LIB_PATH_NEW="@executable_path/../Frameworks"

    $SET_LIB_PATH "$LIB_PATH_NEW" "$PROGRAM_FILE"

    DEP_LIBS=`otool -L "$PROGRAM_FILE"			\
	| grep "$LIB_PATH_OLD"				\
	| awk '{ print $1 }'`

    for i in $DEP_LIBS; do
	LIB_NAME_OLD="$i"
	LIB_NAME_NEW=`echo "$LIB_NAME_OLD"		\
            | sed -e "s/[-\.].*$/.dylib/"		\
            | sed -e "s%$LIB_PATH_OLD%$LIB_PATH_NEW%"`

	LIB_BASENAME_NEW=`basename "$LIB_NAME_NEW"`

	$SET_DEP_NAME "$LIB_NAME_OLD" "$LIB_NAME_NEW" "$PROGRAM_FILE"
    done
}

build_finalize_mac__codesign_bundle ()
{
    BUNDLE_DIR=$1

    CMD_CODESIGN="codesign --force --deep --sign -"

    for i in "$BUNDLE_DIR/Contents/Frameworks"/*.framework; do
	$CMD_CODESIGN "$i"
    done

    $CMD_CODESIGN "$BUNDLE_DIR"
}

build_finalize_mac ()
{
    # build_finalize_mac__set_library_paths "$PROGRAM_BINARY"

    mv "$PROGRAM_BINARY" "$PROGRAM_NAME.app/Contents/MacOS"
    mv * "$PROGRAM_NAME.app/Contents/Resources"
    touch "$PROGRAM_NAME.app"

    build_finalize_mac__codesign_bundle "$PROGRAM_NAME.app"
}

build_finalize_emscripten ()
{
    if [ "$IS_CUSTOM_EDITION" = "1" ]; then
	PREFIX_OLD="$DIST_DIR/$PROGRAM_BASENAME_BUILD"
	PREFIX_NEW="$DIST_DIR/$PROGRAM_BASENAME"

	mv "$PREFIX_OLD.data"    "$PREFIX_NEW.data"
	mv "$PREFIX_OLD.data.js" "$PREFIX_NEW.data.js"
	mv "$PREFIX_OLD.js"      "$PREFIX_NEW.js"
	mv "$PREFIX_OLD.wasm"    "$PREFIX_NEW.wasm"

	sed -i -e "$SED_EXPR_BASENAME" "$PREFIX_NEW.js"
	sed -i -e "$SED_EXPR_BASENAME" "$PREFIX_NEW.data.js"

	INDEX_FILE="$DIST_DIR/index.html"

	sed -i -e "$SED_EXPR_BASENAME" "$INDEX_FILE"
	sed -i -e "$SED_EXPR_NAME"     "$INDEX_FILE"
    fi
}

# -----------------------------------------------------------------------------
# build / package
# -----------------------------------------------------------------------------

build_package_generic ()
{
    tar -c -z -C "$TEMP_DIR" -f "$PACKAGE_DIR/$ARCHIVE_FILE" "$ARCHIVE_DIR"
}

build_package_linux ()
{
    build_package_generic
}

build_package_mac ()
{
    IMAGE_DIR="$PROGRAM_NAME $ARCHIVE_VERSION"
    IMAGE_FILE="$PACKAGE_DIR/$IMAGE_DIR.dmg"
    IMAGE_FILE_FINAL="$PACKAGE_DIR/$ARCHIVE_FILE"
    INFO_TEXT_FILE="To install, just copy the game to your hard disk."

    # calculate required disk image size in steps of 2 MB (plus 4 extra MB)
    IMAGE_SIZE_MB=`du -sk . | awk '{ print int(($1 / 1024 + 2) / 2) * 2 + 4 }'`

    if [ -e "$IMAGE_DIR/$IMAGE_DIR" ]; then
	fail "Image directory structure already exists!"
    fi

    if [ -e "$IMAGE_FILE" ]; then
	fail "Image file '$IMAGE_FILE' already exists."
    fi

    if [ -e "$IMAGE_FILE_FINAL" ]; then
	IMAGE_FILE_FINAL_OLD=$IMAGE_FILE_FINAL.$DIST_DATE

	warn "Image file '$IMAGE_FILE_FINAL' already exists."
	info "Backing up old image file to '$IMAGE_FILE_FINAL_OLD'."

	mv "$IMAGE_FILE_FINAL" "$IMAGE_FILE_FINAL_OLD"
    fi

    info "Creating new $IMAGE_SIZE_MB MB sized disk image file ..."
    hdiutil create -megabytes $IMAGE_SIZE_MB "$IMAGE_FILE" -layout NONE

    info "Associating device with disk image file (without mounting it) ..."
    IMAGE_DEV=$(hdid -nomount "$IMAGE_FILE")

    info "Creating HFS file system in disk image file ..."
    newfs_hfs -v "$IMAGE_DIR" $IMAGE_DEV

    info "Disassociating device from disk image file ..."
    hdiutil eject $IMAGE_DEV

    info "Mounting disk image file ..."
    MOUNT_INFO=$(hdid "$IMAGE_FILE")

    MOUNT_DIR=`echo "$MOUNT_INFO" | sed -e "s%/dev/disk.[[:space:]]*%%"`

    if [ "$MOUNT_DIR" != "/Volumes/$IMAGE_DIR" ]; then
	warn "Problems mounting disk image file:"
	info "- expected  mount point: '/Volumes/$IMAGE_DIR'"
	info "- effective mount point: '$MOUNT_DIR'"

	hdiutil eject $IMAGE_DEV

	fail "Mounting disk image file '$IMAGE_FILE' failed!"
    fi

    info "Copying program package to mounted disk image ..."
    cp -a "$PROGRAM_NAME.app" "/Volumes/$IMAGE_DIR"
    touch "/Volumes/$IMAGE_DIR/$INFO_TEXT_FILE"

    info "Cleaning up disk image ..."
    rm -rf "/Volumes/$IMAGE_DIR/.fseventsd"

    info "Ejecting disk image ..."
    hdiutil eject $IMAGE_DEV

    info "Creating compressed and read-only disk image file ..."
    hdiutil convert -format UDZO "$IMAGE_FILE" -o "$IMAGE_FILE_FINAL"

    info "Removing old (uncompressed, read/write) disk image file ..."
    rm -f "$IMAGE_FILE"
}

build_package_windows_installer ()
{
    headline "Creating installer file '$SETUP_EXE_FILE'"

    cd $TEMP_DIR

    INNOSETUP_PATH="$HOME_DIR/$PROJECTS_SUBDIR/Inno Setup 5"
    INNOSETUP_BIN="$INNOSETUP_PATH/ISCC.exe"
    INNOSETUP_CONFIG_TMPL_FILE="$BUILD_GIT_DIR/$BUILD_PROJECTS_DIR/template.iss"
    INNOSETUP_CONFIG_FILE="$PROGRAM_BASENAME.iss"
    INNOSETUP_OUTPUT_BASENAME=`basename "$SETUP_EXE_FILE" .exe`

    WIN_DIST_DIR=`echo "$DIST_DIR" | sed -e 's%/%\\\\\\\\%g'`

    PROGRAM_ARCH=""
    if [ "$PLATFORM" = "win64" ]; then
	PROGRAM_ARCH="x64"
    fi

    cat "$INNOSETUP_CONFIG_TMPL_FILE"				\
	| sed -e "s/_PRG_BASENAME_/$PROGRAM_BASENAME/g"		\
	| sed -e "s/_PRG_NAME_/$PROGRAM_NAME/g"			\
	| sed -e "s/_PRG_VERSION_/$ARCHIVE_VERSION/g"		\
	| sed -e "s/_PRG_EXE_/$PROGRAM_BINARY/g"		\
	| sed -e "s%_PRG_DIR_%$WIN_DIST_DIR%g"			\
	| sed -e "s%_PRG_ARCH_%$PROGRAM_ARCH%g"			\
	| sed -e "s/_SETUP_EXE_/$INNOSETUP_OUTPUT_BASENAME/g"	\
	> $INNOSETUP_CONFIG_FILE

    wine "$INNOSETUP_BIN" "$INNOSETUP_CONFIG_FILE" > inno_setup.log

    if [ "$?" != "0" ]; then
	fail "Creating Windows installer failed -- aborting packaging process!"
    fi
}

build_package_windows ()
{
    (cd "$TEMP_DIR" ; zip -q -r -y "$PACKAGE_DIR/$ARCHIVE_FILE" "$ARCHIVE_DIR")

    build_package_windows_installer

    mv -f "$SETUP_EXE_FILE" "$PACKAGE_DIR"
}

build_package_android ()
{
    # packaging already done by Android build script -- only copy package here

    $CP "$PACKAGE_DIR_RAW/$ARCHIVE_FILE_RAW" "$PACKAGE_DIR/$ARCHIVE_FILE"
}

build_package_emscripten ()
{
    build_package_generic
}


# -----------------------------------------------------------------------------
# build / main
# -----------------------------------------------------------------------------

build ()
{
    # -------------------------------------------------------------------------
    # create distribution package
    # -------------------------------------------------------------------------

    headline "Building '$PROGRAM_BASENAME' version $ARCHIVE_VERSION"

    DIST_BUILD_PLATFORM="dist-build-$PLATFORM"

    TEMP_DIR="$BUILD_DIR/TMP-distribution-$PLATFORM-$$"
    DIST_DIR="$TEMP_DIR/$ARCHIVE_DIR"

    mkdir -p "$DIST_DIR"
    mkdir -p "$DIST_DIR/$ARCHIVE_LIB_SUBDIR"

    # default: no global "scores" directory => save scores in user data directory
    # mkdir -p "$DIST_DIR/scores"

    cp -a "$PROGRAM_GIT_SUBDIR"/[A-Za-z0-9]* "$DIST_DIR"
    cp -a "$PROGRAM_GIT_SUBDIR"/.git*        "$DIST_DIR"

    # remove certain standard package files before copying custom package files
    if [ "$IS_CUSTOM_EDITION" = "1" ]; then
	for i in $CUSTOM_SUBDIRS; do
	    if [ -d "$BUILD_NON_GIT_DIR/$i" ]; then
		if [ -f "$BUILD_NON_GIT_DIR/$i/$MERGE_ORIGINAL_BASENAME" ]; then
		    rm -f "$BUILD_NON_GIT_DIR/$i/$MERGE_ORIGINAL_BASENAME"
		else
		    rm -rf "$DIST_DIR/$i"
		fi

		rm -f "$BUILD_NON_GIT_DIR/$i/$MERGE_CUSTOM_BASENAME"
	    fi
	done
    fi

    if [ -d "$BUILD_NON_GIT_DIR" ]; then
	$RSYNC "$BUILD_NON_GIT_DIR/" "$DIST_DIR/"
    fi

    if [ "$PLATFORM" = "mac" ]; then
	build_prepare_mac
    elif [ "$PLATFORM_BASE" = "win" ]; then
	build_prepare_windows
    elif [ "$PLATFORM" = "android" ]; then
	build_prepare_android
    fi

    if [ "$PLATFORM" = "emscripten" ]; then
	build_prepare_emscripten
    else
	build_prepare_libraries
    fi

    cd "$DIST_DIR"

    if [ "$PLATFORM" = "mac" ]; then
	export PATH=$PATH:$BIN_DIR_MAC
    fi

    $MAKE dist-clean
    $MAKE $DIST_BUILD_PLATFORM

    if [ "$?" != "0" ]; then
	fail "Compilation failed -- aborting packaging process!"
    fi

    $MAKE dist-clean

    if [ "$PLATFORM" = "android" ]; then
	headline "Creating package file '$ARCHIVE_FILE'"

	build_package_android

	cleanup "$TEMP_DIR"

	exit 0
    fi

    rm -rf "$DIST_DIR"/.git*
    rm -f  "$DIST_DIR/$GFX_CLASSIC_SUBDIR"/Makefile
    rm -f  "$DIST_DIR/$GFX_CLASSIC_SUBDIR"/../Makefile
    rm -f  "$DIST_DIR/src/Android.mk"
    rm -f  "$DIST_DIR/src/header.tmpl"

    if [ "${CONFIG["edition.with_src"]}" = "false" ]; then
	rm -rf "$DIST_DIR/src"
	rm -rf "$DIST_DIR/build-projects"
	rm -rf "$DIST_DIR/build-scripts"
	rm -f  "$DIST_DIR/Makefile"
    fi

    if [ "${CONFIG["edition.with_txt"]}" = "false" ]; then
	for i in $DOC_FILES; do
	    rm -f "$DIST_DIR/$i"
	done
    fi

    if [ "$IS_CUSTOM_EDITION" = "1" -a "$PLATFORM" != "emscripten" ]; then
	PROGRAM_BINARY_BUILD="$PROGRAM_BASENAME_BUILD$BINARY_EXT"

	mv "$PROGRAM_BINARY_BUILD" "$PROGRAM_BINARY"
    fi

    if [ "$PLATFORM" = "mac" ]; then
	build_finalize_mac
    elif [ "$PLATFORM" = "emscripten" ]; then
	build_finalize_emscripten
    fi

    # ---------- create archive file(s) for target platform ----------

    headline "Creating package file '$ARCHIVE_FILE'"

    mkdir -p "$PACKAGE_DIR"
    rm -f "$PACKAGE_DIR/$ARCHIVE_FILE"

    if [ "$PLATFORM_BASE" = "win" ]; then
	rm -f "$PACKAGE_DIR/$SETUP_EXE_FILE"
    fi

    if [ "$PLATFORM" = "linux" ]; then
	build_package_linux
    elif [ "$PLATFORM" = "emscripten" ]; then
	build_package_emscripten
    elif [ "$PLATFORM_BASE" = "win" ]; then
	build_package_windows
    elif [ "$PLATFORM" = "mac" ]; then
	build_package_mac
    fi

    cleanup "$TEMP_DIR"
}


# -----------------------------------------------------------------------------
# copy-package
# -----------------------------------------------------------------------------

copy_package ()
{
    if [ -f "$RELEASE_DIR/$ARCHIVE_FILE" ]; then
	fail "Release package '$ARCHIVE_FILE' already exists!"
    fi

    info "Copying package for $PLATFORM_NAME to local release directory"

    RELEASE_FILE="$RELEASE_DIR/$ARCHIVE_FILE"
    SHA256_FILE="$RELEASE_FILE.sha256"

    $SCP_CMD "$REMOTE_AUTH:$PACKAGE_DIR/$ARCHIVE_FILE" "$RELEASE_FILE"
    (cd "$RELEASE_DIR" && $SHA256_CMD "$ARCHIVE_FILE") > "$SHA256_FILE"

    if [ "$PLATFORM_BASE" = "win" ]; then
	RELEASE_FILE="$RELEASE_DIR/$SETUP_EXE_FILE"
	SHA256_FILE="$RELEASE_FILE.sha256"

	$SCP_CMD "$REMOTE_AUTH:$PACKAGE_DIR/$SETUP_EXE_FILE" "$RELEASE_FILE"
	(cd "$RELEASE_DIR" && $SHA256_CMD "$SETUP_EXE_FILE") > "$SHA256_FILE"
    fi

    cleanup
}

upload ()
{
    REMOTE_HOST=${CONFIG["upload.releases.hostname"]}
    REMOTE_USER=${CONFIG["upload.releases.username"]}
    REMOTE_PATH=${CONFIG["upload.releases.path"]}

    REMOTE_AUTH="$REMOTE_USER@$REMOTE_HOST"
    REMOTE_DIR="$REMOTE_PATH/$UPLOAD_SUBDIR"

    info "Uploading distribution package for $PLATFORM_NAME"

    RELEASE_FILE="$RELEASE_DIR/$ARCHIVE_FILE"
    SHA256_FILE="$RELEASE_FILE.sha256"

    $SCP_CMD $RELEASE_FILE $REMOTE_AUTH:$REMOTE_DIR
    $SCP_CMD $SHA256_FILE  $REMOTE_AUTH:$REMOTE_DIR

    if [ "$PLATFORM_BASE" = "win" ]; then
	RELEASE_FILE="$RELEASE_DIR/$SETUP_EXE_FILE"
	SHA256_FILE="$RELEASE_FILE.sha256"

	$SCP_CMD $RELEASE_FILE $REMOTE_AUTH:$REMOTE_DIR
	$SCP_CMD $SHA256_FILE  $REMOTE_AUTH:$REMOTE_DIR
    fi

    cleanup
}

deploy ()
{
    REMOTE_HOST=${CONFIG["deploy.$PLATFORM.hostname"]}
    REMOTE_USER=${CONFIG["deploy.$PLATFORM.username"]}
    REMOTE_PATH=${CONFIG["deploy.$PLATFORM.path"]}

    REMOTE_AUTH="$REMOTE_USER@$REMOTE_HOST"

    TEMP_DIR="$LOCAL_BUILD_DIR/TMP-deployment-$PLATFORM-$$"
    TARGET_DIR="$PROGRAM_BASENAME"
    DEPLOY_DIR="$TARGET_DIR/play-$ARCHIVE_VERSION"

    headline "Deploying distribution package for $PLATFORM_NAME"

    mkdir -p "$TEMP_DIR"
    cd "$TEMP_DIR"

    if [ ! -f "$RELEASE_DIR/$ARCHIVE_FILE" ]; then
	fail "Cannot find file '$RELEASE_DIR/$ARCHIVE_FILE'!"
    fi

    info "Extracting archive for deployment ..."

    tar xzf "$RELEASE_DIR/$ARCHIVE_FILE"

    info "Preparing target directory '$DEPLOY_DIR' ..."

    mkdir -p "$DEPLOY_DIR"
    cp -a "$ARCHIVE_BASENAME"/index.html          "$DEPLOY_DIR"
    cp -a "$ARCHIVE_BASENAME"/favicon-*.png       "$DEPLOY_DIR"
    cp -a "$ARCHIVE_BASENAME"/loading.svg         "$DEPLOY_DIR"
    cp -a "$ARCHIVE_BASENAME"/$PROGRAM_BASENAME.* "$DEPLOY_DIR"

    info "Deploying target directory '$DEPLOY_DIR' ..."

    $RSYNC "$TARGET_DIR/" "$REMOTE_AUTH:$REMOTE_PATH/$TARGET_DIR/"

    cleanup "$TEMP_DIR"
}


# -----------------------------------------------------------------------------
# main
# -----------------------------------------------------------------------------

trap ctrl_c INT

DEFAULT_PROGRAM_BASENAME="rocksndiamonds"
DEFAULT_PROGRAM_NAME="Rocks'n'Diamonds"
DEFAULT_APP_NAME="Rocks \\\\'n\\\\' Diamonds"

HOME_BASEDIR="$HOME_BASEDIR_DEFAULT"
LOCAL_HOME_BASEDIR="$HOME_BASEDIR_DEFAULT"

if [ "$PLATFORM" = "mac" ]; then
    HOME_BASEDIR="$HOME_BASEDIR_MAC"
fi

# if executed on a Mac, do some special checks and preparations
if [ `uname` = "Darwin" ]; then
    # first, try to find correct directory for required GNU binaries
    for dir in $BIN_DIR_LIST_MAC; do
        if [ -f "$dir/bash" ]; then
            BIN_DIR_MAC="$dir"

            break
        fi
    done

    BASH_CMD="$BIN_DIR_MAC/bash"
    SED_CMD="$BIN_DIR_MAC/gsed"

    # prevent using outdated default Bash version on Mac
    if [ `echo $BASH_VERSION | cut -c 1` = "3" ]; then
	info "Outdated Bash version 3 detected on `hostname -s` -- checking for newer one..."

        if [ `$BASH_CMD --version | head -1 | sed "s/.* version //" | cut -c 1` = "3" ]; then
	    fail "Cannot find newer Bash version -- aborting!"
        fi

        # restart script using newer custom Bash version
        $BASH_CMD $0 $*

        exit $?
    fi

    LOCAL_HOME_BASEDIR="$HOME_BASEDIR_MAC"
fi

DEBUG_MODE=$DEBUG_MODE_INITIAL

declare -A ACTION_TYPE_LIST=\
(				\
    ["build"]="remote"		\
    ["package"]="remote"	\
    ["copy-package"]="local"	\
    ["upload"]="local"		\
    ["deploy"]="local"		\
)

declare -A PLATFORM_NAME_LIST=\
(					\
    ["linux"]="Linux"			\
    ["win32"]="Windows (32-bit)"	\
    ["win64"]="Windows (64-bit)"	\
    ["mac"]="Mac"			\
    ["android"]="Android"		\
    ["emscripten"]="Emscripten"		\
)

declare -A PLATFORM_BASE_LIST=\
(			\
    ["win32"]="win"	\
    ["win64"]="win"	\
)

declare -A PLATFORM_PROJECT_LIST=\
(			\
    ["win32"]="windows"	\
    ["win64"]="windows"	\
)

declare -A ANDROID_ICONSIZE_LIST=\
(			\
    ["36"]="ldpi"	\
    ["48"]="mdpi"	\
    ["72"]="hdpi"	\
    ["96"]="xhdpi"	\
    ["144"]="xxhdpi"	\
)

declare -A CONFIG
declare -a CONFIG_LIST=\
(				\
    "build.$PLATFORM.hostname"	\
    "build.$PLATFORM.username"	\
    "build.git.hostname"	\
    "build.git.username"	\
    "build.git.branch"		\
    "mode.debug"		\
    "mode.test"			\
    "edition.basename"		\
    "edition.with_src"		\
    "edition.with_txt"		\
)
declare -a CONFIG_LIST_DEPLOY=\
(				\
    "deploy.$PLATFORM.hostname"	\
    "deploy.$PLATFORM.username"	\
    "deploy.$PLATFORM.path"	\
)
declare -a CONFIG_LIST_UPLOAD=\
(				\
    "upload.releases.hostname"	\
    "upload.releases.username"	\
    "upload.releases.path"	\
)

check_arguments

if [ "$ACTION" = "deploy" ]; then
    CONFIG_LIST+=("${CONFIG_LIST_DEPLOY[@]}")
fi

if [ "$ACTION" = "upload" ]; then
    CONFIG_LIST+=("${CONFIG_LIST_UPLOAD[@]}")
fi

read_config_file

HOSTNAME=${CONFIG["build.$PLATFORM.hostname"]}
USERNAME=${CONFIG["build.$PLATFORM.username"]}

GIT_HOSTNAME=${CONFIG["build.git.hostname"]}
GIT_USERNAME=${CONFIG["build.git.username"]}
GIT_BRANCH=${CONFIG["build.git.branch"]}

DEBUG_MODE=${CONFIG["mode.debug"]}
TEST_MODE=${CONFIG["mode.test"]}

REMOTE_AUTH="$USERNAME@$HOSTNAME"
REMOTE_CMD="$SSH_CMD $REMOTE_AUTH"

LOCAL_CURRENT_DIR=`pwd`
LOCAL_HOME_DIR=`(cd && pwd)`
REMOTE_HOME_DIR=`$REMOTE_CMD pwd`

if [ "$EDITION_BASENAME" != "" ]; then
    info "Using externally defined custom edition '$EDITION_BASENAME'."

    EXTERNAL_EDITION_BASENAME="EDITION_BASENAME=$EDITION_BASENAME"
else
    EDITION_BASENAME=${CONFIG["edition.basename"]}
fi

if [ "$EDITION_BASENAME" == "" ]; then
    EDITION_BASENAME=$DEFAULT_PROGRAM_BASENAME
fi

if [ "$EDITION_BASENAME" != "$DEFAULT_PROGRAM_BASENAME" ]; then
    IS_CUSTOM_EDITION="1"
    EDITION_BASENAME_CHECK=`echo "$EDITION_BASENAME" | sed "s/[^a-z0-9_]//g"`

    if [ "$EDITION_BASENAME_CHECK" != "$EDITION_BASENAME" ]; then
	fail "Invalid custom edition basename '$EDITION_BASENAME'!"
    fi
fi

LOCAL_USERNAME=$USER

if [ "$ACTION" = "upload" -o "$ACTION" = "deploy" ]; then
    USERNAME="$LOCAL_USERNAME"
    HOME_BASEDIR="$HOME_BASEDIR_DEFAULT"
fi

if [ "$ACTION" = "copy-package" -o "$ACTION" = "upload" -o "$ACTION" = "deploy" ]; then
    LOCAL_HOME_DIR="$LOCAL_HOME_BASEDIR/$LOCAL_USERNAME"
fi

HOME_DIR="$HOME_BASEDIR/$USERNAME"


# -----------------------------------------------------------------------------
# build specific definitions
# -----------------------------------------------------------------------------

PROGRAM_BASENAME="$DEFAULT_PROGRAM_BASENAME"
PROGRAM_NAME="$DEFAULT_PROGRAM_NAME"

PROJECTS_SUBDIR="projects"
RELEASE_SUBDIR="$PROJECTS_SUBDIR/RELEASES/incoming"
PACKAGE_SUBDIR="Packages"
SHARED_LIBS_SUBDIR="$PROJECTS_SUBDIR/libs/SDL2"

MERGE_ORIGINAL_BASENAME=".merge_with_original"
MERGE_CUSTOM_BASENAME=".merge_with_custom"

CREATE_ICONS="1"

if [ "$PLATFORM_BASE" = "win" -o "$PLATFORM_BASE" = "linux" ]; then
    SHARED_LIBS_SUBDIR="$SHARED_LIBS_SUBDIR/$PLATFORM"
fi

PROGRAM_GIT_SUBDIR="$PROGRAM_BASENAME.git"
if [ "$TEST_MODE" = "1" ]; then
    PROGRAM_GIT_SUBDIR="$PROGRAM_GIT_SUBDIR.TEST"
fi

PROGRAM_GIT_URL="$GIT_USERNAME@$GIT_HOSTNAME:~/$PROGRAM_GIT_SUBDIR"

PROJECTS_DIR="$HOME_DIR/$PROJECTS_SUBDIR"
RELEASE_DIR="$LOCAL_HOME_DIR/$RELEASE_SUBDIR"
SHARED_LIBS_DIR="$HOME_DIR/$SHARED_LIBS_SUBDIR"

NON_GIT_SUBDIR="NonGitStuff"
EDITIONS_SUBDIR="Special/Editions"
GFX_CLASSIC_SUBDIR="graphics/gfx_classic"
CUSTOM_SUBDIRS="levels graphics sounds music conf docs build-projects"

PROGRAM_DIR="$PROJECTS_DIR/$PROGRAM_BASENAME"

BUILD_DIR="$PROGRAM_DIR/BUILD"
BUILD_GIT_DIR="$BUILD_DIR/$PROGRAM_GIT_SUBDIR"
BUILD_NON_GIT_DIR="$BUILD_DIR/$NON_GIT_SUBDIR"

PACKAGE_DIR="$BUILD_DIR/$PACKAGE_SUBDIR"

BASE_DIR="$BUILD_GIT_DIR"


# -----------------------------------------------------------------------------
# copy build script to target host and execute it there
# -----------------------------------------------------------------------------

if [ "$ACTION" = "package" ]; then
    package

    exit 0
fi


# -----------------------------------------------------------------------------
# set up target build environment
# -----------------------------------------------------------------------------

if [ "$ACTION" = "build" ]; then
    prepare_repository
fi


# -----------------------------------------------------------------------------
# build specific definitions
# -----------------------------------------------------------------------------

if [ "$ACTION" = "copy-package" -o "$ACTION" = "upload" -o "$ACTION" = "deploy" ]; then
    if [ ! -d "$RELEASE_DIR" ]; then
	fail "Cannot find directory '$RELEASE_DIR'!"
    fi

    BASE_DIR="$LOCAL_CURRENT_DIR"
    LOCAL_BUILD_DIR="$LOCAL_CURRENT_DIR/BUILD"
fi

MAIN_H="$BASE_DIR/src/main.h"

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
    fail "Program version number ('PROGRAM_VERSION_SUPER') not unique!"
fi

UNIQUE_VERSION=`echo "$VERSION_EXTRA" | wc -l | awk '{ print $1 }'`
if [ "$UNIQUE_VERSION" != "1" ]; then
    fail "Program version number ('PROGRAM_VERSION_EXTRA') not unique!"
fi

ARCHIVE_VERSION="$VERSION_SUPER.$VERSION_MAJOR.$VERSION_MINOR.$VERSION_PATCH"

if [ "$VERSION_EXTRA" != "0" ]; then
    ARCHIVE_VERSION="$ARCHIVE_VERSION-$VERSION_EXTRA_TEXT-$VERSION_EXTRA"
fi

PROGRAM_ICON_DEFAULT=`grep "#define PROGRAM_ICON_FILENAME" $MAIN_H	\
    | awk -F\" '{ print $2 }'`
PROGRAM_ICON="graphics/gfx_classic/$PROGRAM_ICON_DEFAULT"

if [ "$IS_CUSTOM_EDITION" = "1" ]; then
    if [ "$ACTION" = "build" -o "$ACTION" = "package" ]; then
	MAIN_PATH="$BUILD_NON_GIT_DIR"
    else
	MAIN_PATH="$EDITIONS_SUBDIR/$EDITION_BASENAME"
    fi

    CONF_FILE="$MAIN_PATH/conf/setup.conf"

    if [ ! -f "$CONF_FILE" ]; then
	fail "Cannot find custom config file for '$EDITION_BASENAME'!"
    fi

    PROGRAM_BASENAME_BUILD=$PROGRAM_BASENAME
    PROGRAM_BASENAME=$EDITION_BASENAME

    PROGRAM_NAME=`grep "^program_title" $CONF_FILE	\
	| head -1					\
	| awk -F\: '{ print $2 }'			\
	| sed -e "s/^[[:space:]]*//"			\
	| sed -e "s/[[:space:]]*$//"`

    if [ "$PROGRAM_NAME" = "" ]; then
	fail "Cannot determine program name for '$EDITION_BASENAME'!"
    fi

    SED_EXPR_BASENAME="s/$DEFAULT_PROGRAM_BASENAME/$PROGRAM_BASENAME/g"
    SED_EXPR_NAME="s/$DEFAULT_PROGRAM_NAME/$PROGRAM_NAME/g"

    GRAPHICS_SET=`grep "^graphics_set" $CONF_FILE	\
	| head -1					\
	| awk -F\: '{ print $2 }'			\
	| sed -e "s/^[[:space:]]*//"			\
	| sed -e "s/[[:space:]]*$//"`

    if [ "$GRAPHICS_SET" = "" ]; then
	fail "Cannot determine custom graphics set for '$EDITION_BASENAME'!"
    fi

    PROGRAM_ICON=`grep "^program_icon_file" $CONF_FILE	\
	| head -1					\
	| awk -F\: '{ print $2 }'			\
	| sed -e "s/^[[:space:]]*//"			\
	| sed -e "s/[[:space:]]*$//"`

    if [ "$PROGRAM_ICON" = "" ]; then
	PROGRAM_ICON=$PROGRAM_ICON_DEFAULT
    fi

    # check for icon in both graphics set and main graphics directory
    PROGRAM_ICON_TEST_1="graphics/$GRAPHICS_SET/$PROGRAM_ICON"
    PROGRAM_ICON_TEST_2="graphics/$PROGRAM_ICON"

    if [ -f "$MAIN_PATH/$PROGRAM_ICON_TEST_1" ]; then
	PROGRAM_ICON=$PROGRAM_ICON_TEST_1
    elif [ -f "$MAIN_PATH/$PROGRAM_ICON_TEST_2" ]; then
	PROGRAM_ICON=$PROGRAM_ICON_TEST_2
    else
	fail "Cannot find program icon file for '$EDITION_BASENAME'!"
    fi

    if [ "$ACTION" = "build" -o "$ACTION" = "package" ]; then
	PROGRAM_ICON_LARGE=`echo "$PROGRAM_ICON" | sed -e "s/\./_large./"`

	if [ -f "$MAIN_PATH/$PROGRAM_ICON_LARGE" ]; then
	    PROGRAM_ICON=$PROGRAM_ICON_LARGE

	    info "Found large program icon ..."
	fi
    fi

    ARCHIVE_VERSION=`grep "^program_version" $CONF_FILE	\
	| head -1					\
	| awk -F\: '{ print $2 }'			\
	| sed -e "s/^[[:space:]]*//"			\
	| sed -e "s/[[:space:]]*$//"`

    if [ "$ARCHIVE_VERSION" = "" ]; then
	fail "Cannot determine archive version for '$EDITION_BASENAME'!"
    fi
fi

# DOC_FILES="COPYING CREDITS INSTALL README ChangeLog"
DOC_FILES="COPYING CREDITS INSTALL ChangeLog"
TEXTFILE_EXT=".txt"
BINARY_EXT=""


# -----------------------------------------------------------------------------
# main actions
# -----------------------------------------------------------------------------

ARCHIVE_BASENAME="$PROGRAM_BASENAME-$ARCHIVE_VERSION"
ARCHIVE_DIR="$ARCHIVE_BASENAME"

PROGRAM_BINARY="$PROGRAM_BASENAME"

BUILD_PROJECTS_DIR="build-projects/$PLATFORM_PROJECT"

if [ "$PLATFORM" = "linux" ]; then
    ARCHIVE_LIB_SUBDIR="lib"
    ARCHIVE_FILE="$ARCHIVE_BASENAME-$PLATFORM.tar.gz"
    TEXTFILE_EXT=""
elif [ "$PLATFORM" = "win32" ]; then
    ARCHIVE_LIB_SUBDIR="."
    ARCHIVE_FILE="$ARCHIVE_BASENAME-$PLATFORM.zip"
    ARCHIVE_DIR="$PROGRAM_NAME $ARCHIVE_VERSION"
    SETUP_EXE_FILE="$ARCHIVE_BASENAME-$PLATFORM-setup.exe"
    BINARY_EXT=".exe"
    PROGRAM_BINARY="$PROGRAM_BASENAME$BINARY_EXT"
elif [ "$PLATFORM" = "win64" ]; then
    ARCHIVE_LIB_SUBDIR="."
    ARCHIVE_FILE="$ARCHIVE_BASENAME-$PLATFORM.zip"
    ARCHIVE_DIR="$PROGRAM_NAME $ARCHIVE_VERSION"
    SETUP_EXE_FILE="$ARCHIVE_BASENAME-$PLATFORM-setup.exe"
    BINARY_EXT=".exe"
    PROGRAM_BINARY="$PROGRAM_BASENAME$BINARY_EXT"
elif [ "$PLATFORM" = "mac" ]; then
    ARCHIVE_LIB_SUBDIR="$PROGRAM_NAME.app/Contents/Frameworks"
    ARCHIVE_FILE="$ARCHIVE_BASENAME-$PLATFORM.dmg"
elif [ "$PLATFORM" = "android" ]; then
    ARCHIVE_LIB_SUBDIR="."
    ARCHIVE_FILE="$ARCHIVE_BASENAME-$PLATFORM.apk"
    ARCHIVE_FILE_RAW="app-debug.apk"
    PACKAGE_DIR_RAW="$BUILD_PROJECTS_DIR/app/build/outputs/apk/debug"
elif [ "$PLATFORM" = "emscripten" ]; then
    ARCHIVE_LIB_SUBDIR="."
    ARCHIVE_FILE="$ARCHIVE_BASENAME-$PLATFORM.tar.gz"
else
    fail "Unknown platform '$PLATFORM' -- this should not happen. :-("
fi

UPLOAD_SUBDIR="$PLATFORM/$PROGRAM_BASENAME"

if [ "$ACTION" = "build" ]; then
    build
elif [ "$ACTION" = "copy-package" ]; then
    copy_package
elif [ "$ACTION" = "upload" ]; then
    upload
elif [ "$ACTION" = "deploy" ]; then
    deploy
fi

exit 0
