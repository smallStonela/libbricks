all:
	@sh ./cmake
	@$(MAKE) --no-print-directory -C build -j8

android:
	@ndk-build APP_BUILD_SCRIPT=Android.mk NDK_PROJECT_PATH=. NDK_APP_OUT=build -j8

clean:
	@$(MAKE) --no-print-directory -C build clean

test: all
	@sh ./ctest

run: all
	@$(MAKE) --no-print-directory -C test run

debug: all
	@$(MAKE) --no-print-directory -C test debug

.PHONY: all clean test run debug
