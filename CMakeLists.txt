cmake_minimum_required(VERSION 3.14.3)

project(SLOPocus VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/sdl2)
find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_mixer REQUIRED)

include_directories(${SDL2_INCLUDE_DIR} ${SDL2_INCLUDE_DIRS} ${SDL2_IMAGE_INCLUDE_DIRS} ${SDL2_MIXER_INCLUDE_DIRS})
# The sources include <SDL2/SDL.h>; the find-modules return the SDL2/ directory
# itself, so its parent must be searchable too (the mingw packages have no
# system-wide include directory).
foreach(sdl_include ${SDL2_INCLUDE_DIR} ${SDL2_IMAGE_INCLUDE_DIR} ${SDL2_MIXER_INCLUDE_DIR})
    get_filename_component(sdl_include_parent ${sdl_include} DIRECTORY)
    include_directories(${sdl_include_parent})
endforeach()
include_directories(dependencies/plog/include)

set(sources
        src/main.cpp
        src/slopocus.cpp
        src/gameversion.cpp
        src/exedata.cpp
        src/exelayout.cpp
        src/savefile.cpp
        src/screens/screen.cpp
        src/screens/textpages.cpp
        src/screens/imagepages.cpp
        src/screens/message.cpp
        src/screens/highscores.cpp
        src/screens/slots.cpp
        src/screens/volume.cpp
        src/screens/keyconfig.cpp
        src/screens/orderscreens.cpp
        src/screens/tally.cpp
        src/screens/episodecomplete.cpp
        src/stategame.cpp
        src/apogeesplash.cpp
        src/introsplash.cpp
        src/mainmenu.cpp
        src/optionsmenu.cpp
        )

set(data_sources
        src/engine/data/datamanager.cpp
        src/engine/data/data.cpp
        src/engine/data/fat.cpp
        src/engine/data/datafile.cpp
        src/engine/data/fatloader.cpp
        src/engine/data/asset/image.cpp
        src/engine/data/asset/palette.cpp
        src/engine/data/asset/pcx.cpp
        src/engine/data/asset/voc.cpp
        src/engine/data/asset/midi.cpp
        src/engine/data/asset/font.cpp
        src/engine/data/asset/level.cpp
        src/engine/data/asset/leveltime.cpp
        src/engine/data/asset/leveltileset.cpp
        src/engine/data/asset/levelbackground.cpp
        src/engine/data/asset/levelmusic.cpp
        src/engine/data/asset/levelelevator.cpp
        src/engine/data/asset/spriteset.cpp
        src/engine/data/asset/iteminfo.cpp
        )

set(engine_sources
        src/engine/log.cpp
        src/engine/pocusengine.cpp
        src/engine/provider/provider.cpp
        src/engine/statemanager.cpp
        src/engine/fade.cpp
        src/engine/sound.h
        src/engine/texture.h
        src/engine/renderer.h
        src/engine/audio.h
        src/engine/musicplayer.cpp
        src/engine/menu.cpp
        src/engine/particles.cpp
        src/engine/animation.cpp
        src/engine/map.cpp
        src/engine/point.cpp
        src/engine/config.cpp
        src/engine/rules.cpp
        src/engine/player.cpp
        src/engine/game.cpp
        src/engine/hud.cpp
        src/engine/entity.cpp
        src/engine/size.cpp
        src/engine/rect.cpp
        src/engine/hocus.cpp
        src/engine/enemy.cpp
        src/engine/texturetools.cpp
        )

set(sdl_sources
        src/engine/sdl/sdlrenderer.cpp
        src/engine/sdl/sdleventhandler.cpp
        src/engine/sdl/sdltexture.cpp
        src/engine/sdl/sdlsound.cpp
        src/engine/sdl/sdlaudio.cpp
        )

add_executable(SLOPocus ${sources} ${data_sources} ${engine_sources} ${sdl_sources})

# tinyxml2: the system library on Linux, or the vendored source when it is
# present (the Windows cross build, see cmake/mingw-w64-x86_64.cmake).
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/tinyxml2/tinyxml2.cpp)
    target_sources(SLOPocus PRIVATE dependencies/tinyxml2/tinyxml2.cpp)
    target_include_directories(SLOPocus PRIVATE dependencies/tinyxml2)
    set(TINYXML2_LIBRARY "")
else()
    set(TINYXML2_LIBRARY -ltinyxml2)
endif()

target_link_libraries(SLOPocus ${SDL2_LIBRARY} ${SDL2_IMAGE_LIBRARIES} ${SDL2_MIXER_LIBRARIES} ${TINYXML2_LIBRARY})

# Windows cross build: the SDL DLLs (and the codec DLLs shipped with them)
# go next to the exe.
if(CMAKE_CROSSCOMPILING AND WIN32 AND MINGW_SDL2_ROOT)
    foreach(package SDL2 SDL2_image SDL2_mixer)
        file(GLOB package_dlls ${MINGW_SDL2_ROOT}/${package}/x86_64-w64-mingw32/bin/*.dll)
        file(COPY ${package_dlls} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
    endforeach()
endif()

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/data/shareware.fat
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR})