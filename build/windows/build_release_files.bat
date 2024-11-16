
REM this creates a zip file for each release platform (exe) and one for the source (all files).
REM previously 'build_release_files_exclude.txt' was set to exclude files from the xcopy cmd,
REM  but there needed to be too many hacks to get all files copied, also there was an uncertainty
REM  if files would be copied or not after adding new files, so now all files are specified here
REM  instead. This means that when new files/dirs are added to the project, lines may need to be
REM  added here.
REM a directory containing all files and the resulting zip files will be created in /release_packages.

REM the executable flag will now be set in the packages with a utility i've made to make files valid
REM  directly after extracting on linux and mac


REM ---------------------------------------------------------------------------
REM path setup

REM normally the 2 lines below is the only things that need editing
SET sevenzip="c:\Program Files\7-Zip\7z.exe"
SET ver=1.01

SET dest_path=..\..\release_packages
SET path_source=%dest_path%\boulder_%ver%_src
SET path_source_plain=%dest_path%\boulder_%ver%_src_plain
SET path_windows_bin=%dest_path%\boulder_%ver%_windows_bin
SET path_linux_bin=%dest_path%\boulder_%ver%_linux_bin
SET path_mac_bin=%dest_path%\boulder_%ver%_mac_bin

pushd "%~dp0"

md "%dest_path%"
md "%path_source%"
md "%path_windows_bin%"
md "%path_linux_bin%"
REM md "%path_mac_bin%"


REM ---------------------------------------------------------------------------
REM source

