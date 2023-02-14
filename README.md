# CIS-427-Program1
"Online Stock Trading" 
## Team Members
Fadi Baza, Connor Fox, and Austin Hall
## Required Software
If not on Mac OS, a SSH client will be required, we reccomend WinSCP.

Additionally, a SFTP client will be required, we reccomend FileZilla. 

All required software is free-to-use, and does not require a subscription.


## How to compile/run program(s)
To start you will have to first download these files:
```
server.cpp
client.cpp
sqlite3.o
sqlite3.h
```

Once downloaded, Use WinSCP or something similar (SSH Client) to transfer the files to your UMICH server. (Make sure to transfer all four files)

Once transfered, your going to need to compiple the .cpp files. To do so its very simple

client.cpp
> g++ client.cpp -o client

server.cpp
> g++ server.cpp -std=c++11 -ldl -pthread sqlite3.o -o server

After compiling both of the files you can now run them

```
./server
./client
```
