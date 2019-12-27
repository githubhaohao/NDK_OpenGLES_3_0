# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

set(jnilibs "${CMAKE_SOURCE_DIR}/../jniLibs")


include_directories(
        util
        render
        sample
        glm
        inc
        inc/assimp
        model
        opencv_3_0_0
)

#add_library(cryptopp STATIC IMPORTED )
#set_target_properties(cryptopp PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/libs/libcryptopp.a)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

add_library( # Sets the name of the library.
             native-render

             # Sets the library as a shared library.
             SHARED

             # Provides a relative path to your source file(s).
        JniImpl.cpp
        render/MyGLRenderContext.cpp
        render/EGLRender.cpp
        util/GLUtils.cpp
        sample/TriangleSample.cpp
        sample/TextureMapSample.cpp
        sample/NV21TextureMapSample.cpp
        sample/VaoSample.cpp
        sample/FBOSample.cpp
        sample/FBOLegLengthenSample.cpp
        sample/CoordSystemSample.cpp
        sample/BasicLightingSample.cpp
        sample/TransformFeedbackSample.cpp
        sample/MultiLightsSample.cpp
        sample/DepthTestingSample.cpp
        sample/InstancingSample.cpp
        sample/Instancing3DSample.cpp
        sample/StencilTestingSample.cpp
        sample/BlendingSample.cpp
        sample/ParticlesSample.cpp
        sample/Noise3DSample.cpp
        sample/SkyBoxSample.cpp
        sample/Model3DSample.cpp
        sample/PBOSample.cpp
              )

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
              log-lib

              # Specifies the name of the NDK library that
              # you want CMake to locate.
              log )

add_library(assimp SHARED IMPORTED )
set_target_properties(assimp PROPERTIES IMPORTED_LOCATION ${jnilibs}/${ANDROID_ABI}/libassimp.so)

add_library(opencv_java3 SHARED IMPORTED )
set_target_properties(opencv_java3 PROPERTIES IMPORTED_LOCATION ${jnilibs}/${ANDROID_ABI}/libopencv_java3.so)

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
                       native-render

                       # Links the target library to the log library
                       # included in the NDK.
                       ${log-lib}
                       assimp
                       opencv_java3
                       EGL
                       GLESv3
                       )