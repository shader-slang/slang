# If stdlib-reference folder does not exist, clone from github repo
if (-not (Test-Path ".\stdlib-reference")) {
    git clone https://github.com/shader-slang/stdlib-reference/
}
else {
    cd stdlib-reference
    git pull
    cd ../
}
Remove-Item -Path ".\stdlib-reference\global-decls" -Recurse -Force
Remove-Item -Path ".\stdlib-reference\interfaces" -Recurse -Force
Remove-Item -Path ".\stdlib-reference\types" -Recurse -Force

cd stdlib-reference
& ../../build/Release/bin/slangc -compile-stdlib -doc
Move-Item -Path ".\toc.html" -Destination ".\_includes\stdlib-reference-toc.html" -Force
git config user.email "bot@shader-slang.com"
git config user.name "Stdlib Reference Bot"
git add .
git commit -m "Update stdlib reference"
git push
cd ../

