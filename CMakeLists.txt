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
    src/compat/Globed.cpp
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

CPMAddPackage("gh:GlobedGD/argon@1.5.1")

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
			  target_link_options(${PROJECT_NAME} PRIVATE
   			    "-Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/exports.android.map"
    		)

			  string(REPLACE ".so" ".debug.so"
  					EDITOR_COLLAB_DEBUG_SUFFIX
  					${GEODE_MOD_BINARY_SUFFIX}
				)

	      add_custom_command(
            TARGET "${PROJECT_NAME}" POST_BUILD
            DEPENDS "${PROJECT_NAME}"
            COMMAND cp $<TARGET_FILE:${PROJECT_NAME}>
            ARGS "$<TARGET_FILE_BASE_NAME:${PROJECT_NAME}>${EDITOR_COLLAB_DEBUG_SUFFIX}"
        )

    		add_custom_command(
       			TARGET "${PROJECT_NAME}" POST_BUILD
       			DEPENDS "${PROJECT_NAME}"
       			COMMAND $<$<CONFIG:release>:${CMAKE_STRIP}>
       			ARGS --strip-all $<TARGET_FILE:${PROJECT_NAME}>
    		)
    endif()
elseif(APPLE)
  	if (CMAKE_BUILD_TYPE STREQUAL "Release")
        target_link_options(${PROJECT_NAME} PRIVATE
     			  "-Wl,-exported_symbols_list,${CMAKE_CURRENT_SOURCE_DIR}/exports.darwin.map"
    		)

        string(REPLACE ".dylib" ".debug.dylib"
  					EDITOR_COLLAB_DEBUG_SUFFIX
  					${GEODE_MOD_BINARY_SUFFIX}
				)

        add_custom_command(
            TARGET "${PROJECT_NAME}" POST_BUILD
            DEPENDS "${PROJECT_NAME}"
            COMMAND cp $<TARGET_FILE:${PROJECT_NAME}>
            ARGS "$<TARGET_FILE_BASE_NAME:${PROJECT_NAME}>${EDITOR_COLLAB_DEBUG_SUFFIX}"
        )

    		add_custom_command(
       			TARGET "${PROJECT_NAME}" POST_BUILD
       			DEPENDS "${PROJECT_NAME}"
       			COMMAND $<$<CONFIG:release>:${CMAKE_STRIP}>
       			ARGS -x $<TARGET_FILE:${PROJECT_NAME}>
    		)
  	endif()
endif()
