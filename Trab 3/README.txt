comando de compilação normal:

gcc main.c -pthread -lbsd -o socket

modo verboso:

gcc main.c -DVERBOSE -pthread -lbsd -o socket
