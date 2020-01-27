@echo off
git branch

set /P branchName=Entre le nom de la branche: 
git checkout master
git branch -D %branchName%
git push origin --delete %branchName%
pause