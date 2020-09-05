CALL :import bin.zip "https://www.dropbox.com/sh/olgddijjxcdoabc/AACkBaPfRkmUXkXRjapfQ1-ya?dl=1" bin
CALL :import lib.zip "https://www.dropbox.com/sh/1p5yfvm1ipowdy4/AAAG7fXpmhTStNWb1yjT_q5Ta?dl=1" lib
CALL :import include.zip "https://www.dropbox.com/sh/8wcwnom54d3iub1/AADdoO609RsbOt1JrdCAp6tIa?dl=1" include
CALL :import assets.zip "https://www.dropbox.com/sh/ky2lb4fefc00xa6/AADN9dDNXM4EMZU-uPoFd49ra?dl=1" assets
CALL :import extras.zip "https://www.dropbox.com/sh/ivstaw0kblgimwe/AADib2fNuH0XbmpP-jshBWoTa?dl=1" extras
EXIT /B 0

:import
mkdir %~3 2> NUL
powershell.exe -nologo -noprofile -command "(New-Object Net.WebClient).DownloadFile('%~2', '%~1')"
powershell.exe -nologo -noprofile -command "& { $shell = New-Object -COM Shell.Application; $dst = Resolve-Path %~3; $src = Resolve-Path '%~1'; $target = $shell.NameSpace($dst.Path); $zip = $shell.NameSpace($src.Path); $target.CopyHere($zip.Items(), 16); }"
del %~1
powershell.exe -nologo -noprofile -command "foreach($file in Get-ChildItem -Recurse %~3) {$(Get-Item $file.Fullname).lastwritetime=$(Get-Date)}"
EXIT /B 0
