"Requirements.txt" is an adaptation of the assignment file on Canvas for better readability. . .

"The client will also create a socket in the Internet domain, send requests to the 
SERVER_PORT of a computer specified on the command-line, and receive responses 
through this socket from a server."

"The server creates a socket in the Internet domain bound to port SERVER_PORT "61410" (a constant you should define in both 
programs, you may use last 4 digits of your UM-ID to get a unique port number). The server receives requests through this socket, 
acts on those requests, and returns the results to the requester."

----------------------------------------------------------------------------------------------------------------------------------

Your client and server should operate as follows. Your server begins execution by opening a 
text file or a SqLite database file and that contains two tables(initially empty):

One table that holds stock records and the other table holds user records including the user’s balance in USD. 

create table if not exists Users 
           ( 
            ID int NOT NULL AUTO_INCREMENT,  
            first_name varchar(255), 
            last_name varchar(255), 
            user_name varchar(255) NOT NULL, 
            password varchar(255),          
            usd_balance DOUBLE NOT NULL, 
            PRIMARY KEY (ID)             
            );

create table if not exists Stocks  
           ( 
            ID int NOT NULL AUTO_INCREMENT, 
            stock_symbol varchar(4) NOT NULL, 
            stock_name varchar(20) NOT NULL, 
            stock_balance DOUBLE, 
            user_id int,          
            PRIMARY KEY (ID), 
            FOREIGN KEY (user_id) REFERENCES Users ID)             
            ); 
            
----------------------------------------------------------------------------------------------------------------------------------

Only your server should create, read, write, update, or delete records into/from the text file 
or SQLite database. Once the server has started, it should check if there is at least one user, 
if there are no users the server should create one user manually or using code with, say, 
$100 balance. Then the server should wait for the connection requests from a client.  

----------------------------------------------------------------------------------------------------------------------------------

"BUY"

Buy an amount of stocks. A client sends the ASCII string “BUY” followed by a space, 
followed by a stock_symbol, followed by a space, followed by a stock_amount, followed by 
a space,  followed by the price per stock, followed by a User_ID, and followed by the 
newline character (i.e., '\n').    
When the server receives a BUY command from a client, the server should handle the 
operation by calculating the stock price and deducting the stock price from the user’s 
balance, a new record in the Stocks table is created or updated if one does exist. If the 
operation is successful return a message to the client with “200 OK”, the new usd_balance 
and new stock_balance; otherwise, an appropriate message should be displayed, e.g.: Not 
enough balance, or user1 doesn’t exist, etc. 

A client-server interaction with the BUY command thus looks like:  
 
c: BUY MSFT 3.4 1.35 1  
s: Received: BUY MSFT 3.4 1.35 1 
c: 200 OK  
BOUGHT: New balance: 3.4 MSFT.  USD balance $95.41 
 
Where 3.4 is the amount of stocks to buy, $1.35 price per stock, 1 is the user id. 

----------------------------------------------------------------------------------------------------------------------------------

"SELL"

SELL an amount of stock. A client sends the ASCII string “BUY” followed by a space, 
followed by a stock_symbol, followed by a space, followed by stock price, followed by a 
space, followed by a stock_amount, followed by a space, followed by a User_ID, and 
followed by the newline character (i.e., '\n').    

When the server receives a SELL command from a client, the server should handle the 
operation by calculating the stock price and deposit the stock price to the user balance. If the 
operation is successful return a message to the client with “200 OK”, the new usd_balance 
and new stock_balance.  

A client-server interaction with the SELL command thus looks like:  
 
c: SELL APPL 2 1.45 1  
s: Received: SELL APPL 2 1.45 1 
c: 200 OK  
SOLD: New balance: 1.4 APPL. USD $98.31 
 
Where stock symbol is APPL, amount to be sold 2, price per stock $1.45, and 1 is the user id.

----------------------------------------------------------------------------------------------------------------------------------

"LIST"

List all records in the Stocks table/file. 
A client-server interaction with the LIST command looks like:  
 
c: LIST 
s: Received: LIST  
c: 200 OK  
The list of records in the Stocks database for user 1: 
1 MSFT 3.4 1  
2 APPL 5 1  
3 GME 200 1 

----------------------------------------------------------------------------------------------------------------------------------

"BALANCE" 

Display the USD balance for user 1 
A client-server interaction with the BALANCE command looks like:  
 
c: BALANCE 
s: Received: BALANCE  
c: 200 OK  
Balance for user John Doe: $98.31 

----------------------------------------------------------------------------------------------------------------------------------

"SHUTDOWN"

The SHUTDOWN command, which is sent from the client to the server, is a single line 
message that allows a user to shutdown the server.  A user that wants to shutdown the 
server should send the ASCII string "SHUTDOWN" followed by the newline character 
(i.e., '\n').  
Upon receiving the SHUTDOWN command, the server should return the string "200 OK" 
(terminated with a newline), close all open sockets and database connection, and then 
terminate.  
A client-server interaction with the SHUTDOWN command looks like:  
c: SHUTDOWN 
s: Received: SHUTDOWN 
c: 200 OK   

----------------------------------------------------------------------------------------------------------------------------------

"QUIT"

Only terminate the client.   The client exits when it receives the confirmation message from 
the server. 
 
A client-server interaction with the QUIT command looks like:  
 
c: QUIT 
c: 200 OK  
 
Note, “400 invalid command” or “403 message format error” should be returned to the 
client if a server receives an invalid command or the command in the wrong format along 
with an appropriate message, e.g.: Not enough MSFT stock balance, not enough USD or 
user 3 doesn’t exist,  etc. 

----------------------------------------------------------------------------------------------------------------------------------

The following items are required for full credit (100%):

• Implement all 6 commands: BUY, SELL, LIST, BALANCE, QUIT, SHUTDOWN 

• After BUY and SELL the Stocks table/file and Users table/file should be updated 

• Both the server and client should be able to run on any UMD login servers.  

• Make sure that you do sufficient error handling such that a user can't crash your 
server. For instance, what will you do if a user provides invalid input?  

• The server IP address should be a command line parameter for the client program. 

• The server should print out all messages received from clients on the screen.

• When the previous client exits, the server should allow the next client to connect. 

• Your source codes must be well commented 

• Submission should not include any object files, compiled files, or project files.  

• Include a README.docx file in your submission with all compile and run 
instructions in details 

• Include a Makefile in your submission. 

• Submission size must be less than 2 MB, otherwise, 10 points will be deducted from 
the project grade.  

• Submit a clear video showing that your project is running as expected with few 
example transactions and error handling cases. 

In your README file, the following information should be included: the commands that 
you have implemented, the instructions about how to compile and run your program, any 
known problems or bugs, and the output at the client side of a sample run of all 
commands you implemented, this can be replaced with a recorded video of running the 
server and client in all cases.   
