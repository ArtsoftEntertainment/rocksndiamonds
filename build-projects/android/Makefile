# =============================================================================
# Rocks'n'Diamonds for Android
# -----------------------------------------------------------------------------
# (c) 1995-2020 by Artsoft Entertainment
#                  Holger Schemel
#                  info@artsoft.org
#                  https://www.artsoft.org/
# -----------------------------------------------------------------------------
# build-projects/android/Makefile
# =============================================================================

PROGBASE = rocksndiamonds

AUTO_GENERATED_BUILD_DIRS = .gradle app/.externalNativeBuild app/build
AUTO_GENERATED_OTHER_DIRS = app/jni/SDL2* app/src/main/assets .prepared

export ANDROID_HOME := $(HOME)/projects/Android/SDK


all: package

sdl:
	./build-scripts/create_sdl.sh

manifest:
	./build-scripts/create_manifest.sh

assets:
	./build-scripts/create_assets.sh

assets-toc:
	./build-scripts/create_asset_tocs.sh app/src/main/assets

.prepared: sdl manifest assets assets-toc
	make -C ../.. auto-conf conf-time conf-hash

prepare: .prepared

package: prepare
	./gradlew assembleDebug

clean-build:
	./gradlew clean

clean-auto-generated:
	rm -rf $(AUTO_GENERATED_BUILD_DIRS) $(AUTO_GENERATED_OTHER_DIRS)

clean: clean-build

dist-clean: clean-build clean-auto-generated
