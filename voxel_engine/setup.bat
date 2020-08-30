Rem powershell.exe -nologo -noprofile -command "& { $shell = New-Object -COM Shell.Application; $dst = Resolve-Path .; $src = Resolve-Path 'lib.zip'; $target = $shell.NameSpace($dst.Path); $zip = $shell.NameSpace($src.Path); $target.CopyHere($zip.Items(), 16); }"
Rem powershell.exe -nologo -noprofile -command "& { $shell = New-Object -COM Shell.Application; $dst = Resolve-Path .; $src = Resolve-Path 'include.zip'; $target = $shell.NameSpace($dst.Path); $zip = $shell.NameSpace($src.Path); $target.CopyHere($zip.Items(), 16); }"

CALL :import bin.zip "https://www.dropbox.com/s/ob70jwjonhm9rek/bin.zip?dl=1"
CALL :import lib.zip "https://www.dropbox.com/s/uehggk5i7yymmg9/lib.zip?dl=1"
CALL :import include.zip "https://www.dropbox.com/s/tcpgqpx9taj1d31/include.zip?dl=1"
cnmake
nmake debug
EXIT /B 0

:import
powershell.exe -nologo -noprofile -command "(New-Object Net.WebClient).DownloadFile('%~2', '%~1')"
powershell.exe -nologo -noprofile -command "& { $shell = New-Object -COM Shell.Application; $dst = Resolve-Path .; $src = Resolve-Path '%~1'; $target = $shell.NameSpace($dst.Path); $zip = $shell.NameSpace($src.Path); $target.CopyHere($zip.Items(), 16); }"
del %~1
EXIT /B 0
