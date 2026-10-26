# syntax=docker/dockerfile:1.6

ARG MOD_ID
ARG CPM_CACHE_DIR
ARG SDK_VERSION=latest
ARG BINDINGS=geode-sdk/bindings
ARG GIT_INVALIDATE_CLONE=0

FROM prevter/geode-sdk:android-${SDK_VERSION} AS build-repositories
ARG BINDINGS
ARG GIT_INVALIDATE_CLONE

WORKDIR /workspace

RUN echo "Cloning Bindings (invalidate=${GIT_INVALIDATE_CLONE})" && \
    git clone https://github.com/${BINDINGS} --depth=1 bindings

WORKDIR /workspace/bindings/codegen

RUN --mount=type=cache,target=/workspace/cpm-cache \
    --mount=type=cache,target=/workspace/project/build,id=android32 \
    cmake -G Ninja -B build \
    -DCPM_SOURCE_CACHE=/workspace/cpm-cache \
    && cmake --build build --parallel \
    && cp build/Codegen /workspace/


FROM prevter/geode-sdk:android-${SDK_VERSION} AS build-android32
ARG MOD_ID
ARG CPM_CACHE_DIR

COPY --from=build-repositories /workspace/bindings /workspace/bindings
COPY --from=build-repositories /workspace/Codegen /workspace/project/build/bindings/codegen/Codegen

WORKDIR /workspace/project
COPY . .

RUN --mount=type=cache,target=/workspace/cpm-cache \
    --mount=type=cache,target=/workspace/project/build,id=android32 \
    cmake -G Ninja -B build \
      -DCMAKE_BUILD_TYPE=Release \
      -DCPM_SOURCE_CACHE=/workspace/cpm-cache \
      -DGEODE_BINDINGS_REPO_PATH=/workspace/bindings \
      -DGEODE_DONT_INSTALL_MODS=ON \
      -DCMAKE_TOOLCHAIN_FILE=/opt/android-ndk/build/cmake/android.toolchain.cmake \
      -DANDROID_ABI=armeabi-v7a \
      -DANDROID_PLATFORM=android-23 \
      -DANDROID_STL=c++_shared \
    && cmake --build build --parallel \
    && ls build \
    && cp build/${MOD_ID}.geode /workspace/ \
    && ls build \
    && cp build/${MOD_ID}.android32.debug.so /workspace/

FROM scratch AS export-android32
ARG MOD_ID
COPY --from=build-android32 /workspace/${MOD_ID}.geode .
COPY --from=build-android32 /workspace/${MOD_ID}.android32.debug.so .

# Android 64
FROM prevter/geode-sdk:android-${SDK_VERSION} AS build-android64
ARG MOD_ID
ARG CPM_CACHE_DIR

COPY --from=build-repositories /workspace/bindings /workspace/bindings
COPY --from=build-repositories /workspace/Codegen /workspace/project/build/bindings/codegen/Codegen

WORKDIR /workspace/project
COPY . .

RUN --mount=type=cache,target=/workspace/cpm-cache \
    --mount=type=cache,target=/workspace/project/build,id=android64 \
    cmake -G Ninja -B build \
      -DCMAKE_BUILD_TYPE=Release \
      -DCPM_SOURCE_CACHE=/workspace/cpm-cache \
      -DGEODE_BINDINGS_REPO_PATH=/workspace/bindings \
      -DGEODE_DONT_INSTALL_MODS=ON \
      -DCMAKE_TOOLCHAIN_FILE=/opt/android-ndk/build/cmake/android.toolchain.cmake \
      -DANDROID_ABI=arm64-v8a \
      -DANDROID_PLATFORM=android-23 \
      -DANDROID_STL=c++_shared \
    && cmake --build build --parallel \
    && ls build \
    && cp build/${MOD_ID}.geode /workspace/ \
    && ls build \
    && cp build/${MOD_ID}.android64.debug.so /workspace/

FROM scratch AS export-android64
ARG MOD_ID
COPY --from=build-android64 /workspace/${MOD_ID}.geode .
COPY --from=build-android64 /workspace/${MOD_ID}.android64.debug.so .


FROM prevter/geode-sdk:ios-${SDK_VERSION} AS build-ios
ARG MOD_ID
ARG CPM_CACHE_DIR

COPY --from=build-repositories /workspace/bindings /workspace/bindings
COPY --from=build-repositories /workspace/Codegen /workspace/project/build/bindings/codegen/Codegen

WORKDIR /workspace/project
COPY . .

