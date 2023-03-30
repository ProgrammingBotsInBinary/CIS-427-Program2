# CIS-427-Program2
"Online Stock Trading" 
## Team Members
Fadi Baza, Connor Fox, and Austin Hall
## Responsibilities
Fadi: Debugging, testing logistics, client modification, and documentation. 

Connor & Austin: General updating program functionnality to support Assignment 2 requirements such as LOOKUP, WHO, etc. 
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

Once transfered, your going to need to compile the .cpp files. To do so its very simple

client.cpp
> g++ client.cpp -o client

server.cpp
> g++ server.cpp -std=c++11 -ldl -pthread sqlite3.o -o server

After compiling both of the files you can now run them

```
./server
./client localhost
```

| Command | Description |
| --- | --- |
| LOGIN root root01 | login as root |
| BUY . . .  | buy something as root |
| SELL . . . | sell something as root |
| LIST | List all accounts |
| Login from a different terminal/ session |
| LOGIN Mary Mary01 | login as Mary |
| DEPOSIT 10 | deposit 10 as Mary |
| BUY . . . | buy something as Mary |
| WHO | who am I? As Mary |
| LOOKUP MSFT | lookup MSFT |
| LIST | LIST as root! |
| LOGOUT | LOGOUT as root! |
| BALANCE | BALANCE as Mary |
| QUIT | QUIT as Mary |
| LOGOUT | LOGOUT as Mary | 
| SHUTDOWN | SHUTDOWN as Root |
