OSTYPE				:=	$(shell echo $$OSTYPE)
CMAKE				:=	$(SHELL) $(BRICKSDIR)/cmake/cmake.sh
CTEST				:=	$(SHELL) $(BRICKSDIR)/cmake/ctest.sh
MAKEIT				:=	$(MAKE) --no-print-directory
TOOLCHAIN_ANDROID	:=	-DCMAKE_TOOLCHAIN_FILE=$(BRICKSDIR)/cmake/toolchain.android.cmake
TOOLCHAIN_IOS		:=	-DCMAKE_TOOLCHAIN_FILE=$(BRICKSDIR)/cmake/toolchain.ios.cmake
TOOLCHAIN_CLANG		:=	-DCMAKE_TOOLCHAIN_FILE=$(BRICKSDIR)/cmake/toolchain.clang.cmake
TOOLCHAIN_MINGW32	:=	-DCMAKE_TOOLCHAIN_FILE=$(BRICKSDIR)/cmake/toolchain.mingw32.cmake
TOOLCHAIN_ASM_JS	:=	-DCMAKE_TOOLCHAIN_FILE=$(BRICKSDIR)/cmake/toolchain.emscripten.cmake -DEMSCRIPTEN_ROOT_PATH=/usr/lib/emscripten

all:
	@$(CMAKE) $(CURDIR) $(CURDIR)/build-$(OSTYPE)
	@+$(MAKEIT) -C $(CURDIR)/build-$(OSTYPE)

clang:
	@$(CMAKE) $(CURDIR) $(CURDIR)/build-$(OSTYPE)-clang $(TOOLCHAIN_CLANG)
	@+$(MAKEIT) -C $(CURDIR)/build-$(OSTYPE)-clang

mingw32:
	@$(CMAKE) $(CURDIR) $(CURDIR)/build-mingw32 $(TOOLCHAIN_MINGW32)
	@+$(MAKEIT) -C $(CURDIR)/build-mingw32

android: android-armv5 android-armv7 android-x86

android-armv5:
	@$(CMAKE) $(CURDIR) $(CURDIR)/build-android-armv5 $(TOOLCHAIN_ANDROID) -DNDK_CPU_ARM=y
	@+$(MAKEIT) -C $(CURDIR)/build-android-armv5

android-armv7:
	@$(CMAKE) $(CURDIR) $(CURDIR)/build-android-armv7 $(TOOLCHAIN_ANDROID) -DNDK_CPU_ARM_V7A=y -DNDK_CPU_ARM_VFPV3=y
	@+$(MAKEIT) -C $(CURDIR)/build-android-armv7

android-x86:
	@$(CMAKE) $(CURDIR) $(CURDIR)/build-android-x86 $(TOOLCHAIN_ANDROID) -DNDK_CPU_X86=y
	@+$(MAKEIT) -C $(CURDIR)/build-android-x86

ifeq ($(OSTYPE),linux-gnu)
ios:
	@$(CMAKE) $(CURDIR) $(CURDIR)/build-ios-armv6 $(TOOLCHAIN_IOS)
	@+$(MAKEIT) -C $(CURDIR)/build-ios-armv6
else
xcode:
	@$(CMAKE) $(CURDIR) $(CURDIR)/build-$(OSTYPE)-xcode -G Xcode

ios-xcode:
	@$(CMAKE) $(CURDIR) $(CURDIR)/build-ios-xcode $(TOOLCHAIN_IOS) -G Xcode

ios: ios-xcode
	@$(CMAKE) $(CURDIR) $(CURDIR)/build-ios-universal $(TOOLCHAIN_IOS)
	@+$(MAKEIT) -C $(CURDIR)/build-ios-universal
	@$(CMAKE) $(CURDIR) $(CURDIR)/build-ios-simulator $(TOOLCHAIN_IOS) -DIOS_SIMULATOR=y
	@+$(MAKEIT) -C $(CURDIR)/build-ios-simulator

all: xcode
endif

asm.js:
	@$(CMAKE) $(CURDIR) $(CURDIR)/build-asm.js $(TOOLCHAIN_ASM_JS)
	@+$(MAKEIT) -C $(CURDIR)/build-asm.js

clean:
	@rm -rf build-*

test: all
	@$(CTEST) $(CURDIR)/build-$(OSTYPE)

ifdef EXECUTABLE_NAME
run: all
	@$(CURDIR)/build-$(OSTYPE)/$(EXECUTABLE_NAME)

clang-run: clang
	@$(CURDIR)/build-$(OSTYPE)-clang/$(EXECUTABLE_NAME)

debug: all
	@gdb $(CURDIR)/build-$(OSTYPE)/$(EXECUTABLE_NAME) -ex run

clang-debug: clang
	@gdb $(CURDIR)/build-$(OSTYPE)-clang/$(EXECUTABLE_NAME) -ex run

clang-test: clang
	@$(CTEST) $(CURDIR)/build-$(OSTYPE)-clang

asm.js-test: asm.js
	@$(CTEST) $(CURDIR)/build-asm.js
endif

.PHONY: all xcode clean test run debug clang clang-run clang-debug android android-armv5 android-armv7 android-x86 ios ios-xcode
