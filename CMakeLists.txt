cmake_minimum_required(VERSION 3.15)
project(RCUT LANGUAGES C CXX)

option(RCUT_BUILD_SHARED "Build the RCUT library as a shared library" OFF)
option(RCUT_BUILD_EXAMPLE "Build the RCUT example executable" ON)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(RCUT_INCLUDE_DIRS
    "${CMAKE_CURRENT_SOURCE_DIR}/Include"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Application"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/IOStream"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Raycasting"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Extern/Includes"
)

set(RCUT_SOURCES
    src/RCUT.c
    src/Application/RCUT_audioSauce.c
    src/Application/RCUT_freeGlut_Bridge.c
    src/Application/RCUT_Win.c
    src/IOStream/RCUT_IOSTREAM.c
    src/Raycasting/RCUT_DDA_Raycaster.c
    src/Raycasting/RCUT_Objects.c
    src/Raycasting/RCUT_Textures.c
)

if(RCUT_BUILD_SHARED)
    add_library(RCUT SHARED ${RCUT_SOURCES})
    target_compile_definitions(RCUT PRIVATE RCUT_BUILD_DLL)
    set_target_properties(RCUT PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
else()
    add_library(RCUT STATIC ${RCUT_SOURCES})
    target_compile_definitions(RCUT PRIVATE RCUT_STATIC)
endif()

target_include_directories(RCUT
    PUBLIC
        ${RCUT_INCLUDE_DIRS}
)

set(FREEGLUT_LIBRARY_PATH "${CMAKE_CURRENT_SOURCE_DIR}/src/Extern/Libraries/libfreeglut.a")
if(EXISTS "${FREEGLUT_LIBRARY_PATH}")
    set(RCUT_FREEGLUT_LIB "${FREEGLUT_LIBRARY_PATH}")
else()
    find_library(RCUT_FREEGLUT_LIB NAMES freeglut PATHS "${CMAKE_CURRENT_SOURCE_DIR}/src/Extern/Libraries" NO_DEFAULT_PATH)
endif()

if(RCUT_FREEGLUT_LIB)
    target_link_libraries(RCUT PUBLIC
        ${RCUT_FREEGLUT_LIB}
        opengl32
        glu32
        gdi32
        winmm
    )
else()
    message(WARNING "Could not locate libfreeglut.a. The RCUT target will still be configured, but you may need to install FreeGLUT or adjust CMakeLists.txt.")
    target_link_libraries(RCUT PUBLIC
        freeglut
        opengl32
        glu32
        gdi32
        winmm
    )
endif()

if(RCUT_BUILD_EXAMPLE)
    add_executable(RCUTApp main.cpp)
    target_include_directories(RCUTApp PRIVATE ${RCUT_INCLUDE_DIRS})
    target_link_libraries(RCUTApp PRIVATE RCUT)

    # Without this, main.cpp falls through to the "else" branch of every
    # RCUT_API macro (plain, unannotated declarations) instead of
    # __declspec(dllimport) -- see RCUT_BUILD_DLL above, which the RCUT
    # target gets but RCUTApp never did on its own.
    if(RCUT_BUILD_SHARED)
        target_compile_definitions(RCUTApp PRIVATE RCUT_USE_DLL)
    endif()

    # freeglut.dll is a vendored binary, not a CMake target, so nothing
    # else puts it next to the exe. Needed at runtime regardless of
    # whether RCUT itself is built static or shared.
    set(RCUT_FREEGLUT_DLL "${CMAKE_CURRENT_SOURCE_DIR}/src/Extern/Libraries/freeglut.dll")
    if(EXISTS "${RCUT_FREEGLUT_DLL}")
        add_custom_command(TARGET RCUTApp POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${RCUT_FREEGLUT_DLL}"
                "$<TARGET_FILE_DIR:RCUTApp>"
        )
    else()
        message(WARNING "Could not find freeglut.dll at ${RCUT_FREEGLUT_DLL} -- RCUTApp.exe will need it copied next to it manually to run.")
    endif()
endif()

message(STATUS "RCUT build configured: ${RCUT_BUILD_SHARED} library, example = ${RCUT_BUILD_EXAMPLE}")