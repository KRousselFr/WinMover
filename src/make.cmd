REM GCC (dans TDM-GCC, MinGW, etc.) doit ˆtre dans le PATH

gcc -std=c11 -c WinMover.c -o WinMover.o -Wall -Wextra -Wconversion -pedantic -O
gcc WinMover.o -o WinMover.exe -Wall -Wextra -O
strip WinMover.exe

