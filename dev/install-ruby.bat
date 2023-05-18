@ECHO ON
for %%I in ("%~dp0.") do for %%J in ("%%~dpI.") do set ParentFolderName=%%~dpnxJ
cd %ParentFolderName%/tasks
powershell Set-ExecutionPolicy Bypass -Scope Process -Force; [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072; iex ((New-Object System.Net.WebClient).DownloadString('https://community.chocolatey.org/install.ps1'))  && SET "PATH=%PATH%;%ALLUSERSPROFILE%\chocolatey\bin"
cmd.exe /C choco install msys2 -y
cmd.exe /C choco install ruby --version=3.1 -y
cmd.exe /C choco install git.install -y --params "/GitAndUnixToolsOnPath /NoGitLfs /SChannel" 
cmd.exe /C choco install innosetup --version=6.2.1 -y
cmd.exe /C choco install vscode -y
cmd.exe /C refreshenv
cmd.exe /C  powershell [Environment]::SetEnvironmentVariable('PATH', $env:PATH + ';c:/Program Files/Git/bin', 'Machine');
cmd.exe /C  powershell [Environment]::SetEnvironmentVariable('PATH', $env:PATH + ';c:/Program Files (x86)/Inno Setup 6', 'Machine');
cmd.exe /C  powershell Get-Command "iscc";
cmd.exe /C powershell $env:PATH = [Environment]::GetEnvironmentVariable('PATH','Machine'); $env:RUBYOPT = [Environment]::GetEnvironmentVariable('RUBYOPT','Machine'); ruby --version;
cmd.exe /C (ridk install 1) &&  (cmd.exe /C ridk install 3)  && (cmd.exe /C ridk enable) 
cmd.exe /C ridk exec sed -i 's/Required DatabaseOptional/Never/' /etc/pacman.conf
cmd.exe /C ridk exec pacman -S msys2-keyring --noconfirm
cmd.exe /C first_setup.bat
cmd.exe /C install_gems.bat
pause
