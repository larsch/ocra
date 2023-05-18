@ECHO ON
for %%I in ("%~dp0.") do for %%J in ("%%~dpI.") do set ParentFolderName=%%~dpnxJ
echo %ParentFolderName%
call cd /D %ParentFolderName%
call git config --global --add safe.directory *
call gem update --silent --system --no-document
call gem install bundler -v 2.3.17 --no-doc
@ECHO ON
call ridk exec pacman -S mingw-w64-x86_64-sqlite3 --noconfirm
call ridk exec pacman -S mingw-w64-x86_64-freetds --noconfirm
call ridk exec pacman -S mingw-w64-ucrt-x86_64-freetds --noconfirm
call copy C:\Ruby32-x64\msys64\mingw64\bin\libiconv-2.dll C:\Ruby32-x64\bin
call copy C:\Ruby32-x64\msys64\mingw64\bin\libsqlite3-0.dll C:\Ruby32-x64\bin
call ridk exec pacman -S mingw-w64-x86_64-ruby --noconfirm
call copy C:\Ruby32-x64\msys64\mingw64\lib\ruby\3.0.0\x64-mingw32\fiber.so C:\Ruby32-x64\lib\ruby\3.1.0\x64-mingw-ucrt
call gem install tiny_tds -- --with-freetds-include=C:\Ruby32-x64\msys64\ucrt64\include\freetds
call bundle config build.tiny_tds --with-freetds-include=C:\Ruby32-x64\msys64\ucrt64\include\freetds
call copy C:\Ruby32-x64\msys64\ucrt64\bin\libsybdb-5.dll C:\Ruby32-x64\bin