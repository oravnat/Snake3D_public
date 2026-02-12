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
        lib/Vector.cpp
        lib/Matrix.cpp
        lib/SnakeRenderer.cpp
        lib/BaseGame.cpp
        lib/examples/SnakeGame/SnakeGame.cpp
        lib/examples/SnakeGame/Entity.cpp
        lib/examples/SnakeGame/Ball.cpp
        lib/examples/SnakeGame/Bullet.cpp
        lib/examples/SnakeGame/Car.cpp
        lib/examples/SnakeGame/Door.cpp
        lib/examples/SnakeGame/Level.cpp
        lib/examples/SnakeGame/Plane.cpp
        lib/examples/SnakeGame/Player.cpp
        lib/examples/SnakeGame/Prize.cpp
        lib/examples/SnakeGame/Sphere.cpp
        lib/examples/SnakeGame/Tail.cpp
        lib/examples/SnakeGame/TakableObject.cpp
        lib/examples/SnakeGame/Wall.cpp)

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


target_include_directories(snake3d
        PRIVATE
        ${PROJECT_SOURCE_DIR}/lib
)
