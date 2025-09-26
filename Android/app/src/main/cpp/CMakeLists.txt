# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

cmake_minimum_required(VERSION 3.22.1)

project("snake3d")

# Creates your game shared library. The name must be the same as the
# one used for loading in your Kotlin/Java or AndroidManifest.txt files.
add_library(snake3d SHARED
        main.cpp
        AndroidOut.cpp
        Renderer.cpp
        Shader.cpp
        TextureAsset.cpp
        Utility.cpp
        PlatformAndroid.cpp
        lib/BaseGame.cpp
        lib/Matrix.cpp
        lib/Vector.cpp
        lib/SnakeGame.cpp
        lib/SnakeRenderer.cpp
        lib/Entity.cpp
        lib/Ball.cpp
        lib/Bullet.cpp
        lib/Car.cpp
        lib/Door.cpp
        lib/Level.cpp
        lib/Plane.cpp
        lib/Player.cpp
        lib/Prize.cpp
        lib/Sphere.cpp
        lib/Tail.cpp
        lib/TakableObject.cpp
        lib/Wall.cpp)

# Searches for a package provided by the game activity dependency
find_package(game-activity REQUIRED CONFIG)

# Forces the linker to keep the JNI entry point for GameActivity
set(CMAKE_SHARED_LINKER_FLAGS
        "${CMAKE_SHARED_LINKER_FLAGS} -u Java_com_google_androidgamesdk_GameActivity_initializeNativeCode")

# Configure libraries CMake uses to link your target library.
target_link_libraries(snake3d
        # The game activity
        game-activity::game-activity_static

        # EGL and other dependent libraries required for drawing
        # and interacting with Android system
        EGL
        GLESv3
        jnigraphics
        android
        log)