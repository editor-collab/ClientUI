cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if ("${CMAKE_SYSTEM_NAME}" STREQUAL "iOS" OR IOS)
    set(CMAKE_OSX_ARCHITECTURES "arm64")
else()
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
endif()
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
    src/hooks/ui/LevelEditorLayer.cpp
    src/managers/AccountManager.cpp
    src/managers/BrowserManager.cpp
    src/managers/CellManager.cpp
    src/managers/ErrorManager.cpp
    src/managers/FetchManager.cpp
    src/managers/LevelManager.cpp
    src/managers/LocalManager.cpp
    src/managers/WebManager.cpp
    src/ui/EditorOverlay.cpp
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

CPMAddPackage("gh:GlobedGD/argon@1.4.3")

CPMAddPackage("gh:tplgy/cppcodec#8019b8b")

CPMAddPackage("gh:altalk23/lavender#main")

target_link_libraries(${PROJECT_NAME}
    cppcodec
    Lavender
    argon
)

setup_geode_mod(${PROJECT_NAME} EXTERNALS )


if (ANDROID)
	if (CMAKE_BUILD_TYPE STREQUAL "Release")
		# 1. Extract full debug info into a separate .sym file
        add_custom_command(
            TARGET "${PROJECT_NAME}" POST_BUILD
            DEPENDS "${PROJECT_NAME}"
            COMMAND ${CMAKE_OBJCOPY} 
			ARGS --only-keep-debug "$<TARGET_FILE:${PROJECT_NAME}>" "$<TARGET_FILE:${PROJECT_NAME}>.dbg"
        )
		# 2. Strip everything from the binary
		add_custom_command(
			TARGET "${PROJECT_NAME}" POST_BUILD
			DEPENDS "${PROJECT_NAME}"
			COMMAND $<$<CONFIG:release>:${CMAKE_STRIP}>
			ARGS --strip-all $<TARGET_FILE:${PROJECT_NAME}>
		)
		# 3. Link the .dbg file back for debugging tools
        add_custom_command(
            TARGET "${PROJECT_NAME}" POST_BUILD
            DEPENDS "${PROJECT_NAME}"
            COMMAND ${CMAKE_OBJCOPY} --add-gnu-debuglink="$<TARGET_FILE:${PROJECT_NAME}>.dbg" "$<TARGET_FILE:${PROJECT_NAME}>"
        )
	endif()
elseif(APPLE)
	if (CMAKE_BUILD_TYPE STREQUAL "Release")
		# Generate dSYM bundle
        add_custom_command(
            TARGET "${PROJECT_NAME}" POST_BUILD
            DEPENDS "${PROJECT_NAME}"
            COMMAND dsymutil $<TARGET_FILE:${PROJECT_NAME}> 
			ARGS -o $<TARGET_FILE:${PROJECT_NAME}>.dSYM
        )
        # Strip the executable
		add_custom_command(
			TARGET "${PROJECT_NAME}" POST_BUILD
			DEPENDS "${PROJECT_NAME}"
			COMMAND $<$<CONFIG:release>:${CMAKE_STRIP}>
			ARGS -x $<TARGET_FILE:${PROJECT_NAME}>
		)
	endif()
endif()