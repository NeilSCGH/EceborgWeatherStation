@echo off
git pull

set /P branchName=Entre le nom de la branche: 
git branch %branchName%
git checkout %branchName%

git push --set-upstream origin %branchName%

git add .
git commit -m "new branch %branchName%"
git push
pause