@echo off
git branch -a

set /P branchName=Entre le nom de la branche: 
git checkout %branchName%
pause