REM /
xcopy ..\..\*.md "%path_source%\"
xcopy ..\..\*.txt "%path_source%\"
xcopy ..\..\*.jpg "%path_source%\"
REM /src
md "%path_source%\src"
md "%path_source%\src\common"
md "%path_source%\src\editor"
md "%path_source%\src\boulder"
md "%path_source%\src\graph"
xcopy ..\..\src\common\*.cpp   "%path_source%\src\common\"
xcopy ..\..\src\common\*.h     "%path_source%\src\common\"
xcopy ..\..\src\editor\*.cpp   "%path_source%\src\editor\"
xcopy ..\..\src\editor\*.h     "%path_source%\src\editor\"
xcopy ..\..\src\boulder\*.cpp "%path_source%\src\boulder\"
xcopy ..\..\src\boulder\*.h   "%path_source%\src\boulder\"
xcopy ..\..\src\graph\*.cpp    "%path_source%\src\graph\"
xcopy ..\..\src\graph\*.h      "%path_source%\src\graph\"
REM /ext_include
md "%path_source%\ext_include"
md "%path_source%\ext_include\pa"
md "%path_source%\ext_include\mac"
md "%path_source%\ext_include\sdl"
xcopy ..\..\ext_include\pa\*.h   "%path_source%\ext_include\pa\"
xcopy ..\..\ext_include\pa\*.c   "%path_source%\ext_include\pa\"
xcopy ..\..\ext_include\mac\*.h     "%path_source%\ext_include\mac\"
xcopy ..\..\ext_include\mac\*.cpp   "%path_source%\ext_include\mac\"
xcopy ..\..\ext_include\mac\*.m     "%path_source%\ext_include\mac\"
xcopy ..\..\ext_include\sdl\*.*     "%path_source%\ext_include\sdl\"
REM /lib
md "%path_source%\lib"
md "%path_source%\lib\linux_x64"
md "%path_source%\lib\mac"
md "%path_source%\lib\win_x64"
md "%path_source%\lib\win_x86"
xcopy ..\..\lib\linux_x64\*.so "%path_source%\lib\linux_x64\"
xcopy ..\..\lib\mac\*.a "%path_source%\lib\mac\"
xcopy ..\..\lib\mac\*.dylib "%path_source%\lib\mac\"
xcopy ..\..\lib\win_x64\*.lib "%path_source%\lib\win_x64\"
xcopy ..\..\lib\win_x64\*.dll "%path_source%\lib\win_x64\"
xcopy ..\..\lib\win_x86\*.lib "%path_source%\lib\win_x86\"
xcopy ..\..\lib\win_x86\*.dll "%path_source%\lib\win_x86\"
REM /exe
md "%path_source%\exe"
md "%path_source%\exe\data"
md "%path_source%\exe\linux_x64"
md "%path_source%\exe\mac"
md "%path_source%\exe\windows_x86"
md "%path_source%\exe\windows_x64"
xcopy ..\..\exe\*.txt "%path_source%\exe\"
xcopy ..\..\exe\data\* "%path_source%\exe\data\"
xcopy ..\..\exe\linux_x64\* "%path_source%\exe\linux_x64\"
xcopy ..\..\exe\windows_x86\* "%path_source%\exe\windows_x86\"
xcopy ..\..\exe\windows_x64\* "%path_source%\exe\windows_x64\"
REM /original_gfx
md "%path_source%\original_gfx"
xcopy ..\..\original_gfx\* "%path_source%\original_gfx\"
REM /build
md "%path_source%\build"
md "%path_source%\build\linux"
md "%path_source%\build\mac"
md "%path_source%\build\mac\Boulder"
md "%path_source%\build\mac\Boulder\Boulder"
md "%path_source%\build\mac\Boulder\Boulder\en.lproj"
md "%path_source%\build\mac\Boulder\Boulder.xcodeproj"
md "%path_source%\build\mac\Boulder.xcworkspace"
md "%path_source%\build\mac\MapEditor"
md "%path_source%\build\mac\MapEditor\MapEditor"
md "%path_source%\build\mac\MapEditor\MapEditor\en.lproj"
md "%path_source%\build\mac\MapEditor\MapEditor.xcodeproj"
md "%path_source%\build\windows"
xcopy ..\..\build\linux\Makefile "%path_source%\build\linux\"
xcopy ..\..\build\linux\virtualbox.txt "%path_source%\build\linux\"
xcopy ..\..\build\mac\Boulder\*.icns "%path_source%\build\mac\Boulder\"
xcopy ..\..\build\mac\Boulder\Boulder\Boulder-Info.plist "%path_source%\build\mac\Boulder\Boulder\"
xcopy ..\..\build\mac\Boulder\Boulder.xcodeproj\project.pbxproj "%path_source%\build\mac\Boulder\Boulder.xcodeproj\"
xcopy ..\..\build\mac\Boulder.xcworkspace\contents.xcworkspacedata "%path_source%\build\mac\Boulder.xcworkspace\"
xcopy ..\..\build\mac\MapEditor\*.icns "%path_source%\build\mac\MapEditor\"
xcopy ..\..\build\mac\MapEditor\MapEditor\MapEditor-Info.plist "%path_source%\build\mac\MapEditor\MapEditor\"
xcopy ..\..\build\mac\MapEditor\MapEditor.xcodeproj\project.pbxproj "%path_source%\build\mac\MapEditor\MapEditor.xcodeproj\"
xcopy ..\..\build\windows\*.bat "%path_source%\build\windows\"
xcopy ..\..\build\windows\*.rc "%path_source%\build\windows\"
xcopy ..\..\build\windows\*.ico "%path_source%\build\windows\"
xcopy ..\..\build\windows\*.sln "%path_source%\build\windows\"
xcopy ..\..\build\windows\zip_exec.exe "%path_source%\build\windows\"
xcopy ..\..\build\windows\Boulder.vcxproj "%path_source%\build\windows\"
xcopy ..\..\build\windows\Boulder.vcxproj.user "%path_source%\build\windows\"
xcopy ..\..\build\windows\Boulder.vcxproj.filters "%path_source%\build\windows\"
xcopy ..\..\build\windows\MapEditor.vcxproj "%path_source%\build\windows\"
xcopy ..\..\build\windows\MapEditor.vcxproj.filters "%path_source%\build\windows\"
xcopy ..\..\build\windows\MapEditor.vcxproj.user "%path_source%\build\windows\"
xcopy ..\..\build\windows\fnt_comp.vcxproj "%path_source%\build\windows\"
xcopy ..\..\build\windows\fnt_comp.vcxproj.filters "%path_source%\build\windows\"
xcopy ..\..\build\windows\img_cut.vcxproj "%path_source%\build\windows\"
xcopy ..\..\build\windows\img_cut.vcxproj.filters "%path_source%\build\windows\"
xcopy ..\..\build\windows\res_comp.vcxproj "%path_source%\build\windows\"
xcopy ..\..\build\windows\zip_exec.vcxproj "%path_source%\build\windows\"
REM /utils
md "%path_source%\utils"
md "%path_source%\utils\fnt_comp"
md "%path_source%\utils\res_comp"
md "%path_source%\utils\img_cut"
md "%path_source%\utils\zip_exec"
xcopy ..\..\utils\fnt_comp\*   "%path_source%\utils\fnt_comp\"
xcopy ..\..\utils\res_comp\*   "%path_source%\utils\res_comp\"
xcopy ..\..\utils\img_cut\*   "%path_source%\utils\img_cut\"
xcopy ..\..\utils\zip_exec\*   "%path_source%\utils\zip_exec\"

%sevenzip% a -mx=9 -mtc=off "%dest_path%\boulder_%ver%_src.zip" "%path_source%"

