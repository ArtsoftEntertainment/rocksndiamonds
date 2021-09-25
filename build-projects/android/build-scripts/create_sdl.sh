#!/bin/bash

JNI_DIR="app/jni"

ANDROID_MK_SDL_IMAGE="$JNI_DIR/SDL2_image/Android.mk"
ANDROID_MK_SDL_MIXER="$JNI_DIR/SDL2_mixer/Android.mk"

SDL_BASE_URL_ORIGINAL="https://www.libsdl.org"
SDL_BASE_URL_FALLBACK="https://www.artsoft.org"
SDL_VERSIONS=`cat SDL_VERSIONS`

for i in $SDL_VERSIONS; do
    SDL_SUBDIR=`echo $i | sed -e "s/-.*//"`
    SDL_SUBURL=`echo $SDL_SUBDIR | tr -d '2'`

    if [ -d "$JNI_DIR/$SDL_SUBDIR" ]; then
	continue;
    fi

    if [ "$SDL_SUBURL" = "SDL" ]; then
	SDL_RELEASE_DIR="release"
    else
	SDL_RELEASE_DIR="projects/$SDL_SUBURL/release"
    fi

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

if [ ! -f "$ANDROID_MK_SDL_IMAGE.dist" ]; then
    cp -a "$ANDROID_MK_SDL_IMAGE" "$ANDROID_MK_SDL_IMAGE.dist"
    cat "$ANDROID_MK_SDL_IMAGE.dist"					\
	| sed -e "s/^SUPPORT_JPG ?= true/SUPPORT_JPG ?= false/"	\
	| sed -e "s/^SUPPORT_WEBP ?= true/SUPPORT_WEBP ?= false/"	\
	> "$ANDROID_MK_SDL_IMAGE"
fi

if [ ! -f "$ANDROID_MK_SDL_MIXER.dist" ]; then
    cp -a "$ANDROID_MK_SDL_MIXER" "$ANDROID_MK_SDL_MIXER.dist"
    cat "$ANDROID_MK_SDL_MIXER.dist"					\
	| sed -e "s/^SUPPORT_OGG ?= true/SUPPORT_OGG ?= false/"	\
	| sed -e "s/^SUPPORT_FLAC ?= true/SUPPORT_FLAC ?= false/"	\
	| sed -e "s/^LOCAL_CFLAGS :=/LOCAL_CFLAGS := -DMUSIC_WAV/"	\
	> "$ANDROID_MK_SDL_MIXER"
fi

exit 0
