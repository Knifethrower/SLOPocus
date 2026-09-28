# Cross-compile SLOPocus for 64-bit Windows from Linux with mingw-w64:
#   cmake -S . -B build-win -G Ninja -DCMAKE_TOOLCHAIN_FILE=cmake/mingw-w64-x86_64.cmake \
#         -DMINGW_SDL2_ROOT=<dir holding the SDL2/SDL2_image/SDL2_mixer mingw devel trees>
# The SDL2 development packages are the "-mingw.tar.gz" releases from
# libsdl-org (each has an x86_64-w64-mingw32/ prefix); tinyxml2 is built from
# dependencies/tinyxml2. The C++ runtime is linked statically so only the
# SDL DLLs travel with the exe.
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

set(TOOLCHAIN_PREFIX x86_64-w64-mingw32)
set(CMAKE_C_COMPILER ${TOOLCHAIN_PREFIX}-gcc-posix)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}-g++-posix)
set(CMAKE_RC_COMPILER ${TOOLCHAIN_PREFIX}-windres)

set(MINGW_SDL2_ROOT "" CACHE PATH "Directory holding the extracted SDL2 mingw development packages")
set(CMAKE_FIND_ROOT_PATH
        /usr/${TOOLCHAIN_PREFIX}
        ${MINGW_SDL2_ROOT}/SDL2/${TOOLCHAIN_PREFIX}
        ${MINGW_SDL2_ROOT}/SDL2_image/${TOOLCHAIN_PREFIX}
        ${MINGW_SDL2_ROOT}/SDL2_mixer/${TOOLCHAIN_PREFIX})
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

set(CMAKE_EXE_LINKER_FLAGS_INIT "-static-libgcc -static-libstdc++ -Wl,-Bstatic,--whole-archive -lwinpthread -Wl,--no-whole-archive,-Bdynamic")
