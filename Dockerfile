FROM debian:oldstable

RUN apt-get update
RUN apt-get install gcc g++ -y
RUN apt-get install git -y
RUN apt-get install curl zip unzip tar -y
RUN apt-get install pkg-config python3 -y

RUN git clone https://github.com/microsoft/vcpkg.git
RUN cd vcpkg; ./bootstrap-vcpkg.sh

ENV VCPKG_ROOT=/vcpkg
ENV PATH=$VCPKG_ROOT:$PATH

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get install xorg-dev libglu1-mesa-dev libxinerama-dev libxcursor-dev -y

RUN apt-get install file -y
RUN apt-get install desktop-file-utils -y

RUN apt-get install pip -y
RUN python3 -m pip install cmake --upgrade
RUN apt-get install ninja-build -y

COPY vcpkg.json ./vcpkg.json
COPY CMakePresets.json ./CMakePresets.json
COPY CMakeListsDocker.txt ./CMakeLists.txt

RUN ./vcpkg/vcpkg install

RUN mkdir src
COPY World ./src/World
COPY engine ./src/engine/
COPY lib ./src/lib/

COPY GameState.h ./src/GameState.h
COPY UseImGui.h ./src/UseImGui.h
COPY UseImGui.cpp ./src/UseImGui.cpp
COPY main.cpp ./src/main.cpp

RUN cmake --preset=default
RUN cmake --build build

COPY appimagetool-x86_64.AppImage ./appimagetool.AppImage
RUN mkdir -p demo.AppDir/usr/bin
RUN cp ./build/imGUIexample ./demo.AppDir/usr/bin/imGUIexample

COPY demo.AppDir/AppRun ./demo.AppDir/AppRun
COPY demo.AppDir/icon.png ./demo.AppDir/icon.png
COPY demo.AppDir/demo.desktop ./demo.AppDir/demo.desktop
COPY assets ./demo.AppDir/assets
COPY shaders ./demo.AppDir/shaders

RUN ./appimagetool.AppImage --appimage-extract-and-run ./demo.AppDir
