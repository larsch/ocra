@ECHO ON
for %%I in ("%~dp0.") do for %%J in ("%%~dpI.") do set ParentFolderName=%%~dpnxJ
echo %ParentFolderName%
cd /D %ParentFolderName%
call bundle install
