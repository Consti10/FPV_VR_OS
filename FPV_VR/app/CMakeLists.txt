cmake_minimum_required(VERSION 3.4.1)


find_library( log-lib
              log )

######################## This is for the gvr (google vr) library #########################
set(GVR_DIR ${CMAKE_SOURCE_DIR}/libs/google/gvr)

add_library( gvr-lib
             SHARED
             IMPORTED )
set_target_properties( # Specifies the target library.
                       gvr-lib
                       # Specifies the parameter you want to define.
                       PROPERTIES IMPORTED_LOCATION
                       # Provides the path to the library you want to import.
                       ${GVR_DIR}/${ANDROID_ABI}/libgvr.so )
include_directories(${GVR_DIR})
###########################################################################################

include_directories(src/main/cpp/glm)
include_directories(src/main/cpp/GLPrograms)
######################################include_directories(src/main/cpp/Helper)
include_directories(src/main/cpp/Network)
include_directories(src/main/cpp/OSD)
include_directories(src/main/cpp/Other)
include_directories(src/main/cpp/Telemetry)
######################################include_directories(src/main/cpp/Telemetry/cFiles)
include_directories(src/main/cpp/Video)

###########################
#OTHER
##########################
add_library( SettingsN
             SHARED
             src/main/cpp/SettingsN.cpp )
target_link_libraries( SettingsN
                       #${log-lib}
                       android
                       log)
add_library( Chronometer
             SHARED
             src/main/cpp/Other/Chronometer.cpp )
target_link_libraries( Chronometer
                       ${log-lib}
                       android
                       )
add_library( FrameCPUChronometer
             SHARED
             src/main/cpp/Other/FrameCPUChronometer.cpp )
target_link_libraries( FrameCPUChronometer
                       ${log-lib}
                       android
                       log)
add_library( FPSCalculator
             SHARED
             src/main/cpp/Other/FPSCalculator.cpp )
target_link_libraries( FPSCalculator
                       ${log-lib}
                       android
                       log)
add_library( FBRManager
             SHARED
             src/main/cpp/Other/FBRManager.cpp )
target_link_libraries( FBRManager
                       ${log-lib}
                       android
                       Chronometer
                       FrameCPUChronometer
                       log
                       EGL
                       GLESv2)

add_library( HeadTrackerExtended
             SHARED
             src/main/cpp/Other/HeadTrackerExtended.cpp )
target_link_libraries( HeadTrackerExtended
                       ${log-lib}
                       android
                       SettingsN
                       gvr-lib)
##########################################################################################################
#GLPrograms
##########################################################################################################
add_library( GLRenderGeometry
             SHARED
             src/main/cpp/GLPrograms/GLRenderGeometry.cpp )
target_link_libraries( GLRenderGeometry
                       ${log-lib}
                       android
                       log
                       GLESv2)
add_library( GLRenderLine
             SHARED
             src/main/cpp/GLPrograms/GLRenderLine.cpp )
target_link_libraries( GLRenderLine
                       ${log-lib}
                       android
                       log
                       GLESv2)
add_library( GLRenderTexture
             SHARED
             src/main/cpp/GLPrograms/GLRenderTexture.cpp )
target_link_libraries( GLRenderTexture
                       ${log-lib}
                       android
                       log
                       GLESv2)
add_library( GLRenderText
             SHARED
             src/main/cpp/GLPrograms/GLRenderText.cpp )
target_link_libraries( GLRenderText
                       ${log-lib}
                       android
                       log
                       EGL
                       GLESv2)
add_library( GLRenderTextureExternal
             SHARED
             src/main/cpp/GLPrograms/GLRenderTextureExternal.cpp )
target_link_libraries( GLRenderTextureExternal
                       ${log-lib}
                       android
                       log
                       GLESv2)
add_library( GLTextObj
             SHARED
             src/main/cpp/GLPrograms/GLTextObj.cpp )
target_link_libraries( GLTextObj
                       ${log-lib}
                       log)
##########################################################################################################
#Network
##########################################################################################################
add_library( UDPReceiver
             SHARED
             src/main/cpp/Network/UDPReceiver.cpp )
target_link_libraries(UDPReceiver
                      ${log-lib}
                      android
                      log)
##########################################################################################################
#Telemetry
##########################################################################################################
####################
#C-Files
####################
add_library( ltm
             SHARED
             src/main/cpp/Telemetry/cFiles/ltm.c )
target_link_libraries(ltm
                      ${log-lib}
                      android
                      log)
add_library( frsky
             SHARED
             src/main/cpp/Telemetry/cFiles/frsky.c )
target_link_libraries(frsky
                      ${log-lib}
                      android
                      log)
add_library( mavlink
              SHARED
              src/main/cpp/Telemetry/cFiles/mavlink.c )
target_link_libraries(mavlink
                       ${log-lib}
                       android
                       log)
#######################################################
add_library( TelemetryReceiver
             SHARED
             src/main/cpp/Telemetry/TelemetryReceiver.cpp )
