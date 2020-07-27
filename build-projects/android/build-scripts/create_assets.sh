#!/bin/bash

ASSETS_SRC_PATH="../.."
ASSETS_SRC_SUBDIRS="conf docs graphics levels music sounds"

ASSETS_DST_PATH="app/src/main/assets"

if [ -d "$ASSETS_DST_PATH" ]; then
    exit 0
fi

mkdir -p "$ASSETS_DST_PATH"

for i in $ASSETS_SRC_SUBDIRS; do
    ASSETS_SRC_DIR="$ASSETS_SRC_PATH/$i"
    ASSETS_DST_DIR="$ASSETS_DST_PATH/$i"

    if [ -d "$ASSETS_DST_DIR" ]; then
	echo "ERROR: Directory '$ASSETS_DST_DIR' already exists!"
	exit 10
    fi

    cp -a "$ASSETS_SRC_DIR" "$ASSETS_DST_DIR"
done

exit 0
