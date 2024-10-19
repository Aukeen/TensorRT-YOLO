@echo off
call "C:\ProgramData\miniconda3\Scripts\activate.bat" pytorch212

pip install . -i https://pypi.tuna.tsinghua.edu.cn/simple
pause
