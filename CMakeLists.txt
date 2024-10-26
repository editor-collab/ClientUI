cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

project(EditorCollabUI VERSION 1.0.0)

add_library(${PROJECT_NAME} SHARED
    src/main.cpp
    src/hooks/EditLevelLayer.cpp
    src/hooks/LevelBrowserLayer.cpp
    src/hooks/ui/LevelBrowserLayer.cpp
    src/hooks/ui/EditorUI.cpp
    src/managers/AccountManager.cpp
    src/managers/BrowserManager.cpp
    src/managers/CellManager.cpp
    src/managers/ErrorManager.cpp
    src/managers/FetchManager.cpp
    src/managers/LevelManager.cpp
    src/managers/WebManager.cpp
    src/ui/GenericForm.cpp
    src/ui/GenericList.cpp
    src/ui/MainScene.cpp
    src/ui/ExploreMenu.cpp
    src/ui/ShareSettings.cpp
    src/utils/CryptoHelper.cpp
    src/utils/ThemeProvider.cpp
)

if (${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
    target_compile_options(${PROJECT_NAME} PRIVATE
        -Wno-reorder-init-list
    )
endif()

target_include_directories(${PROJECT_NAME} PRIVATE
    include
)

if (NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "Unable to find Geode SDK! Please define GEODE_SDK environment variable to point to Geode")
else()
    message(STATUS "Found Geode: $ENV{GEODE_SDK}")
endif()

add_subdirectory($ENV{GEODE_SDK} ${CMAKE_CURRENT_BINARY_DIR}/geode)

CPMAddPackage(
    NAME ChaCha20Poly1305
    GITHUB_REPOSITORY "mrdcvlsc/ChaCha20-Poly1305"
    GIT_TAG "v3.0"
    DOWNLOAD_ONLY YES
)

CPMAddPackage(
    NAME BLAKE2
    GITHUB_REPOSITORY "BLAKE2/BLAKE2"
    GIT_TAG "ed1974e"
    DOWNLOAD_ONLY YES
)

CPMAddPackage("gh:tplgy/cppcodec#8019b8b")

add_library(ChaCha20Poly1305 STATIC
    ${ChaCha20Poly1305_SOURCE_DIR}/ChaCha20-Poly1305.cpp
)
target_include_directories(ChaCha20Poly1305 PUBLIC ${ChaCha20Poly1305_SOURCE_DIR})

add_library(BLAKE2 STATIC
    ${BLAKE2_SOURCE_DIR}/ref/blake2b-ref.c
)
target_include_directories(BLAKE2 PUBLIC ${BLAKE2_SOURCE_DIR}/ref)

target_link_libraries(${PROJECT_NAME}
    ChaCha20Poly1305
    BLAKE2
    cppcodec
)

setup_geode_mod(${PROJECT_NAME})
