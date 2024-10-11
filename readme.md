![Screenshot](/boulder_screen3.jpg?raw=true "Title")

https://rh-galaxy.itch.io/boulder

---
#### Windows

Software:
1. Install Visual Studio
   https://visualstudio.microsoft.com/vs/older-downloads/
   Select "Visual Studio Community 2019"

Building:
1. Open
   build/windows/Boulder.sln with VC2019 or
   newer and upgrade
2. Select build configuration and platform (x86 or x64)
3. Build all (F7)

Running:
1. Run either from the IDE or copy files from "output_directory" to
    /exe/windows_x86 or /exe/windows_x64, where output_directory is
    either Release Debug Release_x64 or Debug_x64 in build/windows/



---
#### Linux

Software:
1. If the build fails, this might help
   > sudo apt-get install build-essential

Building:
1. Enter the package dir build/linux
2. Type 'make all'
   Binaries are copied to exe/linux_x64 on success

Running:
1. Enter the package dir ./exe/linux_x64
2. Run ./boulder
(3. Run ./mapeditor fileandpath.map)



---
#### Mac

Only on Intelx64 or Arm64

Building:
1. Open build/mac/Boulder.xcworkspace with Xcode 15
2. Press menu: "Product/Build For/Build For Profiling" (Release)
               "Product/Build For/Build For Running" (Debug)
   Select all other projects and repeat (next to 'My Mac' text, grey not obvious
   drop down)

Running:
1. Run either from the IDE or copy files from "output_directory" to /exe/mac
   Where output_directory is some hidden directory with a 170 char name default
    (You can change this in an Xcode setting "Xcode/Settings/Locations.Derived Data"
    To get it in "build/mac/DerivedData" for example:
    - Change it to "Relative" and "DerivedData"
    - Click "advanced" and select the Custom "Relative to Derived Data").



---
#### Files

* build
    - linux
        - Makefile - to build on linux
        - virtualbox.txt - describes how to setup, build and run on a virtual machine (running linux)
    - mac
        - XCode 15 project files - to build and run on mac
    - windows       
        - VS project files - to build and run on windows
        - build_release_files.bat - to build the release zip packages

* exe
    - executable files for windows, linux and mac
    - data folder is common for all versions

* ext_include
    - external includes, with the exception of mac/fs*.* files

* lib
    - external libs for windows, linux and mac

* original_gfx
    - source files for gfx

* utils
    - fnt_comp - font compiler project source files (uses opengl and builds on windows and linux)
    - img_cut - image cutter project source files
    - res_comp - resource compiler project source files
    - zip_exec - zip executable flag fix project source files

* src
    - common - common source files for all projects, also includes gui.cpp/h and tileset.cpp/h which are used in both game and editor and depends on other code
    - graph - common source files for handling graphics
    - editor - editor project source files
    - boulder - game project source files