target_link_libraries(TelemetryReceiver
                      ${log-lib}
                      android
                      log
                      UDPReceiver
                      SettingsN
                      ltm
                      frsky
                      mavlink)
##########################################################################################################
#OSD Stuff
##########################################################################################################
add_library( CompasLadder
             SHARED
             src/main/cpp/OSD/CompasLadder.cpp )
target_link_libraries( CompasLadder
                       GLRenderText
                       GLRenderLine
                       GLRenderGeometry
                       ${log-lib}
                       android
                       log
                       GLESv2)
add_library( HeightLadder
             SHARED
             src/main/cpp/OSD/HeightLadder.cpp )
target_link_libraries( HeightLadder
                       GLRenderText
                       GLRenderGeometry
                       GLRenderLine
                       ${log-lib}
                       android
                       log
                       GLESv2)
add_library( ModelRollPitch
             SHARED
             src/main/cpp/OSD/ModelRollPitch.cpp )
target_link_libraries( ModelRollPitch
                       GLRenderGeometry
                       ${log-lib}
                       android
                       log
                       GLESv2)
add_library( TextElements
             SHARED
             src/main/cpp/OSD/TextElements.cpp )
target_link_libraries( TextElements
                       TelemetryReceiver
                       GLRenderText
                       GLTextObj
                       GLRenderGeometry
                       ${log-lib}
                       android
                       log
                       GLESv2)
add_library( OSDRenderer
             SHARED
             src/main/cpp/OSD/OSDRenderer.cpp )
target_link_libraries( OSDRenderer
                       TelemetryReceiver
                       GLRenderGeometry
                       GLRenderLine
                       GLRenderText
                       GLTextObj
                       ModelRollPitch
                       CompasLadder
                       HeightLadder
                       TextElements
                       SettingsN
                       ${log-lib}
                       android
                       log
                       EGL
                       GLESv2)
##########################################################################################################
#Video
##########################################################################################################
add_library( LowLagDecoder
             SHARED
             src/main/cpp/Video/LowLagDecoder.cpp )
target_link_libraries(LowLagDecoder
                       ${log-lib}
                       log
                       android
                       mediandk
                       OpenMAXAL)
add_library( VideoRenderer
             SHARED
             src/main/cpp/Video/VideoRenderer.cpp )
target_link_libraries(VideoRenderer
                      GLRenderGeometry
                      GLRenderTextureExternal
                       ${log-lib}
                       android
                       log
                       EGL
                       GLESv2)
add_library( VideoPlayerN
             SHARED
             src/main/cpp/Video/VideoPlayerN.cpp )
target_link_libraries(VideoPlayerN
                      LowLagDecoder
                      UDPReceiver
                      TelemetryReceiver
                      ${log-lib}
                      android
                      log)
##########################################################################################################
#Renderer Mono/Stereo (called from GLSurfaceView.Renderer)
##########################################################################################################

add_library( GLRendererMono
             SHARED
             src/main/cpp/GLRenderer/GLRendererMono.cpp )
target_link_libraries( GLRendererMono
                       GLRenderGeometry
                       GLRenderLine
                       GLRenderText
                       GLRenderTexture
                       OSDRenderer
                       TelemetryReceiver
                       Chronometer
                       SettingsN
                       ${log-lib}
                       android
                       log
                       EGL
                       GLESv2
                       gvr-lib
                       FPSCalculator
                       )

add_library( GLRendererStereo
             SHARED
             src/main/cpp/GLRenderer/GLRendererStereo.cpp )
target_link_libraries( GLRendererStereo
                       GLRenderGeometry
                       GLRenderLine
                       GLRenderText
                       GLRenderTexture
                       GLRenderTextureExternal
                       OSDRenderer
                       VideoRenderer
                       TelemetryReceiver
                       Chronometer
                       SettingsN
                       ${log-lib}
                       android
                       log
                       EGL
                       GLESv2
                       #gvr-lib
                       HeadTrackerExtended
                       FPSCalculator)

add_library( GLRSuperSync
             SHARED
             src/main/cpp/GLRenderer/GLRSuperSync.cpp )
target_link_libraries( GLRSuperSync
                       GLRenderGeometry
                       GLRenderLine
                       GLRenderText
                       GLRenderTexture
                       GLRenderTextureExternal
                       OSDRenderer
                       VideoRenderer
                       TelemetryReceiver
                       Chronometer
                       FBRManager
                       SettingsN
                       ${log-lib}
                       android
                       log
                       EGL
                       GLESv2
                       #gvr-lib
                       HeadTrackerExtended
                       FPSCalculator)
############################################################
## OTHER 2
############################################################
add_library( TestReceiverN
             SHARED
             src/main/cpp/Other/TestReceiverN.cpp )
target_link_libraries( TestReceiverN
                       UDPReceiver
                       TelemetryReceiver
                       ${log-lib}
                       android
                       log)