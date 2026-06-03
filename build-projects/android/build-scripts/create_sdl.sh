#!/bin/bash

JNI_DIR="app/jni"

ANDROID_MK_SDL_IMAGE="$JNI_DIR/SDL2_image/Android.mk"
ANDROID_MK_SDL_MIXER="$JNI_DIR/SDL2_mixer/Android.mk"

SDL_BASE_URL_ORIGINAL="https://github.com"
SDL_BASE_URL_FALLBACK="https://www.artsoft.org"
SDL_VERSIONS=`cat SDL_VERSIONS`

for i in $SDL_VERSIONS; do
    SDL_SUBDIR=`echo $i | sed -e "s/-.*//"`
    SDL_VERSION=`echo $i | sed -e "s/.*-//"`
    SDL_SUBURL=`echo $SDL_SUBDIR | tr -d '2'`

    if [ -d "$JNI_DIR/$SDL_SUBDIR" ]; then
	continue;
    fi

    SDL_RELEASE_DIR="libsdl-org/$SDL_SUBURL/releases/download/release-$SDL_VERSION"
    SDL_URL="$SDL_BASE_URL_ORIGINAL/$SDL_RELEASE_DIR/$i.tar.gz"

    wget --timeout=10 -O - "$SDL_URL" | (cd "$JNI_DIR" && tar xzf -)

    if [ "$?" != "0" ]; then
	echo "ERROR: Installing '$i' from main site failed -- trying fallback!"

	SDL_URL="$SDL_BASE_URL_FALLBACK/RELEASES/sdl/$i.tar.gz"

	wget --timeout=10 -O - "$SDL_URL" | (cd "$JNI_DIR" && tar xzf -)

	if [ "$?" != "0" ]; then
	    echo "ERROR: Installing '$i' from fallback site failed!"
	    exit 10
	fi
    fi

    mv "$JNI_DIR/$i" "$JNI_DIR/$SDL_SUBDIR"
done

if [ ! -f "$ANDROID_MK_SDL_MIXER.dist" ]; then
    cp -a "$ANDROID_MK_SDL_MIXER" "$ANDROID_MK_SDL_MIXER.dist"
    cat "$ANDROID_MK_SDL_MIXER.dist"							\
	| sed -e "s/^SUPPORT_MOD_XMP ?= false/SUPPORT_MOD_XMP ?= true/"			\
	| sed -e "s/^SUPPORT_MID_TIMIDITY ?= false/SUPPORT_MID_TIMIDITY ?= true/"	\
	| sed -e "s/^SUPPORT_WAVPACK ?= true/SUPPORT_WAVPACK ?= false/"			\
	| sed -e "s/^SUPPORT_OGG_STB ?= true/SUPPORT_OGG_STB ?= false/"			\
	| sed -e "s/^SUPPORT_FLAC_DRFLAC ?= true/SUPPORT_FLAC_DRFLAC ?= false/"		\
	> "$ANDROID_MK_SDL_MIXER"
fi

./$JNI_DIR/SDL2_mixer/external/download.sh

exit 0
