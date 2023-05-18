# debugging tip: docker run -it --name temp mcr.microsoft.com/powershell:nanoserver-2004 
# define current windows version. cannot be newer than host windows
# windows containers only work on an windows host!

ARG windowsversion=2004 
FROM mcr.microsoft.com/windows/servercore:$windowsversion as windowscontainerbase 
SHELL ["powershell.exe", "-Command"]

# Ruby Version and SHA256 Hash of the file (with 7z by readable on Explorer context menu.)
# find out hash with: powershell Get-FileHash .\path\to\foo.zip 
ENV RUBY_VERSION="3.2.0-1" \ 
    RUBY_SHA256="50AAD95C66C0F9BD28C45709D5F4AEA32EC6FD92403B265BC02EB4C078B3EE3D" \
    GIT_VERSION="2.36.0" \
    GIT_SHA256="5196563BA07031257D972C0B3C2EBD3227D98A40587278E11930DBC2F78D4E69" \
    INNOSETUP_VERSION="6.2.1" \   
    INNOSETUP_SHA256="50D21AAB83579245F88E2632A61B943AD47557E42B0F02E6CE2AFEF4CDD8DEB1"

RUN [Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12; \
    Invoke-WebRequest \
        -UseBasicParsing  \
        -Uri https://github.com/git-for-windows/git/releases/download/v$($env:GIT_VERSION).windows.1/Git-$($env:GIT_VERSION)-64-bit.exe \
        -OutFile gitinstaller.exe;  \
    if ((Get-FileHash gitinstaller.exe -Algorithm sha256).Hash -ne $env:GIT_SHA256) {Write-Host "Hash of Git Installer incorrect"; exit 1} else { \
        Write-Host "SHA OK"; \
        (cmd.exe /C 'gitinstaller.exe /verysilent && del gitinstaller.exe');  \
    };

# check if git is installed correctly
RUN Get-Command "git";

RUN [Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12; \
    Invoke-WebRequest \
        -UseBasicParsing  \
        -Uri https://files.jrsoftware.org/is/6/innosetup-$($env:INNOSETUP_VERSION).exe \
        -OutFile innosetup.exe;  \
    if ((Get-FileHash innosetup.exe -Algorithm sha256).Hash -ne $env:INNOSETUP_SHA256) {Write-Host "Hash of Innosetup Installer wrong"; exit 1} else { \
        Write-Host "SHA OK"; \
        (cmd.exe /C 'innosetup.exe /verysilent /allusers && del innosetup.exe');  \
    };

# Check if innosetup is installed correctly
RUN [Environment]::SetEnvironmentVariable('PATH', $env:PATH + ';c:/Program Files (x86)/Inno Setup 6', 'Machine');
RUN Get-Command "iscc";

# Download base ruby
RUN [Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12; \
    Invoke-WebRequest \
        -UseBasicParsing  \
        -Uri https://github.com/oneclick/rubyinstaller2/releases/download/RubyInstaller-$($env:RUBY_VERSION)/rubyinstaller-devkit-$($env:RUBY_VERSION)-x64.exe \
        -OutFile rubyinstaller.exe;  \
    if ((Get-FileHash rubyinstaller.exe -Algorithm sha256).Hash -ne $env:RUBY_SHA256) {Write-Host "Hash of Ruby Installer wrong"; exit 1} else { \
        Write-Host "SHA OK"; \
        (cmd.exe /C 'rubyinstaller.exe /verysilent /log=install.log && del install.log && del rubyinstaller.exe'); \
    };

SHELL ["cmd.exe", "/C"] # For some reason ridk install 1 hangs at the end if you use powershell. https://github.com/docker/for-win/issues/2565

# initalize devkit and check version
RUN powershell $env:PATH = [Environment]::GetEnvironmentVariable('PATH','Machine'); \
    $env:RUBYOPT = [Environment]::GetEnvironmentVariable('RUBYOPT','Machine'); \
    ruby --version;
RUN (ridk install 1) && (ridk enable)

FROM windowscontainerbase AS ruby_base

# D: create as empty partion in the container
RUN powershell -NoProfile -Command \
    New-Item -ItemType directory -Path C:\drived ; \
    New-ItemProperty -Path 'HKLM:\SYSTEM\CurrentControlSet\Control\Session Manager\DOS Devices' -Name 'D:'  -Value '\??\C:\drived' -PropertyType String;

FROM ruby_base AS exampleproject_base

FROM exampleproject_base AS exampleproject_dev

RUN mkdir D:\exampleproject\build

SHELL ["cmd.exe", "/C"] 
WORKDIR  "D:/exampleproject"

# Install required rubygems
# If these files changes it should automatically rebuild the rest
RUN ridk exec sed -i 's/Required DatabaseOptional/Never/' /etc/pacman.conf
RUN ridk exec pacman -S msys2-keyring --noconfirm
#RUN ridk exec pacman -Syuu --noconfirm
#RUN ridk exec sed -i 's/Never/Required DatabaseOptional/' /etc/pacman.conf
ADD tasks/first_setup.bat  C:/drived/exampleproject/tasks/first_setup.bat
RUN C:/drived/exampleproject/tasks/first_setup.bat
ADD Gemfile C:/drived/exampleproject/Gemfile
ADD Gemfile.lock C:/drived/exampleproject/Gemfile.lock
ADD vendor C:/drived/exampleproject/vendor
ADD tasks/install_gems.bat  C:/drived/exampleproject/tasks/install_gems.bat
ADD .dockerbundleconfig C:/drived/exampleproject/.bundle/config
RUN C:/drived/exampleproject/tasks/install_gems.bat

CMD "cmd.exe"

FROM exampleproject_base AS exampleproject_compile

