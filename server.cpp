// Standard C++ headers
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// Server Port/Socket/Addr related headers
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>

// SQLite3 related headers/definitions
#include "sqlite3.h"

// Definitions
#define server_port 3976
#define max_wait 5
#define max_output 256

// Functions
std::string buildCommand(char*);
bool extractInfo(char*, std::string*, std::string);
static int callback(void*, int, char**, char**);

// Main Function
int main(int argc, char* argv[]) {
    
    // Database Variables
    sqlite3* db;
    char* zErrMsg = 0;
    int rc;
    const char* sql;
    
    std::string resultant;
    std::string* ptr = &resultant;
    
    // Open connection to the database
    rc = sqlite3_open("DB.sqlite", &db);
    
    // Check to see if the database was opened
    if (rc) {
        fprintf(stderr, "[DB] Can't open database: %s\n", sqlite3_errmsg(db));
        return(0);
    }
    else {
        fprintf(stderr, "[DB] Opened database successfully!\n");
    }
    
    // Create the SQL "users" table
    sql = "create table if not exists users\
    (\
        ID INTEGER PRIMARY KEY AUTOINCREMENT,\
        email varchar(255) NOT NULL,\
        first_name varchar(255),\
        last_name varchar(255),\
        user_name varchar(255) NOT NULL,\
        password varchar(255),\
        usd_balance DOUBLE NOT NULL\
    );";
    rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
    
    // stock_name varchar(20),\
    // Create SQL "stocks" table
    sql = "create table if not exists stocks (\
        ID INTEGER PRIMARY KEY AUTOINCREMENT,\
        stock_symbol varchar(4) NOT NULL,\
        stock_balance DOUBLE,\
        user_id varchar(255),\
        FOREIGN KEY(user_id) REFERENCES users(ID)\
    );";
    rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
    
    //Checking if user 1 exists in the table, if this user doesn't exist, create it
    sql = "SELECT IIF(EXISTS(SELECT 1 FROM users WHERE  users.ID=1), 'USER_PRESENT', 'USER_NOT_PRESENT') result;";
    rc = sqlite3_exec(db, sql, callback, ptr, &zErrMsg);
    
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    else if (resultant == "USER_NOT_PRESENT") {
        
        // Creating a user if one doesn't already exist
        fprintf(stdout, "User's table is empty! Creating a new user.\n");
        
        // User 1, Stock Trader, StockTrader, stocktrader@umich.edu, password, balance of $999
        sql = "INSERT INTO users VALUES (1, 'stocktrader@umich.edu', 'Stock', 'Trader', 'StockTrader', 'password', 999);";
        rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
        
        if (rc != SQLITE_OK) {
            fprintf(stderr, "SQL error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
        }
        else {
            fprintf(stdout, "\t[USER] Stock Trader, 'StockTrader' was added to the table!.\n");
        }
    }
    else if (resultant == "USER_PRESENT") {
        std::cout << "[USER] Users exist in the table, continuing.\n";
    }
    else {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        std::cout << "Error returned resultant: " << resultant << std::endl;
    }
    
    // Server Variables
    struct sockaddr_in srv;
    char buf[max_output];
    socklen_t buf_len, addr_len;
    int nRet;
    int nClient;
    int nSocket;
    
    std::string infoArr[4];
    std::string command = "";
    
    // Open the socket
    nSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (nSocket < 0) {
        std::cout << "[SOCKET] Socket not Opened!\n";
        sqlite3_close(db);
        std::cout << "[SOCKET] Closing database!" << std::endl;
        exit(EXIT_FAILURE);
    }
    else {
        std::cout << "[SOCKET] The socket has been opened: " << nSocket << std::endl;
    }
    
    // Building the internet address structure
    srv.sin_family = AF_INET;
    srv.sin_port = htons(server_port);
    srv.sin_addr.s_addr = INADDR_ANY;
    memset(&(srv.sin_zero), 0, 8);
    
    // Seting up the Socket Options
    int nOptVal = 0;
    int nOptLen = sizeof(nOptVal);
    nRet = setsockopt(nSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&nOptVal, nOptLen);
    if (!nRet) {
        std::cout << "[SOCKET] The setsockopt call was a success!\n";
    }
    else {
        std::cout << "[SOCKET] Failed setsockopt call!\n";
        sqlite3_close(db);
        std::cout << "[DB] Closing database!" << std::endl;
        close(nSocket);
        std::cout << "[SOCKET] Closed the socket: " << nSocket << std::endl;
        exit(EXIT_FAILURE);
    }
    
    //Bind socket to the local port
    nRet = (bind(nSocket, (struct sockaddr*)&srv, sizeof(srv)));
    if (nRet < 0) {
        std::cout << "[PORT] Failed to bind the socket to local port!\n";
        sqlite3_close(db);
        std::cout << "[DB] Closing database!" << std::endl;
        close(nSocket);
        std::cout << "[SOCKET] Closed the socket: " << nSocket << std::endl;
        exit(EXIT_FAILURE);
    }
    else {
        std::cout << "[SOCKET] Bound socket to the local port!\n";
    }
    
    //Listens to the command requests from the client
    nRet = listen(nSocket, max_wait);
    if (nRet < 0) {
        std::cout << "[PORT] Failure in listening to the local port!\n";
        sqlite3_close(db);
        std::cout << "[DB] Closing the database!" << std::endl;
        close(nSocket);
        std::cout << "[SOCKET] Closed the socket: " << nSocket << std::endl;
        exit(EXIT_FAILURE);
    }
    else {
        std::cout << "[PORT] Successfully listening to the local port!\n";
    }
    
    // Waits for connection, then receive and print texts
    while (1) {
        if ((nClient = accept(nSocket, (struct sockaddr*)&srv, &addr_len)) < 0) {
            perror("[SOCKET] Error encountered while accepting the connection!");
            sqlite3_close(db);
            std::cout << "[DB] Closing database!" << std::endl;
            close(nSocket);
            std::cout << "[SOCKET] Closed the socket: " << nSocket << std::endl;
            exit(EXIT_FAILURE);
        }
        else {
            std::cout << "[SOCKET] Client has successfully connected on the socket: " << nClient << std::endl << std::endl;
            send(nClient, "[SERVER] Hello! You have been connected to the server!", 47, 0);
        }
        
        while ((buf_len = (recv(nClient, buf, sizeof(buf), 0)))) {
            
            // Displays the message receieved:
            std::cout << "[SERVER] Recieved message: " << buf;
            
            // Begin parsing the command via the buffer
            command = buildCommand(buf);
            
            if (command == "BUY") {
                
                // Checks for proper command formatting
                if (!extractInfo(buf, infoArr, command)) {
                    send(nClient, "[BUY] 403 message format error: Missing information\n [BUY] EX. Command: BUY stock_sybmol stock_amount price user_ID", sizeof(buf), 0);
                }
                else {
                    // Check if the user exists within the user table
                    std::string selectedUsr = infoArr[3];
                    std::string sql = "SELECT IIF(EXISTS(SELECT 1 FROM users WHERE users.ID=" + selectedUsr + "), 'PRESENT', 'NOT_PRESENT') result;";
                    rc = sqlite3_exec(db, sql.c_str(), callback, ptr, &zErrMsg);
                    
                    //Checks if SQL executed correctly
                    if (rc != SQLITE_OK) {
                        fprintf(stderr, "[BUY] SQL error: %s\n", zErrMsg);
                        sqlite3_free(zErrMsg);
                        send(nClient, "[BUY] SQL error", 10, 0);
                    }
                    else if (resultant == "PRESENT") {
                        
                        // Confirm user exists
                        fprintf(stdout, "[BUY] User confirmed.\n");
                        
                        // Calculate the stock price
                        double stockPrice = std::stod(infoArr[1]) * std::stod(infoArr[2]);
                        std::cout << "[BUY] Stock Price: " << stockPrice << std::endl;
                        
                        // Gets the usd balance of the user
                        sql = "SELECT usd_balance FROM users WHERE users.ID=" + selectedUsr;
                        rc = sqlite3_exec(db, sql.c_str(), callback, ptr, &zErrMsg);
                        std::string usd_balance = resultant;
                        std::cout << "[BUY] User Balance: " << usd_balance << std::endl;
                        
                        // Check SQL status
                        if (rc != SQLITE_OK) {
                            fprintf(stderr, "[BUY] SQL error: %s\n", zErrMsg);
                            sqlite3_free(zErrMsg);
                            send(nClient, "[BUY] SQL error", 10, 0);
                        }
                        else if (stod(usd_balance) >= stockPrice) {
                            
                            // If the user had enough balance, update the new balance.
                            double difference = stod(usd_balance) - stockPrice;
                            std::string sql = "UPDATE users SET usd_balance=" + std::to_string(difference) + " WHERE ID =" + selectedUsr + ";";
                            rc = sqlite3_exec(db, sql.c_str(), callback, 0, &zErrMsg);
                            std::cout << "[BUY] New User Balance: " << difference << std::endl;
                            
                            // Check SQL status
                            if (rc != SQLITE_OK) {
                                fprintf(stderr, "[BUY] SQL error: %s\n", zErrMsg);
                                sqlite3_free(zErrMsg);
                                send(nClient, "[BUY] SQL error", 10, 0);
                            }
                            
                            // Check if record exists in table, then updates or creates a new one
                            sql = "SELECT IIF(EXISTS(SELECT 1 FROM stocks WHERE stocks.stock_symbol='" + infoArr[0] + "' AND stocks.user_id='" + selectedUsr + "'), 'RECORD_PRESENT', 'RECORD_NOT_PRESENT') result;";
                            rc = sqlite3_exec(db, sql.c_str(), callback, ptr, &zErrMsg);
                            
                            // Check SQL status
                            if (rc != SQLITE_OK) {
                                fprintf(stderr, "[BUY] SQL error: %s\n", zErrMsg);
                                sqlite3_free(zErrMsg);
                                send(nClient, "[BUY] SQL error", 10, 0);
                            }
                            
                            else if (resultant == "RECORD_PRESENT") {
                                // Record exists in the table - update it
                                sql = "UPDATE stocks SET stock_balance= stock_balance +" + infoArr[1] + " WHERE stocks.stock_symbol='" + infoArr[0] + "' AND stocks.user_id='" + selectedUsr + "';";
                                rc = sqlite3_exec(db, sql.c_str(), NULL, NULL, &zErrMsg);
                                std::cout << "[BUY] Added " << infoArr[1] << " stock to " << infoArr[0] << " for " << selectedUsr << std::endl;
                                
                                // SQL Status Check
                                if (rc != SQLITE_OK) {
                                    fprintf(stderr, "[BUY] SQL error: %s\n", zErrMsg);
                                    sqlite3_free(zErrMsg);
                                    send(nClient, "[BUY] SQL error", 10, 0);
                                }
                            }
                            else {
                                // A record does not exist, so add a record
                                sql = "INSERT INTO stocks(stock_symbol, stock_balance, user_id) VALUES ('" + infoArr[0] + "', '" + infoArr[1] + "', '" + selectedUsr + "');";
                                rc = sqlite3_exec(db, sql.c_str(), NULL, NULL, &zErrMsg);
                                std::cout << "[BUY] New record created:\n\tStock Symbol: " << infoArr[0] << "\n\tStock Balance: " << infoArr[1] << "\n\tUserID: " << selectedUsr << std::endl;
                                
                                // SQL status check
                                if (rc != SQLITE_OK) {
                                    fprintf(stderr, "[BUY] SQL error: %s\n", zErrMsg);
                                    sqlite3_free(zErrMsg);
                                    send(nClient, "[BUY] SQL error", 10, 0);
                                }
                            }
                            
                            // Balance
                            sql = "SELECT usd_balance FROM users WHERE users.ID=" + selectedUsr;
                            rc = sqlite3_exec(db, sql.c_str(), callback, ptr, &zErrMsg);
                            usd_balance = resultant;
                            
                            // SQL Status
                            if (rc != SQLITE_OK) {
                                fprintf(stderr, "[BUY] SQL error: %s\n", zErrMsg);
                                sqlite3_free(zErrMsg);
                                send(nClient, "[BUY] SQL error", 10, 0);
                            }
                            
                            // Gets new stock_balances
                            sql = "SELECT stock_balance FROM stocks WHERE stocks.stock_symbol='" + infoArr[0] + "' AND stocks.user_id='" + selectedUsr + "';";
                            rc = sqlite3_exec(db, sql.c_str(), callback, ptr, &zErrMsg);
                            
                            // SQL Status
                            if (rc != SQLITE_OK) {
                                fprintf(stderr, "[BUY] SQL error: %s\n", zErrMsg);
                                sqlite3_free(zErrMsg);
                                send(nClient, "[BUY] SQL error", 10, 0);
                            }
                            std::string stock_balance = resultant;
                            
                            // If reaching this point, the commands have completed successfully, return 200 to indicate success, and display new balance and stock balance
                            std::string tempStr = "[BUY] 200 OK\n[BOUGHT] New balance: " + stock_balance + " " + infoArr[0] + ". Balance $" + usd_balance;
                            send(nClient, tempStr.c_str(), sizeof(buf), 0);
                        }
                        else {
                            std::cout << "[SERVER] Not enough balance. Purchase Aborted." << std::endl;
                            send(nClient, "[BUY] 403 message format error: not enough balance!", sizeof(buf), 0);
                        }
                    }
                    else {
                        
                        // USER DOES NOT EXIST
                        fprintf(stdout, "[SERVER] User Does Not Exist in Users Table. Aborting Buy\n");
                        std::string tempStr = "[BUY] 403 message format error: user " + selectedUsr + " does not exist!";
                        send(nClient, tempStr.c_str(), sizeof(buf), 0);
                    }
                }
                std::cout << "[SERVER] Successfully executed BUY command\n\n";
            }
            
            else if (command == "SELL") {
                
                // Checks if the client used the command properly
                if (!extractInfo(buf, infoArr, command)) {
                    std::cout << "[SELL] Invalid command: Missing information" << std::endl;
                    send(nClient, "[SELL] 403 message format error: Missing information\n EX. Command: SELL stock_symbol stock_price amount userID", sizeof(buf), 0);
                }
                else {
                    std::string selectedUsr = infoArr[3];
                    
                    // Check if the user exists
                    std::string sql = "SELECT IIF(EXISTS(SELECT 1 FROM users WHERE users.ID=" + selectedUsr + "), 'PRESENT', 'NOT_PRESENT') result;";
                    rc = sqlite3_exec(db, sql.c_str(), callback, ptr, &zErrMsg);
                    
                    // SQL status check
                    if (rc != SQLITE_OK) {
                        fprintf(stderr, "[SELL] SQL error: %s\n", zErrMsg);
                        sqlite3_free(zErrMsg);
                        send(nClient, "[SELL] SQL error", 10, 0);
                    }
                    else if (resultant == "PRESENT") {
                        
                        // Checks if the user owns the selected stock
                        sql = "SELECT IIF(EXISTS(SELECT 1 FROM stocks WHERE stocks.stock_symbol='" + infoArr[0] + "' AND stocks.user_id='" + selectedUsr + "'), 'RECORD_PRESENT', 'RECORD_NOT_PRESENT') result;";
                        rc = sqlite3_exec(db, sql.c_str(), callback, ptr, &zErrMsg);
                        
                        // SQL check
                        if (rc != SQLITE_OK) {
                            fprintf(stderr, "[SELL] SQL error: %s\n", zErrMsg);
                            sqlite3_free(zErrMsg);
                            send(nClient, "[SELL] SQL error", 10, 0);
                        }
                        else if (resultant == "RECORD_NOT_PRESENT") {
                            std::cout << "[SERVER] User doesn't own the selected stock. Aborting Sell\n";
                            send(nClient, "[SELL] ERROR User does not own this stock.", sizeof(buf), 0);
                        }
                        else {
                            // Checks if the user has enough stock to sell
                            double stockToSell = std::stod(infoArr[1]);
                            
                            // Get the numbers of stock user has of a specific symbol
                            sql = "SELECT stock_balance FROM stocks WHERE stocks.stock_symbol='" + infoArr[0] + "' AND stocks.user_id='" + selectedUsr + "';";
                            rc = sqlite3_exec(db, sql.c_str(), callback, ptr, &zErrMsg);
                            
                            // SQL check
                            if (rc != SQLITE_OK) {
                                fprintf(stderr, "[SELL] SQL error: %s\n", zErrMsg);
                                sqlite3_free(zErrMsg);
                                send(nClient, "[SELL] SQL error", 10, 0);
                            }
                            
                            double stock_balance = std::stod(resultant);
                            // Not enough stock in balance to sell
                            if (stock_balance < stockToSell) {
                                std::cout << "[SERVER] Attempting to sell more stock than the user owns. Aborting sell.\n";
                                send(nClient, "[SELL] ERROR Attempting to sell more stock than the user owns.", sizeof(buf), 0);
                            }
                            else {
                                // Get dollars amount to sell
                                double stockPrice = std::stod(infoArr[1]) * std::stod(infoArr[2]);
                                
                                
                                // Update the user's balance
                                sql = "UPDATE users SET usd_balance= usd_balance +" + std::to_string(stockPrice) + " WHERE users.ID='" + selectedUsr + "';";
                                rc = sqlite3_exec(db, sql.c_str(), NULL, NULL, &zErrMsg);
                                
                                // SQL status check
                                if (rc != SQLITE_OK) {
                                    fprintf(stderr, "[SELL] SQL error: %s\n", zErrMsg);
                                    sqlite3_free(zErrMsg);
                                    send(nClient, "[SELL] SQL error", 10, 0);
                                }
                                
                                // Updates Stocks table and removes the sold stock from table
                                sql = "UPDATE stocks SET stock_balance= stock_balance -" + std::to_string(stockToSell) + " WHERE stocks.stock_symbol='" + infoArr[0] + "' AND stocks.user_id='" + selectedUsr + "';";
                                rc = sqlite3_exec(db, sql.c_str(), NULL, NULL, &zErrMsg);
                                
                                // SQL status check
                                if (rc != SQLITE_OK) {
                                    fprintf(stderr, "[SELL] SQL error: %s\n", zErrMsg);
                                    sqlite3_free(zErrMsg);
                                    send(nClient, "[SELL] SQL error", 10, 0);
                                }
                                
                                // Gets the new usd_balance
                                sql = "SELECT usd_balance FROM users WHERE users.ID=" + selectedUsr;
                                rc = sqlite3_exec(db, sql.c_str(), callback, ptr, &zErrMsg);
                                std::string usd_balance = resultant;
                                
                                // Gets the new stock_balance
                                sql = "SELECT stock_balance FROM stocks WHERE stocks.stock_symbol='" + infoArr[0] + "' AND stocks.user_id='" + selectedUsr + "';";
                                rc = sqlite3_exec(db, sql.c_str(), callback, ptr, &zErrMsg);
                                std::string stock_balance = resultant;
                                
                                // Sells the command completed successfully
                                std::string tempStr = "[SELL] 200 OK\n[SOLD] New balance: " + stock_balance + " " + infoArr[0] + ". USD $" + usd_balance;
                                send(nClient, tempStr.c_str(), sizeof(buf), 0);
                            }
                        }
                    }
                    else {
                        fprintf(stdout, "[SERVER] User Does Not Exist in Users Table. Aborting Sell.\n");
                        send(nClient, "[SELL] ERROR User does not exist.", sizeof(buf), 0);
                    }
                }
                std::cout << "[SERVER] Successfully executed SELL command\n\n";
            }
            else if (command == "LIST") {
                std::cout << "[LIST] List command." << std::endl;
                resultant = "";
                // Lists all the records in Stocks table for user_id = 1
                std::string sql = "SELECT * FROM stocks WHERE stocks.user_id='1'";
                
                // Executes SQL statement
                rc = sqlite3_exec(db, sql.c_str(), callback, ptr, &zErrMsg);
                
                // SQL status check
                if (rc != SQLITE_OK) {
                    fprintf(stderr, "[LIST] SQL error: %s\n", zErrMsg);
                    sqlite3_free(zErrMsg);
                    send(nClient, "[LIST] SQL error", 10, 0);
                }
                
                std::string sendStr;
                
                if (resultant == "") {
                    sendStr = "[LIST] 200 OK\n[LIST] No records in the Stock Database.";
                }
                else {
                    sendStr = "[LIST] 200 OK\n[LIST] The list of records in the Stock database for [User 1]:\n" + resultant;
                }
                send(nClient, sendStr.c_str(), sizeof(buf), 0);
            }
            else if (command == "BALANCE") {
                std::cout << "[BALANCE] Balance command." << std::endl;
                
                // Check for user
                std::string sql = "SELECT IIF(EXISTS(SELECT 1 FROM users WHERE users.ID=1), 'PRESENT', 'NOT_PRESENT') result;";
                
                // Executes SQL
                rc = sqlite3_exec(db, sql.c_str(), callback, ptr, &zErrMsg);
                
                // SQL check
                if (rc != SQLITE_OK) {
                    fprintf(stderr, "[BALANCE] SQL error: %s\n", zErrMsg);
                    sqlite3_free(zErrMsg);
                    send(nClient, "[BALANCE] SQL error", 10, 0);
                }
                else if (resultant == "PRESENT") {
                    
                    // Outputs balance
                    sql = "SELECT usd_balance FROM users WHERE users.ID=1";
                    rc = sqlite3_exec(db, sql.c_str(), callback, ptr, &zErrMsg);
                    std::string usd_balance = resultant;
                    
                    // Outputs full name
                    sql = "SELECT first_name FROM users WHERE users.ID=1";
                    rc = sqlite3_exec(db, sql.c_str(), callback, ptr, &zErrMsg);
                    std::string user_name = resultant;
                    
                    sql = "SELECT last_name FROM users WHERE users.ID=1";
                    rc = sqlite3_exec(db, sql.c_str(), callback, ptr, &zErrMsg);
                    user_name += " " + resultant;
                    
                    std::string tempStr = "[BALANCE] 200 OK\n[BALANCE] Balance for user " + user_name + ": $" + usd_balance;
                    send(nClient, tempStr.c_str(), sizeof(buf), 0);
                }
                else {
                    std::cout << "[SERVER] User does not exist. Aborting Balance.\n";
                    send(nClient, "[BALANCE] ERROR User does not exist.", sizeof(buf), 0);
                }
            }
            
            // Process to turn off the server and terminate all connections:
            else if (command == "SHUTDOWN") {
                // Send command confirmation
                send(nClient, "200 OK", 7, 0);
                std::cout << "[CLIENT] Shutdown command." << std::endl;
                
                // Close the database
                sqlite3_close(db);
                std::cout << "[DB] Closed DB" << std::endl;
                
                // Terminate the client connection
                close(nClient);
                std::cout << "[CLIENT] Closed Client Connection: " << nClient << std::endl;
                
                // Close the socket
                close(nSocket);
                std::cout << "[SERVER] Closed Server socket: " << nSocket << std::endl;
                
                // Exit
                exit(EXIT_SUCCESS);
            }
            
            // Close client, when the client quits
            else if (command == "QUIT") {
                std::cout << "[QUIT] Quit command." << std::endl;
                send(nClient, "[QUIT] 200 OK", 7, 0);
                close(nClient);
                break;
            }
            
            // Default response to invalid command
            else {
                std::cout << "[SERVER] Command not recognized?" << std::endl;
                send(nClient, "[SERVER] 400 invalid command", 20, 0);
            }
        }
    }
    // Close client
    close(nClient);
    
    // Close database
    sqlite3_close(db);
    std::cout << "[DB] Closed DB" << std::endl;
    
    // Close the socket
    close(nSocket);
    std::cout << "[SOCKET] Closed socket: " << nSocket << std::endl;
    
    // Exit
    exit(EXIT_SUCCESS);
}

// Function to build a command from the buffer CRITICAL
std::string buildCommand(char line[]) {
    std::string command = "";
    size_t len = strlen(line);
    for (size_t i = 0; i < len; i++) {
        if (line[i] == '\n')
            continue;
        if (line[i] == ' ')
            break;
        command += line[i];
    }
    return command;
}

// Enters the command info into an array such as [stock, amount, price, userID]
// Returns true if the parse is successful, if not, returns false (obviously)
bool extractInfo(char line[], std::string info[], std::string command) {
    int l = command.length();
    
    // Variable to hold the location of a space " "
    int spaceLocation = l + 1;
    
    for (int i = 0; i < 4; i++) {
        info[i] = "";
        
        // Parses the information
        for (int j = spaceLocation; j < strlen(line); j++) {
            
            if (line[j] == ' ')
                break;
            if (line[j] == '\n')
                break;
            info[i] += line[j];
            
            // Makes sure that only numbers are entered into the array for index 1, 2 and 3
            if (i > 0) {
                if (((int)line[j] > 57 || (int)line[j] < 46) && (int)line[j] != 47)
                    return false;
            }
        }
        
        if (info[i] == "") {
            std::fill_n(info, 4, 0);
            return false;
        }
        
        spaceLocation += info[i].length() + 1;
        
    }
    return true;
}

// Callback function
static int callback(void* ptr, int count, char** data, char** azColName) {
    
    std::string* resultant = (std::string*)ptr;
    
    if (count == 1) {
        *resultant = data[0];
    }
    else if (count > 1) {
        for (int i = 0; i < count; i++) {
            
            if (*resultant == "") {
                *resultant = data[i];
            }
            else {
                *resultant = *resultant + " " + data[i];
            }
            // Create a new line between every record
            if (i == 3)
            {
                *resultant += "\n  ";
            }
        }
    }
    return 0;
}
