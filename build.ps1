<#
#>

$PrefixDir=[System.IO.Path]::GetDirectoryName($MyInvocation.MyCommand.Definition)
$SourcesDir="$PrefixDir/src"
$ObjectDir="$PrefixDir/obj"

<#
# Compile-Domake Function
#>
Function Start-CompileDomake{
    param(
        [Parameter(Position=0,Mandatory=$True,HelpMessage="Enter Sources Directory")]
        [ValidateNotNullorEmpty()]
        [String]$SrcDir,
        [String]$Include
    )
    Push-Location $PWD
    if(!(Test-Path "$PrefixDir/obj")){
        mkdir -Force "$PrefixDir/obj"
    }
    Set-Location "$PrefixDir/obj"
    $filelist=Get-ChildItem "$SrcDir"  -Recurse *.cc | Foreach {$_.FullName}
    foreach($file in $filelist){
        #Build File
        &cl -c $file -O2 -TP  -W4 -EHsc -Zc:forScope -Zc:wchar_t -MT -I$SrcDir
    }
    Pop-Location
}

Function Start-LinkDomake{
    param(
        [Parameter(Position=0,Mandatory=$True,HelpMessage="Enter Objects Directory")]
        [ValidateNotNullorEmpty()]
        [String]$ObjectDir
    )
    Push-Location $PWD
    &link "$ObjectDir\*.obj" KERNEL32.lib   ADVAPI32.lib Shell32.lib USER32.lib GDI32.lib comctl32.lib Shlwapi.lib Secur32.lib
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
    #
}

param(
    [ValidateSet("build","rebuild","clear")]
    [String]$action="build"
)

Restore-Environment
Start-CompileDomake -SrcDir $SourcesDir -Include "$PrefixDir/include"
Start-LinkDomake -ObjectDir $ObjectDir
