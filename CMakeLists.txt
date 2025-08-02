cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

project(EditorCollabUI VERSION 1.0.0)

add_library(${PROJECT_NAME} SHARED
    src/main.cpp
    src/hooks/CustomSongLayer.cpp
    src/hooks/EditLevelLayer.cpp
    src/hooks/LevelBrowserLayer.cpp
    src/hooks/ui/LevelBrowserLayer.cpp
    src/hooks/ui/EditorUI.cpp
    src/hooks/ui/EditorPauseLayer.cpp
    src/hooks/ui/PauseLayer.cpp
    src/managers/AccountManager.cpp
    src/managers/BrowserManager.cpp
    src/managers/CellManager.cpp
    src/managers/ErrorManager.cpp
    src/managers/FetchManager.cpp
    src/managers/LevelManager.cpp
    src/managers/LocalManager.cpp
    src/managers/WebManager.cpp
    src/ui/BuyPopup.cpp
    src/ui/GenericForm.cpp
    src/ui/GenericList.cpp
    src/ui/MainScene.cpp
    src/ui/LevelUserList.cpp
    src/ui/ExploreMenu.cpp
    src/ui/ShareSettings.cpp
    src/ui/LimitsSettings.cpp
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

CPMAddPackage("gh:GlobedGD/argon@1.1.9")

CPMAddPackage("gh:tplgy/cppcodec#8019b8b")

CPMAddPackage("gh:altalk23/lavender#v1.0.0")

target_link_libraries(${PROJECT_NAME}
    cppcodec
    Lavender
    argon
)

setup_geode_mod(${PROJECT_NAME} EXTERNALS alk.editor-collab cvolton.level-id-api geode.node-ids)
