<#
#>

$PrefixDir=[System.IO.Path]::GetDirectoryName($MyInvocation.MyCommand.Definition)
$SourcesDir="$PrefixDir/src"

Function Build-Sources{
    param(
        [String]$SrcDir,
        [String]$Include
    )
    Push-Location $PWD
    if((Test-Path "$PrefixDir/obj")){
        mkdir "$PrefixDir/obj"
    }
    Set-Location "$PrefixDir/obj"
    $filelist=Get-ChildItem "$SrcDir"  -recurse *.cc | %{$_.FullName}
    foreach($file in filelist){
        #Build File
        IEX "cl -c $file -O2 -I$SrcDir"
    }
    Pop-Location
}

Function Link-Domake{
    param(
        [String]$ObjectDir
    )
    Push-Location $PWD
    Pop-Location
}
