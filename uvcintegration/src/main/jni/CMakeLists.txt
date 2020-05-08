
add_library( HELLO
        SHARED
        hello.cpp)

target_link_libraries( HELLO
        ${log-lib}
        log
        android
        mediandk
        OpenMAXAL)