RUN --mount=type=cache,target=/workspace/cpm-cache \
    --mount=type=cache,target=/workspace/project/build,id=ios \
    cmake -G Ninja -B build \
      -DCMAKE_BUILD_TYPE=Release \
      -DCPM_SOURCE_CACHE=/workspace/cpm-cache \
      -DGEODE_BINDINGS_REPO_PATH=/workspace/bindings \
      -DGEODE_DONT_INSTALL_MODS=ON \
      -DGEODE_IOS_SDK="${CMAKE_OSX_SYSROOT}" \
    && cmake --build build --parallel \
    && ls build \
    && cp build/${MOD_ID}.geode /workspace/ \
    && cp -r build/${MOD_ID}.ios.debug.dylib /workspace/

FROM scratch AS export-ios
ARG MOD_ID
COPY --from=build-ios /workspace/${MOD_ID}.geode .
COPY --from=build-ios /workspace/${MOD_ID}.ios.debug.dylib .


# MacOS
FROM prevter/geode-sdk:macos-${SDK_VERSION} AS build-macos
ARG MOD_ID
ARG CPM_CACHE_DIR

COPY --from=build-repositories /workspace/bindings /workspace/bindings
COPY --from=build-repositories /workspace/Codegen /workspace/project/build/bindings/codegen/Codegen

WORKDIR /workspace/project
COPY . .

RUN --mount=type=cache,target=/workspace/cpm-cache \
    --mount=type=cache,target=/workspace/project/build,id=macos \
    cmake -G Ninja -B build \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_OSX_DEPLOYMENT_TARGET=11.0 \
      -DCPM_SOURCE_CACHE=/workspace/cpm-cache \
      -DGEODE_BINDINGS_REPO_PATH=/workspace/bindings \
      -DGEODE_DONT_INSTALL_MODS=ON \
    && cmake --build build --parallel \
    && cp build/${MOD_ID}.geode /workspace/ \
    && cp -r build/${MOD_ID}.debug.dylib /workspace/

FROM scratch AS export-macos
ARG MOD_ID
COPY --from=build-macos /workspace/${MOD_ID}.geode .
COPY --from=build-macos /workspace/${MOD_ID}.debug.dylib .

# Windows
FROM prevter/geode-sdk:windows-${SDK_VERSION} AS build-windows
ARG MOD_ID
ARG CPM_CACHE_DIR

COPY --from=build-repositories /workspace/bindings /workspace/bindings
COPY --from=build-repositories /workspace/Codegen /workspace/project/build/bindings/codegen/Codegen

WORKDIR /workspace/project
COPY . .

RUN --mount=type=cache,target=/workspace/cpm-cache \
    --mount=type=cache,target=/workspace/project/build,id=windows \
    cmake -G Ninja -B build \
      -DCMAKE_BUILD_TYPE=RelWithDebInfo \
      -DCPM_SOURCE_CACHE=/workspace/cpm-cache \
      -DGEODE_BINDINGS_REPO_PATH=/workspace/bindings \
      -DCMAKE_TOOLCHAIN_FILE=/root/.local/share/Geode/cross-tools/clang-msvc-sdk/clang-msvc.cmake \
      -DSPLAT_DIR=/root/.local/share/Geode/cross-tools/splat \
      -DHOST_ARCH=x64 \
      -DGEODE_DONT_INSTALL_MODS=ON \
    && cmake --build build --parallel \
      && cp build/${MOD_ID}.geode /workspace/ \
      && cp build/${MOD_ID}.pdb /workspace/

FROM scratch AS export-windows
ARG MOD_ID
COPY --from=build-windows /workspace/${MOD_ID}.geode .
COPY --from=build-windows /workspace/${MOD_ID}.pdb .

# Combine
FROM prevter/geode-sdk:windows-${SDK_VERSION} AS build-combine
ARG MOD_ID

WORKDIR /workspace

COPY --from=export-windows /${MOD_ID}.geode ${MOD_ID}.geode
COPY --from=export-android32 /${MOD_ID}.geode ${MOD_ID}-a32.geode
COPY --from=export-android64 /${MOD_ID}.geode ${MOD_ID}-a64.geode
COPY --from=export-macos /${MOD_ID}.geode ${MOD_ID}-mac.geode
COPY --from=export-ios /${MOD_ID}.geode ${MOD_ID}-ios.geode

RUN geode package merge ${MOD_ID}.geode ${MOD_ID}-a32.geode ${MOD_ID}-a64.geode ${MOD_ID}-mac.geode ${MOD_ID}-ios.geode

FROM debian:bookworm-slim AS export-combine
ARG MOD_ID

WORKDIR /workspace/export
COPY --from=build-combine /workspace/${MOD_ID}.geode .
COPY --from=export-android32 /${MOD_ID}.android32.debug.so .
COPY --from=export-android64 /${MOD_ID}.android64.debug.so .
COPY --from=export-ios /${MOD_ID}.ios.debug.dylib .
COPY --from=export-macos /${MOD_ID}.debug.dylib .
COPY --from=export-windows /${MOD_ID}.pdb .

WORKDIR /
COPY extract.sh .
RUN chmod +x /extract.sh
ENTRYPOINT ["/extract.sh"]
