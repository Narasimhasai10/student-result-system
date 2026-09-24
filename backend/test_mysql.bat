@echo off
"C:\msys64\ucrt64\bin\g++.exe" -std=c++17 -I"C:\Program Files\MySQL\MySQL Server 8.0\include" test_mysql.cpp -L"C:\Program Files\MySQL\MySQL Server 8.0\lib" -lmysql -o test_mysql_64.exe 2>&1
echo Exit: %ERRORLEVEL%
pause
