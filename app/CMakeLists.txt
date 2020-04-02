cmake_minimum_required(VERSION 3.6)


find_library( log-lib
        log )
include_directories(libs/other)


##########################################################################################################
#Telemetry receiver is included as an imported libarary, e.g we don't compile it here, but rather use .so and .h files
##########################################################################################################
#T_CORE_DIR is passed as an argument in app/build.gradle
#set(T_CORE_DIR ${CMAKE_SOURCE_DIR}/../../Telemetry/TelemetryCore)
set(T_SOURCE_DIR ${T_CORE_DIR}/src/main/cpp/Telemetry)
set(T_SO_FILES   ${T_CORE_DIR}/jniLibs)
include_directories(${T_SOURCE_DIR})
#hmmm
set(IO_PATH ${V_CORE_DIR}/../VideoTelemetryShared/InputOutput)
set(HELPER_PATH ${V_CORE_DIR}/../VideoTelemetryShared/Helper)
include_directories(${HELPER_PATH})
include_directories(${IO_PATH})
#mm

add_library( Telemetry-lib
        SHARED
        IMPORTED )
set_target_properties( # Specifies the target library.
        Telemetry-lib
        # Specifies the parameter you want to define.
        PROPERTIES IMPORTED_LOCATION
        # Provides the path to the library you want to import.
        ${T_SO_FILES}/${ANDROID_ABI}/libTelemetryReceiver.so )

##########################################################################################################
# compile and add sources of RenderingXCore library
##########################################################################################################
include(${RX_CORE_DIR}/CMakeLists.txt)

##########################################################################################################
# Scene (Engine) Stuff (OSD and video rendering)
##########################################################################################################
include_directories(src/main/cpp/Scene)
set(SCENE_PATH src/main/cpp/Scene)
add_library( Scene
        SHARED
        ${SCENE_PATH}/OSD/ArtificialHorizon/AHorizon.cpp
        ${SCENE_PATH}/OSD/Ladders/CompassLadder.cpp
        ${SCENE_PATH}/OSD/Ladders/AVerticalLadder.cpp
        ${SCENE_PATH}/OSD/Ladders/VLAltitude.cpp
        ${SCENE_PATH}/OSD/Ladders/VLSpeed.cpp
        ${SCENE_PATH}/OSD/ElementBatching/CpuGpuBuff.cpp
        ${SCENE_PATH}/OSD/ElementBatching/BatchingManager.cpp
        ${SCENE_PATH}/OSD/Text/TextElements1.cpp
        ${SCENE_PATH}/OSD/Text/TextElements2.cpp
        ${SCENE_PATH}/OSD/Text/TEWarning.cpp
        ${SCENE_PATH}/OSD/OSDRenderer.cpp
        ${SCENE_PATH}/OSD/Settings/SettingsOSDElements.cpp
        ${SCENE_PATH}/OSD/Settings/SettingsOSDStyle.cpp
        ${SCENE_PATH}/Video/VideoRenderer.cpp)
target_link_libraries( Scene GLPrograms Telemetry-lib ${log-lib} android EGL GLESv2 GLESv3 Time)
include_directories(${SCENE_PATH}/OSD/Settings)

##########################################################################################################
# Renderer Mono/Stereo (called from GLSurfaceView.Renderer)
##########################################################################################################
include_directories(src/main/cpp/VRSettings)
add_library(VRSettings SHARED
        src/main/cpp/VRSettings/SettingsVR.cpp)
target_link_libraries(VRSettings android log GLPrograms)

set(GLR_SOME_DEPENDENCIES VRSettings GLPrograms Scene Telemetry-lib Time)

add_library( GLRMono SHARED
        src/main/cpp/GLRenderer/GLRMono.cpp )
target_link_libraries( GLRMono ${GLR_SOME_DEPENDENCIES} )

add_library( GLRStereoNormal SHARED
        src/main/cpp/GLRenderer/GLRStereoNormal.cpp
        )
target_link_libraries( GLRStereoNormal ${GLR_SOME_DEPENDENCIES} )

add_library( GLRStereoSuperSync SHARED
        src/main/cpp/GLRenderer/GLRStereoSuperSync.cpp
        src/main/cpp/GLRenderer/GLRStereoNormal.cpp
        )
target_link_libraries( GLRStereoSuperSync ${GLR_SOME_DEPENDENCIES} SuperSync)

add_library( GLRStereoDaydream SHARED
        ${RX_CORE_DIR}/src/main/cpp/Other/gvr_util/util.cc
        src/main/cpp/GLRenderer/GLRStereoDaydream.cpp )
target_link_libraries( GLRStereoDaydream ${GLR_SOME_DEPENDENCIES} )

