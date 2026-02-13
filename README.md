# Snake3D_public

A multi-platform 3D game and custom engine for iOS/Android/PC/VR

## Building

### Windows

### macOS

An xcode project is available in the PC directory.

### Linux

A build file will be added soon.

### Android

An Android Studio project is available in the Android directory.
A soft link named lib should be in the directory Android/app/src/main/cpp that points to the lib directory in the root project directory.
In case this soft link is not created automatically when cloning, you should create it by going to Android/app/src/main/cpp and running:

In macos and linux:
```sh
ln -s ../../../../../lib lib
```
In windows:
```cmd
mklink /D lib ../../../../../lib
```
 
### iOS

An xcode project is available in the iOS directory.

### VR versions

Support files for SteamVR and for meta quest devices will be added soon. 
