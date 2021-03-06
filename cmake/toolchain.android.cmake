# Adapted from http://code.google.com/p/android-cmake/

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_VERSION 1)

if(APPLE)
	set(HOST_APPLE true)
elseif(UNIX)
	set(HOST_UNIX true)
endif()

set(APPLE false)
set(UNIX true)

set(ANDROID_NDK_DEFAULT_SEARCH_PATH /opt/android-ndk)
set(ANDROID_NDK_SUPPORTED_VERSIONS -r8 -r7 -r6 -r5c -r5b -r5 "")
set(ANDROID_NDK_TOOLCHAIN_DEFAULT_SEARCH_PATH /opt/android-toolchain)
set(TOOL_OS_SUFFIX "")

if(NOT DEFINED ANDROID_NDK)
	set(ANDROID_NDK "$ENV{ANDROID_NDK}")
endif()

if(NOT DEFINED ANDROID_NDK_TOOLCHAIN_ROOT)
	set(ANDROID_NDK_TOOLCHAIN_ROOT $ENV{ANDROID_NDK_TOOLCHAIN_ROOT})
endif()

if(NOT EXISTS "${ANDROID_NDK}")
	foreach(ndk_version ${ANDROID_NDK_SUPPORTED_VERSIONS})
		if(EXISTS ${ANDROID_NDK_DEFAULT_SEARCH_PATH}${ndk_version})
			set(ANDROID_NDK "${ANDROID_NDK_DEFAULT_SEARCH_PATH}${ndk_version}")
			message(STATUS "Using default path for android NDK ${ANDROID_NDK}")
			message(STATUS "  If you prefer to use a different location, please define the variable: ANDROID_NDK")
			break()
		endif()
	endforeach()
endif()

if(EXISTS "${ANDROID_NDK}")
	if(APPLE)
		set(ANDROID_TOOLCHAIN_SYSTEM "darwin-x86")
	elseif(WIN32)
		set(ANDROID_TOOLCHAIN_SYSTEM "windows")
		set(TOOL_OS_SUFFIX ".exe")
	elseif(UNIX)
		set(ANDROID_TOOLCHAIN_SYSTEM "linux-x86")
	else()
		message(FATAL_ERROR "Your platform is not supported")
	endif()

	if(NOT ANDROID_API_LEVEL GREATER 2)
		if(NDK_CPU_X86)
			set(ANDROID_API_LEVEL 9)
		else()
			set(ANDROID_API_LEVEL 5)
		endif()
		message(STATUS "Using default android API level android-${ANDROID_API_LEVEL}")
		message(STATUS "  If you prefer to use a different API level, please define the variable: ANDROID_API_LEVEL")
	endif()

	if (NDK_CPU_X86)
		set(ANDROID_TOOLCHAIN_PREFIX "i686-android-linux")
		set(ANDROID_TOOLCHAIN_NAME "x86")
		set(ANDROID_TOOLCHAIN_PLATFORM_NAME "x86")
	else()
		set(ANDROID_TOOLCHAIN_PREFIX "arm-linux-androideabi")
		set(ANDROID_TOOLCHAIN_NAME "${ANDROID_TOOLCHAIN_PREFIX}")
		set(ANDROID_TOOLCHAIN_PLATFORM_NAME "arm")
	endif()

	if (NOT DEFINED ANDROID_TOOLCHAIN_VERSION)
		set(SUPPORTED_TOOLCHAIN_VERSIONS 4.4.0 4.4.3 4.6.0 4.6.1 4.6.2 4.6.3 4.7.0 4.8)
		foreach(gcc_version ${SUPPORTED_TOOLCHAIN_VERSIONS})
			if (EXISTS "${ANDROID_NDK}/toolchains/${ANDROID_TOOLCHAIN_NAME}-${gcc_version}")
				set(ANDROID_TOOLCHAIN_VERSION ${gcc_version})
			endif()
		endforeach()
		if(NOT DEFINED ANDROID_TOOLCHAIN_VERSION)
			message(FATAL_ERROR "Unable to find toolchain")
		else()
			message(STATUS "Using auto-detected toolchain ${ANDROID_TOOLCHAIN_NAME}-${ANDROID_TOOLCHAIN_VERSION}")
		endif()
	endif()
	if (EXISTS "${ANDROID_NDK}/toolchains/clang-android-${ANDROID_TOOLCHAIN_VERSION}")
		set (ANDROID_TOOLCHAIN_NAME "clang-android")
		message(STATUS "Found clang toolchain ${ANDROID_TOOLCHAIN_NAME}-${ANDROID_TOOLCHAIN_VERSION}")
	endif()

	set(ANDROID_NDK_SYSROOT "${ANDROID_NDK}/platforms/android-${ANDROID_API_LEVEL}/arch-${ANDROID_TOOLCHAIN_PLATFORM_NAME}")
	set(ANDROID_NDK_TOOLCHAIN_ROOT "${ANDROID_NDK}/toolchains/${ANDROID_TOOLCHAIN_NAME}-${ANDROID_TOOLCHAIN_VERSION}/prebuilt/${ANDROID_TOOLCHAIN_SYSTEM}")
endif()

