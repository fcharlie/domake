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
        &cl -c $file -O2 -I$SrcDir
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
    &link 
    Pop-Location
}

Start-CompileDomake -SrcDir $SourcesDir -Include "$PrefixDir/include"
Start-LinkDomake -ObjectDir $ObjectDir
