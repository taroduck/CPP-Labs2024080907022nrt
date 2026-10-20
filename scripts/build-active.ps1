param(
    [Parameter(Mandatory = $true)]
    [string]$SourceFile,
    [string]$Compiler = 'C:/mingw64/bin/g++.exe'
)

$ErrorActionPreference = 'Stop'
$source = Get-Item -LiteralPath $SourceFile
if ($source.Extension -ne '.cpp') {
    throw 'Select a C++ source file before building.'
}

$sourceDirectory = $source.Directory.FullName
$labDirectory = $sourceDirectory
if ($source.Directory.Name -eq 'tests') {
    $labDirectory = $source.Directory.Parent.FullName
}

$arguments = @(
    '-std=c++17', '-Wall', '-Wextra', '-Wpedantic',
    '-Wshadow', '-Wconversion', '-Wsign-conversion', '-g3', '-O0'
)
$executor = Join-Path $labDirectory 'Executor.cpp'
if (Test-Path -LiteralPath $executor -PathType Leaf) {
    $arguments += @('-I', $labDirectory)
    if ($source.FullName -eq $executor) {
        # Building the component itself uses the demo as the entry point.
        $arguments += @(Join-Path $labDirectory 'main.cpp')
    } else {
        $arguments += @($source.FullName)
    }
    $arguments += @($executor)
} else {
    $arguments += @($source.FullName)
}

$outputFile = Join-Path $sourceDirectory ($source.BaseName + '.exe')
$arguments += @('-o', $outputFile)
& $Compiler @arguments
exit $LASTEXITCODE
