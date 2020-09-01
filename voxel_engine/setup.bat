CALL :import bin.zip "https://www.dropbox.com/s/ob70jwjonhm9rek/bin.zip?dl=1" .
CALL :import lib.zip "https://www.dropbox.com/s/uehggk5i7yymmg9/lib.zip?dl=1" .
CALL :import include.zip "https://www.dropbox.com/s/tcpgqpx9taj1d31/include.zip?dl=1" .
CALL :import assets.zip "https://www.dropbox.com/sh/ky2lb4fefc00xa6/AADN9dDNXM4EMZU-uPoFd49ra?dl=1" ./assets
cnmake
nmake debug
EXIT /B 0

:import
mkdir %~3 2> NUL
powershell.exe -nologo -noprofile -command "(New-Object Net.WebClient).DownloadFile('%~2', '%~1')"
powershell.exe -nologo -noprofile -command "& { $shell = New-Object -COM Shell.Application; $dst = Resolve-Path %~3; $src = Resolve-Path '%~1'; $target = $shell.NameSpace($dst.Path); $zip = $shell.NameSpace($src.Path); $target.CopyHere($zip.Items(), 16); }"
del %~1
EXIT /B 0
