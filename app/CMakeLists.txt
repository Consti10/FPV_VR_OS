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
#we just include and buld here the files from RenderingXCore directly,except glm and gvr
##########################################################################################################
#RX_CORE_DIR is passed as an argument in app/build.gradle
#set(RX_CORE_DIR ${CMAKE_SOURCE_DIR}/../../RenderingX/RenderingXCore)
#first, get all the external libraries that come with RenderingXCore - glm and gvr
#glm is header only, we only need to provide the path
include_directories(${RX_CORE_DIR}/libs/glm)
include_directories(${RX_CORE_DIR}/src/main/cpp)
include_directories(${RX_CORE_DIR}/src/main/cpp/Color)
include_directories(${RX_CORE_DIR}/src/main/cpp/Other)
include_directories(${RX_CORE_DIR}/src/main/cpp/Helper)
#gvr is a shared,imported library
set(GVR_DIR ${RX_CORE_DIR}/libs/google/gvr)
include_directories(${GVR_DIR}/headers)
add_library( gvr-lib SHARED IMPORTED )
set_target_properties(gvr-lib PROPERTIES IMPORTED_LOCATION
        ${GVR_DIR}/jni/${ANDROID_ABI}/libgvr.so )

#now build our libraries
#Time library
include_directories(${RX_CORE_DIR}/src/main/cpp/Time)
add_library( Time SHARED
        ${RX_CORE_DIR}/src/main/cpp/Time/Chronometer.cpp
        ${RX_CORE_DIR}/src/main/cpp/Time/VRFrameCPUChronometer.cpp
        ${RX_CORE_DIR}/src/main/cpp/Time/FPSCalculator.cpp)
target_link_libraries( Time ${log-lib} android)
#SuperSync Library
include_directories(${RX_CORE_DIR}/src/main/cpp/SuperSync)
add_library( SuperSync SHARED
        ${RX_CORE_DIR}/src/main/cpp/SuperSync/FBRManager.cpp)
target_link_libraries( SuperSync ${log-lib} android Time log EGL GLESv2)
#OpenGL Programs
include_directories(${RX_CORE_DIR}/src/main/cpp/GLPrograms)
add_library( GLPrograms SHARED
        ${RX_CORE_DIR}/src/main/cpp/DistortionCorrection/DistortionManager.cpp
        ${RX_CORE_DIR}/src/main/cpp/DistortionCorrection/MLensDistortion.cpp
        ${RX_CORE_DIR}/src/main/cpp/DistortionCorrection/MPolynomialRadialDistortion.cpp
        ${RX_CORE_DIR}/src/main/cpp/DistortionCorrection/VRHeadsetParams.cpp
        ${RX_CORE_DIR}/src/main/cpp/GLPrograms/GLProgramVC.cpp
        ${RX_CORE_DIR}/src/main/cpp/GLPrograms/GLProgramText.cpp
        ${RX_CORE_DIR}/src/main/cpp/GLPrograms/GLProgramTexture.cpp
        ${RX_CORE_DIR}/src/main/cpp/GLPrograms/GLProgramLine.cpp
        ${RX_CORE_DIR}/src/main/cpp/GeometryBuilder/Sphere.cpp
        ${RX_CORE_DIR}/src/main/cpp/GeometryBuilder/GvrSphere.cpp)
target_link_libraries( GLPrograms ${log-lib} android GLESv2 gvr-lib)



##Other stuff (cannot really be generalized)
include_directories(src/main/cpp/Other)
add_library(OtherStuff SHARED
        src/main/cpp/Other/IGLRenderer.cpp
        src/main/cpp/Other/MatricesManager.cpp)
target_link_libraries(OtherStuff android log gvr-lib)

##########################################################################################################
#Scene (Engine) Stuff (OSD and video rendering)
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
        ${SCENE_PATH}/Video/VideoRenderer.cpp
        ${SCENE_PATH}/Settings/SettingsOSDElements.cpp
        ${SCENE_PATH}/Settings/SettingsOSDStyle.cpp)
target_link_libraries( Scene GLPrograms Telemetry-lib ${log-lib} android EGL GLESv2 GLESv3 Time)

##########################################################################################################
#Renderer Mono/Stereo (called from GLSurfaceView.Renderer)
##########################################################################################################
include_directories(src/main/cpp/VRSettings)
add_library(VRSettings SHARED
        src/main/cpp/VRSettings/SettingsVR.cpp)
target_link_libraries(VRSettings android log GLPrograms)

set(GLR_SOME_DEPENDENCIES VRSettings OtherStuff GLPrograms Scene Telemetry-lib Time)

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

