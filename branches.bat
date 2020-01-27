@echo off
git pull
git branch -a
echo.
git log --oneline --decorate --graph --all
pause