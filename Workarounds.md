# A collection of workaround information related to ocran

Workarounds with technical behind the scenes information. Maybe not necessary but good to know.

## 2022-04-28 Ruby 3.1 Update with sqlite3:

https://github.com/larsch/ocra/issues/174

call ridk exec pacman -S mingw-w64-x86_64-sqlite3 --noconfirm
copy C:\Ruby31-x64\msys64\mingw64\bin\libiconv-2.dll C:\Ruby31-x64\bin
copy C:\Ruby31-x64\msys64\mingw64\bin\libsqlite3-0.dll C:\Ruby31-x64\bin
ridk.cmd exec pacman -S mingw-w64-x86_64-ruby3.0.3-2 --noconfirm
copy C:\Ruby31-x64\msys64\mingw64\lib\ruby\3.0.0\x64-mingw32\fiber.so C:\Ruby31-x64\lib\ruby\3.1.0\x64-mingw-ucrt

ocra test.rb --dll libiconv-2.dll --dll libsqlite3-0.dll --dll ruby_builtin_dlls/libgmp-10.dll --dll ruby_builtin_dlls/libffi-7.dll --dll ruby_builtin_dlls/zlib1.dll --dll ruby_builtin_dlls/libssp-0.dll --dll ruby_builtin_dlls/libssl-1_1-x64.dll --dll ruby_builtin_dlls/libcrypto-1_1-x64.dll --dll ruby_builtin_dlls/libyaml-0-2.dll --dll ruby_builtin_dlls/libwinpthread-1.dll  --dll ruby_builtin_dlls/libgcc_s_seh-1.dll

Maximalist Example with Innosetup, OpenSSL and tzinfo-data Gems:

ocra test.rb --chdir-first --no-lzma --innosetup setup.iss --gem-full --add-all-core --icon  iconfile.ico --output test.exe --dll ruby_builtin_dlls/libgmp-10.dll --dll ruby_builtin_dlls/libffi-7.dll --dll ruby_builtin_dlls/zlib1.dll --dll ruby_builtin_dlls/libssp-0.dll --dll ruby_builtin_dlls/libssl-1_1-x64.dll --dll ruby_builtin_dlls/libcrypto-1_1-x64.dll --dll ruby_builtin_dlls/libyaml-0-2.dll --dll ruby_builtin_dlls/libwinpthread-1.dll  --dll libiconv-2.dll --dll libsqlite3-0.dll  --dll ruby_builtin_dlls/libgcc_s_seh-1.dll --gem-full=openssl --gem-full=tzinfo-data

TinyTds Users:
ridk exec pacman -S mingw-w64-ucrt-x86_64-freetds --noconfirm
gem install tiny_tds --with-freetds-include=C:\Ruby31-x64\msys64\ucrt64\include\freetds
copy C:\Ruby31-x64\msys64\ucrt64\bin\libsybdb-5.dll C:\Ruby31-x64\bin
Add the following flag:  --dll libsybdb-5.dll 

How I figured out which dll was missing?
Open Sysinternals Process Explorer and compare ocra and regular ruby process > View > Show Lower Pane , Lower Pane View > DLLs


## Ruby 3.0 workarounds

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


## Tinytds missing dll

Strange problem where tiny_tds.so is there but "the module cannot be found" error appears.
lib/ruby/gems/3.2.0/gems/tiny_tds-2.1.5/lib/tiny_tds/tiny_tds.so (LoadError)

I compared the used dlls with procmon but could not figure it out.

I first created an exe file which extracts itself using ocran --debug-extract and then
checked the directory and added the files from my ruby installation to the folder.
By adding and removing files in the ocr1234.tmp/msys64 folder I could get the program to work
It turned out that the /msys64/usr/bin/msys-2.0.dll file was required...

ruby .\bin\ocran .\samples\tiny_tds.rb --dll ../msys64/usr/bin/msys-2.0.dll

After some research, I learned that this file is a big compatibility library from msys.
It should not be necessary, but I don't see an easy way to avoid it for now.
Or rubyinstaller might check for the existence of the file?
https://github.com/oneclick/rubyinstaller2/blob/rubyinstaller-2.5.1-1/lib/ruby_installer/build/msys2_installation.rb#L32-L69

lib\ruby\site_ruby\3.2.0\ruby_installer\runtime

https://stackoverflow.com/questions/37524839/msys2-statically-link-output-binary

[Add compat with RubyInstaller-2.4.1 #368](https://github.com/rails-sqlserver/tiny_tds/pull/368)