zip_exec.exe "%dest_path%\boulder_%ver%_src.zip" "boulder_%ver%_src/exe/linux_x64/boulder"
zip_exec.exe "%dest_path%\boulder_%ver%_src.zip" "boulder_%ver%_src/exe/linux_x64/mapeditor"
REM zip_exec.exe "%dest_path%\boulder_%ver%_src.zip" "boulder_%ver%_src/exe/mac/Boulder.app/Contents/MacOS/Boulder"
REM zip_exec.exe "%dest_path%\boulder_%ver%_src.zip" "boulder_%ver%_src/exe/mac/MapEditor.app/Contents/MacOS/MapEditor"

REM ---------------------------------------------------------------------------
REM only source (without libs)

REM /
xcopy ..\..\*.md "%path_source_plain%\"
xcopy ..\..\*.txt "%path_source_plain%\"
xcopy ..\..\*.jpg "%path_source_plain%\"
REM /src
md "%path_source_plain%\src"
md "%path_source_plain%\src\common"
md "%path_source_plain%\src\editor"
md "%path_source_plain%\src\boulder"
md "%path_source_plain%\src\graph"
xcopy ..\..\src\common\*.cpp   "%path_source_plain%\src\common\"
xcopy ..\..\src\common\*.h     "%path_source_plain%\src\common\"
xcopy ..\..\src\editor\*.cpp   "%path_source_plain%\src\editor\"
xcopy ..\..\src\editor\*.h     "%path_source_plain%\src\editor\"
xcopy ..\..\src\boulder\*.cpp "%path_source_plain%\src\boulder\"
xcopy ..\..\src\boulder\*.h   "%path_source_plain%\src\boulder\"
xcopy ..\..\src\graph\*.cpp    "%path_source_plain%\src\graph\"
xcopy ..\..\src\graph\*.h      "%path_source_plain%\src\graph\"
REM /ext_include
md "%path_source_plain%\ext_include"
md "%path_source_plain%\ext_include\pa"
md "%path_source_plain%\ext_include\mac"
md "%path_source_plain%\ext_include\sdl"
xcopy ..\..\ext_include\pa\*.h   "%path_source_plain%\ext_include\pa\"
xcopy ..\..\ext_include\pa\*.c   "%path_source_plain%\ext_include\pa\"
xcopy ..\..\ext_include\mac\*.h     "%path_source_plain%\ext_include\mac\"
xcopy ..\..\ext_include\mac\*.cpp   "%path_source_plain%\ext_include\mac\"
xcopy ..\..\ext_include\mac\*.m     "%path_source_plain%\ext_include\mac\"
xcopy ..\..\ext_include\sdl\*.*     "%path_source_plain%\ext_include\sdl\"
REM /lib
REM removed
REM /exe
md "%path_source%\exe"
md "%path_source%\exe\data"
md "%path_source%\exe\linux_x64"
xcopy ..\..\exe\*.txt "%path_source%\exe\"
xcopy ..\..\exe\data\* "%path_source%\exe\data\"
xcopy ..\..\exe\linux_x64\*.desktop "%path_source%\exe\linux_x64\"
xcopy ..\..\exe\linux_x64\*.png "%path_source%\exe\linux_x64\"
REM /original_gfx
REM removed
REM /build
md "%path_source_plain%\build"
md "%path_source_plain%\build\linux"
md "%path_source_plain%\build\mac"
md "%path_source_plain%\build\mac\Boulder"
md "%path_source_plain%\build\mac\Boulder\Boulder"
md "%path_source_plain%\build\mac\Boulder\Boulder\en.lproj"
md "%path_source_plain%\build\mac\Boulder\Boulder.xcodeproj"
md "%path_source_plain%\build\mac\Boulder.xcworkspace"
md "%path_source_plain%\build\mac\MapEditor"
md "%path_source_plain%\build\mac\MapEditor\MapEditor"
md "%path_source_plain%\build\mac\MapEditor\MapEditor\en.lproj"
md "%path_source_plain%\build\mac\MapEditor\MapEditor.xcodeproj"
md "%path_source_plain%\build\windows"
xcopy ..\..\build\linux\Makefile "%path_source_plain%\build\linux\"
xcopy ..\..\build\linux\virtualbox.txt "%path_source_plain%\build\linux\"
xcopy ..\..\build\mac\Boulder\*.icns "%path_source_plain%\build\mac\Boulder\"
xcopy ..\..\build\mac\Boulder\Boulder\Boulder-Info.plist "%path_source_plain%\build\mac\Boulder\Boulder\"
xcopy ..\..\build\mac\Boulder\Boulder.xcodeproj\project.pbxproj "%path_source_plain%\build\mac\Boulder\Boulder.xcodeproj\"
xcopy ..\..\build\mac\Boulder.xcworkspace\contents.xcworkspacedata "%path_source_plain%\build\mac\Boulder.xcworkspace\"
xcopy ..\..\build\mac\MapEditor\*.icns "%path_source_plain%\build\mac\MapEditor\"
xcopy ..\..\build\mac\MapEditor\MapEditor\MapEditor-Info.plist "%path_source_plain%\build\mac\MapEditor\MapEditor\"
xcopy ..\..\build\mac\MapEditor\MapEditor.xcodeproj\project.pbxproj "%path_source_plain%\build\mac\MapEditor\MapEditor.xcodeproj\"
xcopy ..\..\build\windows\*.bat "%path_source_plain%\build\windows\"
xcopy ..\..\build\windows\*.rc "%path_source_plain%\build\windows\"
xcopy ..\..\build\windows\*.ico "%path_source_plain%\build\windows\"
xcopy ..\..\build\windows\*.sln "%path_source_plain%\build\windows\"
xcopy ..\..\build\windows\zip_exec.exe "%path_source_plain%\build\windows\"
xcopy ..\..\build\windows\Boulder.vcxproj "%path_source_plain%\build\windows\"
xcopy ..\..\build\windows\Boulder.vcxproj.user "%path_source_plain%\build\windows\"
xcopy ..\..\build\windows\Boulder.vcxproj.filters "%path_source_plain%\build\windows\"
xcopy ..\..\build\windows\MapEditor.vcxproj "%path_source_plain%\build\windows\"
xcopy ..\..\build\windows\MapEditor.vcxproj.filters "%path_source_plain%\build\windows\"
xcopy ..\..\build\windows\MapEditor.vcxproj.user "%path_source_plain%\build\windows\"
xcopy ..\..\build\windows\fnt_comp.vcxproj "%path_source_plain%\build\windows\"
xcopy ..\..\build\windows\fnt_comp.vcxproj.filters "%path_source_plain%\build\windows\"
xcopy ..\..\build\windows\img_cut.vcxproj "%path_source_plain%\build\windows\"
xcopy ..\..\build\windows\img_cut.vcxproj.filters "%path_source_plain%\build\windows\"
xcopy ..\..\build\windows\res_comp.vcxproj "%path_source_plain%\build\windows\"
xcopy ..\..\build\windows\zip_exec.vcxproj "%path_source_plain%\build\windows\"
REM /utils
md "%path_source_plain%\utils"
md "%path_source_plain%\utils\fnt_comp"
md "%path_source_plain%\utils\res_comp"
md "%path_source_plain%\utils\img_cut"
md "%path_source_plain%\utils\zip_exec"
xcopy ..\..\utils\fnt_comp\*   "%path_source_plain%\utils\fnt_comp\"
xcopy ..\..\utils\res_comp\*   "%path_source_plain%\utils\res_comp\"
xcopy ..\..\utils\img_cut\*   "%path_source_plain%\utils\img_cut\"
xcopy ..\..\utils\zip_exec\*   "%path_source_plain%\utils\zip_exec\"

