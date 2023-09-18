REM GCC (dans TDM-GCC, MinGW, etc.) doit ˆtre dans la PATH

gcc -std=c11 -c WinMover.c -o WinMover.o -Wall -Wextra -Wconversion -pedantic
gcc WinMover.o -o WinMover.exe -Wall -Wextra -O
strip WinMover.exe

