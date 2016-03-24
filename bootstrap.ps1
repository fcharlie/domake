<#
#>
param(
    [ValidateSet("build","rebuild","clear")]
    [String]$Action="build",
    [ValidateSet("Release","Debug")]
    [String]$Flavor="Release"
)
# powershell ./build.ps1 -Flavor Debug
# $args.Counts foreach ($a in $args)
if($PSVersionTable.PSVersion.Major -lt 3)
{
    $PSVersionString=$PSVersionTable.PSVersion.Major
    Write-Error "Build.ps1 must run under PowerShell 3.0 or later host environment !"
    Write-Error "Your PowerShell Version:$PSVersionString"
    if($Host.Name -eq "ConsoleHost"){
        [System.Console]::ReadKey()
    }
    Exit
}

#$PrefixDir=[System.IO.Path]::GetDirectoryName($MyInvocation.MyCommand.Definition)
$PrefixDir=$PSScriptRoot
$SourcesDir="$PSScriptRoot\src"
$ObjectDir="$PSScriptRoot\obj"

<#
# Compile-Domake Function
#>
Function Start-CompileDomake{
    param(
        [Parameter(Position=0,Mandatory=$True,HelpMessage="Enter Sources Directory")]
        [ValidateNotNullorEmpty()]
        [String]$SrcDir,
        [String]$Include,
        [ValidateSet("Release","Debug")]
        [String]$Flavor="Release"
    )
    Push-Location $PWD
    if(!(Test-Path "$PrefixDir\obj")){
        mkdir -Force "$PrefixDir\obj"
    }
    Set-Location "$PrefixDir\obj"
    Write-Output "Compiler sources, Flavor:$Flavor `nSources Dir: $SrcDir `nInclude Dir: $Include"
    $filelist=Get-ChildItem "$SrcDir"  -Recurse *.cc | Foreach-Object {$_.FullName}
    foreach($file in $filelist){
        #Build File
        if($Flavor -eq "Debug"){
                &cl -nologo -I"$PrefixDir\include"  -c  $file -O2 -TP  -DDEBUG -W4 -EHsc -Zc:forScope -Zc:wchar_t -MTd -I$SrcDir
        }else{
                &cl -nologo -I"$PrefixDir\include" -c  $file -O2 -TP -DNODEBUG -W4 -EHsc -Zc:forScope -Zc:wchar_t -MT -I$SrcDir
        }
    }
    &rc -nologo  -fo "$PrefixDir\obj\domake.res" "$SrcDir\domake.rc"
    Pop-Location
}

Function Start-LinkDomake{
    param(
        [Parameter(Position=0,Mandatory=$True,HelpMessage="Enter Objects Directory")]
        [ValidateNotNullorEmpty()]
        [String]$ObjectDir,
        [Parameter(Position=1,Mandatory=$True,HelpMessage="Enter Target Name")]
        [ValidateNotNullorEmpty()]
        [String]$Target,
        [ValidateSet("Release","Debug")]
        [String]$Flavor="Release"
    )
    Push-Location $PWD
    Write-Output "Link domake.....`nFlavor: $Flavor"
    Set-Location "$ObjectDir"
    if($Flavor -eq "Debug"){
        &link -nologo "*.obj" domake.res KERNEL32.lib  ADVAPI32.lib Shell32.lib WinHTTP.lib USER32.lib GDI32.lib comctl32.lib Shlwapi.lib Secur32.lib -out:$Target
    }else{
        &link -nologo "*.obj" domake.res KERNEL32.lib  ADVAPI32.lib Shell32.lib WinHTTP.lib USER32.lib GDI32.lib comctl32.lib Shlwapi.lib Secur32.lib -out:$Target
    }
    if(Test-Path $Target){
        Write-Output "Target: $Target`nBuild domake success !"
    }else{
        Write-Error "Build domake failed !"
    }
    Pop-Location
}

Function Restore-Environment{
    param(
        [Parameter(Position=0,Mandatory=$True,HelpMessage="Enter Root Folder")]
        [String]$Root=$PrefixDir
    )
    #Check Root
}

Function Clear-Domake{
    param(
        [Parameter(Position=0,Mandatory=$True,HelpMessage="Enter Root Folder")]
        [String]$Root=$PrefixDir
    )   
}

if($Action -eq "build"){
    Restore-Environment -Root $PrefixDir
    Start-CompileDomake -SrcDir $SourcesDir -Include "$PrefixDir\include" -Flavor $Flavor
    Start-LinkDomake -ObjectDir $ObjectDir -Target "$PrefixDir\domake.exe" -Flavor $Flavor
}elseif($Action -eq "rebuild"){
    Clear-Domake -Root $PrefixDir
    Restore-Environment -Root $PrefixDir
    Start-CompileDomake -SrcDir $SourcesDir -Include "$PrefixDir\include" -Flavor $Flavor
    Start-LinkDomake -ObjectDir $ObjectDir -Target "$PrefixDir\domake.exe" -Flavor $Flavor
}elseif($Action -eq "clear"){
    Clear-Domake -Root $PrefixDir
    exit 0
}