set(CMAKE_C_COMPILER   "${ANDROID_NDK_TOOLCHAIN_ROOT}/bin/${ANDROID_TOOLCHAIN_PREFIX}-gcc${TOOL_OS_SUFFIX}"     CACHE PATH "gcc" FORCE)
set(CMAKE_CXX_COMPILER "${ANDROID_NDK_TOOLCHAIN_ROOT}/bin/${ANDROID_TOOLCHAIN_PREFIX}-g++${TOOL_OS_SUFFIX}"     CACHE PATH "g++" FORCE)
set(CMAKE_AR           "${ANDROID_NDK_TOOLCHAIN_ROOT}/bin/${ANDROID_TOOLCHAIN_PREFIX}-ar${TOOL_OS_SUFFIX}"      CACHE PATH "archive" FORCE)
set(CMAKE_LINKER       "${ANDROID_NDK_TOOLCHAIN_ROOT}/bin/${ANDROID_TOOLCHAIN_PREFIX}-ld${TOOL_OS_SUFFIX}"      CACHE PATH "linker" FORCE)
set(CMAKE_NM           "${ANDROID_NDK_TOOLCHAIN_ROOT}/bin/${ANDROID_TOOLCHAIN_PREFIX}-nm${TOOL_OS_SUFFIX}"      CACHE PATH "nm" FORCE)
set(CMAKE_OBJCOPY      "${ANDROID_NDK_TOOLCHAIN_ROOT}/bin/${ANDROID_TOOLCHAIN_PREFIX}-objcopy${TOOL_OS_SUFFIX}" CACHE PATH "objcopy" FORCE)
set(CMAKE_OBJDUMP      "${ANDROID_NDK_TOOLCHAIN_ROOT}/bin/${ANDROID_TOOLCHAIN_PREFIX}-objdump${TOOL_OS_SUFFIX}" CACHE PATH "objdump" FORCE)
set(CMAKE_STRIP        "${ANDROID_NDK_TOOLCHAIN_ROOT}/bin/${ANDROID_TOOLCHAIN_PREFIX}-strip${TOOL_OS_SUFFIX}"   CACHE PATH "strip" FORCE)
set(CMAKE_RANLIB       "${ANDROID_NDK_TOOLCHAIN_ROOT}/bin/${ANDROID_TOOLCHAIN_PREFIX}-ranlib${TOOL_OS_SUFFIX}"  CACHE PATH "ranlib" FORCE)

if(NDK_CPU_X86)
	set(ANDROID_NDK_CPU_NAME "x86")
	set(CMAKE_SYSTEM_PROCESSOR "x86")
elseif(NDK_CPU_ARM_V7A)
	set(NDK_CPU_ARM_NEON false)
	set(NDK_CPU_ARM_VFPV3 false)
	set(NDK_CPU_ARM true)
	set(ANDROID_NDK_CPU_NAME "armeabi-v7a")
	set(CMAKE_SYSTEM_PROCESSOR "armv7-a")
else()
	set(NDK_CPU_ARM true)
	set(ANDROID_NDK_CPU_NAME "armeabi")
	set(NDK_CPU_ARM_NEON false)
	set(CMAKE_SYSTEM_PROCESSOR "armv5te")
endif()

set(CMAKE_FIND_ROOT_PATH "${ANDROID_NDK_TOOLCHAIN_ROOT}/bin" "${ANDROID_NDK_TOOLCHAIN_ROOT}/${ANDROID_TOOLCHAIN_PREFIX}" "${ANDROID_NDK_SYSROOT}" "${CMAKE_INSTALL_PREFIX}" "${CMAKE_INSTALL_PREFIX}/share")

set(STL_PATH "${ANDROID_NDK}/sources/cxx-stl/gnu-libstdc++/${ANDROID_TOOLCHAIN_VERSION}")
set(STL_LIBRARIES_PATH "${STL_PATH}/libs/${ANDROID_NDK_CPU_NAME}")
include_directories(SYSTEM "${STL_PATH}/include" "${STL_LIBRARIES_PATH}/include")
link_directories("${ANDROID_NDK_SYSROOT}/usr/lib")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM BOTH)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

set(CMAKE_C_FLAGS "-fPIC -DANDROID -Wno-psabi -fsigned-char")

set(FORCE_ARM OFF CACHE BOOL "Use 32-bit ARM instructions instead of Thumb-1")
if(NDK_CPU_X86)
elseif(NOT FORCE_ARM)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mthumb")
else()
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -marm")
endif()

set(CMAKE_C_FLAGS "--sysroot=${ANDROID_NDK_SYSROOT} ${CMAKE_C_FLAGS}")

include(CMakeForceCompiler)
CMAKE_FORCE_C_COMPILER("${CMAKE_C_COMPILER}" GNU)
CMAKE_FORCE_CXX_COMPILER("${CMAKE_CXX_COMPILER}" GNU)

if(NDK_CPU_ARM_V7A)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=armv7-a -mfloat-abi=softfp")
	if(NDK_CPU_ARM_NEON)
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mfpu=neon")
	elseif(NDK_CPU_ARM_VFPV3)
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mfpu=vfpv3")
	endif()
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS}" CACHE STRING "c++ flags")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}" CACHE STRING "c flags")
      
set(LINKER_FLAGS "-L\"${STL_LIBRARIES_PATH}\" -L\"${CMAKE_INSTALL_PREFIX}/libs/${ANDROID_NDK_CPU_NAME}\"")
if(NDK_CPU_ARM OR NDK_CPU_ARM_V7A)
	set(LINKER_FLAGS "-Wl,--fix-cortex-a8 ${LINKER_FLAGS}")
endif()

set(CMAKE_SHARED_LINKER_FLAGS "${LINKER_FLAGS}" CACHE STRING "linker flags" FORCE)
set(CMAKE_MODULE_LINKER_FLAGS "${LINKER_FLAGS}" CACHE STRING "linker flags" FORCE)
set(CMAKE_EXE_LINKER_FLAGS "${LINKER_FLAGS}" CACHE STRING "linker flags" FORCE)

set(ANDROID true)
set(BUILD_ANDROID true)

MARK_AS_ADVANCED(FORCE_ARM NO_UNDEFINED)
