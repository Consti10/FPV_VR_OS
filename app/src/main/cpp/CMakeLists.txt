cmake_minimum_required(VERSION 3.6)

find_library( log-lib
        log )

##########################################################################################################
# Add the native part of all the external modules (e.g. the cpp part)
##########################################################################################################
# TelemetryCore
include(${T_CORE_DIR}/TelemetryCore.cmake)
# RenderingXCore
include(${RX_CORE_DIR}/RenderingXCore.cmake)
# VideoCore is not needed

##########################################################################################################
# Scene (Engine) Stuff (OSD and video rendering)
##########################################################################################################
set(SCENE_PATH Scene)
include_directories(${SCENE_PATH})
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
target_link_libraries( Scene GLPrograms TelemetryReceiver ${log-lib} android EGL GLESv2 GLESv3 Time)
include_directories(${SCENE_PATH}/OSD/Settings)

##########################################################################################################
# Renderer Mono/Stereo (called from GLSurfaceView.Renderer)
##########################################################################################################
set(VR_SETTINGS_PATH VRSettings)
include_directories(${VR_SETTINGS_PATH})
add_library(VRSettings SHARED
        ${VR_SETTINGS_PATH}/SettingsVR.cpp)
target_link_libraries(VRSettings android log GLPrograms)

set(GLR_SOME_DEPENDENCIES VRSettings GLPrograms Scene TelemetryReceiver Time)

add_library( GLRMono SHARED
        GLRenderer/GLRMono.cpp)
target_link_libraries( GLRMono ${GLR_SOME_DEPENDENCIES} )

add_library( GLRStereoNormal SHARED
        GLRenderer/GLRStereoNormal.cpp
        )
target_link_libraries( GLRStereoNormal ${GLR_SOME_DEPENDENCIES} )

add_library( GLRStereoSuperSync SHARED
        GLRenderer/GLRStereoSuperSync.cpp
        GLRenderer/GLRStereoNormal.cpp
        )
target_link_libraries( GLRStereoSuperSync ${GLR_SOME_DEPENDENCIES} SuperSync)

add_library( GLRStereoDaydream SHARED
        ${RX_CORE_DIR}/src/main/cpp/Other/gvr_util/util.cc
        GLRenderer/GLRStereoDaydream.cpp)
target_link_libraries( GLRStereoDaydream ${GLR_SOME_DEPENDENCIES} )