%sevenzip% a -ttar -so "%dest_path%\boulder_%ver%_src_plain.tar" "%path_source_plain%" | %sevenzip% a -si "%dest_path%\boulder_%ver%_src_plain.tar.gz"

REM ---------------------------------------------------------------------------
REM linux bin (x64)

md "%path_linux_bin%\data"
md "%path_linux_bin%\x64"
xcopy ..\..\exe\linux_x64\* "%path_linux_bin%\x64\"
xcopy ..\..\exe\data\* "%path_linux_bin%\data\"
xcopy ..\..\exe\*.txt "%path_linux_bin%\"

%sevenzip% a -mx=9 -mtc=off "%dest_path%\boulder_%ver%_linux_bin.zip" "%path_linux_bin%"

zip_exec.exe "%dest_path%\boulder_%ver%_linux_bin.zip" "boulder_%ver%_linux_bin/x64/boulder"
zip_exec.exe "%dest_path%\boulder_%ver%_linux_bin.zip" "boulder_%ver%_linux_bin/x64/mapeditor"


REM ---------------------------------------------------------------------------
REM mac bin, must now be put together and zipped on a mac

REM -

REM ---------------------------------------------------------------------------
REM windows bin

md "%path_windows_bin%\data"
xcopy ..\..\exe\windows_x86\* "%path_windows_bin%\"
xcopy ..\..\exe\data\* "%path_windows_bin%\data\"
xcopy ..\..\exe\*.txt "%path_windows_bin%\"

%sevenzip% a -mx=9 -mtc=off "%dest_path%\boulder_%ver%_windows_bin.zip" "%path_windows_bin%"

REM ---------------------------------------------------------------------------
REM creates a windows installer with InnoSetup (cmdline)
REM the iss file should be up to date by using InnoSetup (gui)

REM %innosetup% "boulder.iss"

REM ---------------------------------------------------------------------------
popd
