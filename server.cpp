// Standard C++ headers
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

// Server Port, Socket, Addrress header files
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "sqlite3.h"

#define server_port  3976
#define max_wait  5
#define max_output  256

// Server Variables
struct sockaddr_in srv;
char buf[max_output];
socklen_t buf_len, addr_len;
int nRet;
int nClient[10] = { 0, };
int nSocket;
std::string infoArr[3];

sqlite3* db;
char* zErrMsg = 0;
const char* sql;
int rc;
std::string resultant;
std::string* ptr = &resultant;

typedef struct
{
    int socket;
    int id;
    std::string user;
    std::string password;
}userInfo;

typedef struct
{
    std::string ip;
    std::string user;
    int socket;
    pthread_t userThread;
}userConnect;

void* temp = malloc(sizeof(userInfo));
userInfo u;

std::vector<userConnect> list;

fd_set fr;
fd_set fw;
fd_set fe;
int nMaxFd;

pthread_t thread_handles;
long thread;

// Functions
std::string buildCommand(char*);
std::string extractInfo(char*, std::string);
bool extractInfo(char*, std::string*, std::string);
void* serverCommands(void*);
static int callback(void*, int, char**, char**);
std::string getPassword(char line[], int n);
void newConnection();
void clientData();

int main(int argc, char* argv[]) {

    // Open Database and Connect to Database
    rc = sqlite3_open("DB.sqlite", &db);

    // Check if Database was opened successfully
    if (rc) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return(0);
    }
    else {
        fprintf(stderr, "Opened database successfully\n");
    }

    // Create sql users table creation command
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

    // Execute users table creation
    rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);

    // Create sql stocks table creation command
    sql = "create table if not exists stocks (\
        ID INTEGER PRIMARY KEY AUTOINCREMENT,\
        stock_symbol varchar(4) NOT NULL,\
        stock_balance DOUBLE,\
        user_id varchar(255),\
        FOREIGN KEY(user_id) REFERENCES users(ID)\
    );";

    // Execute stocks table creation
    rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);

    // Checks if the root exists in the database. If no user is found, create it
    sql = "SELECT IIF(EXISTS(SELECT 1 FROM users WHERE  users.user_name='root'), 'USER_PRESENT', 'USER_NOT_PRESENT') result;";
    rc = sqlite3_exec(db, sql, callback, ptr, &zErrMsg);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }

    else if (resultant == "USER_NOT_PRESENT") {
        // Create the root user:
        fprintf(stdout, "[USER] Root user is not present. Attempting to add the user.\n");

        // Adds the root user
        sql = "INSERT INTO users VALUES (1, 'root123@gmail.com', 'Root', 'User', 'root', 'root01', 800);";
        rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);

        if (rc != SQLITE_OK) {
            fprintf(stderr, "SQL error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
        }
        else {
            fprintf(stdout, "\t[USER] Root User, 'root' was added to the table!.\n");
        }
    }
    else if (resultant == "USER_PRESENT") {
        std::cout << "\t[USER] Root exists in the table, continuing.\n";
    }
    else {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        std::cout << "[Error] Error returned Resultant = " << resultant << std::endl;
    }

    // Checks if Mary exists in the database. If no user is found, create it
    sql = "SELECT IIF(EXISTS(SELECT 1 FROM users WHERE  users.user_name='mary'), 'USER_PRESENT', 'USER_NOT_PRESENT') result;";
    rc = sqlite3_exec(db, sql, callback, ptr, &zErrMsg);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    else if (resultant == "USER_NOT_PRESENT") {
        fprintf(stdout, "[User] Creating a new user.\n");

        // Adds mary
        sql = "INSERT INTO users VALUES (2, 'marypoppins@gmail.com', 'Mary', 'Poppins', 'mary', 'mary01', 800);";
        rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);

        if (rc != SQLITE_OK) {
            fprintf(stderr, "SQL error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
        }
        else {
            fprintf(stdout, "\t[USER] Mary Poppins, 'mary' was added to the table!.\n");
        }
    }
    else if (resultant == "USER_PRESENT") {
        std::cout << "\t[User] Mary already exists in the users table, continuing.\n";
    }
    else {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        std::cout << "Error returned Resultant = " << resultant << std::endl;
    }

    // Check if John exists in the database. If no user is found, create it
    sql = "SELECT IIF(EXISTS(SELECT 1 FROM users WHERE  users.user_name='john'), 'USER_PRESENT', 'USER_NOT_PRESENT') result;";
    rc = sqlite3_exec(db, sql, callback, ptr, &zErrMsg);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    else if (resultant == "USER_NOT_PRESENT") {
        fprintf(stdout, "[User] Creating a new user. john.\n");

        // Adds john
        sql = "INSERT INTO users VALUES (3, 'johndoe@gmail.com', 'John', 'Doe', 'john', 'john01', 800);";
        rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);

        if (rc != SQLITE_OK) {
            fprintf(stderr, "SQL error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
        }
        else {
            fprintf(stdout, "\t[USER] John Doe, 'john' was added to the table!.\n");
        }
    }
    else if (resultant == "USER_PRESENT") {
        std::cout << "\t[User] John already exists in the users table, continuing.\n";
    }
    else {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        std::cout << "Error returned Resultant = " << resultant << std::endl;
    }

    // Check if Moe exists in the database. If no user is found, create it
    sql = "SELECT IIF(EXISTS(SELECT 1 FROM users WHERE  users.user_name='moe'), 'USER_PRESENT', 'USER_NOT_PRESENT') result;";
    rc = sqlite3_exec(db, sql, callback, ptr, &zErrMsg);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    else if (resultant == "USER_NOT_PRESENT") {
        fprintf(stdout, "[User] Creating a new user. 'moe'.\n");

        // Adds moe
        sql = "INSERT INTO users VALUES (4, 'moecurly@gmail.com', 'Moe', 'Curly', 'moe', 'moe01', 800);";
        rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);

        if (rc != SQLITE_OK) {
            fprintf(stderr, "SQL error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
        }
        else {
            fprintf(stdout, "\t[USER] Moe Curly, 'MoeCurly' was added to the table!.\n");
        }
    }
    else if (resultant == "USER_PRESENT") {
        std::cout << "\t[USER] User Moe exists in the table, continuing.\n";
    }
    else {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        std::cout << "Error returned Resultant = " << resultant << std::endl;
    }

    // Open the socket
    nSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (nSocket < 0) {
        std::cout << "[SOCKET] Socket not Opened\n";
        sqlite3_close(db);
        std::cout << "[SOCKET] Closing database!" << std::endl;
        exit(EXIT_FAILURE);
    }
    else {
        std::cout << "[SOCKET] The socket has been opened: " << nSocket << std::endl;
    }

    // Set Socket Options
    int nOptVal = 1;
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

    // Build the internet address data structure
    srv.sin_family = AF_INET;
    srv.sin_port = htons(server_port);
    srv.sin_addr.s_addr = INADDR_ANY;
    memset(&(srv.sin_zero), 0, 8);

    //Bind the socket to the local port
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

    //Listen to the command requests from the client
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

    struct timeval tv;
    std::cout << "\n[SOCKET] Waiting for connections ...\n";

    while (1)
    {
        tv.tv_sec = 1;
        tv.tv_usec = 0;

        // Set the FD_SET
        FD_ZERO(&fr);
        FD_SET(nSocket, &fr);
        nMaxFd = nSocket;

        for (int nIndex = 0; nIndex < 5; nIndex++)
        {
            if (nClient[nIndex] > 0)
            {
                FD_SET(nClient[nIndex], &fr);
            }
            if (nClient[nIndex] > nMaxFd)
                nMaxFd = nClient[nIndex];
        }

        nRet = select(nMaxFd + 1, &fr, NULL, NULL, &tv);

        //After the prior call, reset each bit
        if (nRet < 0)
        {
            std::cout << std::endl << "Select API call FAILED, exiting.";
            return (EXIT_FAILURE);
        }
        else
        {
            // Instance that a client is waiting to connect or data is incoming from an existing client
            if (FD_ISSET(nSocket, &fr))
            {
                newConnection();
            }
            else
            {
                clientData();
            }
        }
    }

    for (int l = 0; l < 10; l++) {
        close(nClient[l]);
    }

    //Close database
    sqlite3_close(db);
    std::cout << "[DB] Closed DB" << std::endl;

    //Close socket
    close(nSocket);
    std::cout << "[SOCKET] Closed socket: " << nSocket << std::endl;
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

// Enters the command information into an array 
std::string extractInfo(char line[], std::string command) {
    int l = command.length();
    int spaceLocation = l + 1;
    int i = spaceLocation;
    std::string info = "";

    while (line[i] != '\n') {
        if (line[i] == 0)
            return "";
        if (line[i] == ' ')
            return info;
        info += line[i];
        i++;
    }
    return info;
}

void* serverCommands(void* userData) {
    std::cout << "[COMMAND] Username: " << ((userInfo*)userData)->user << std::endl;;
    int clientIndex = ((userInfo*)userData)->socket;
    int clientID = nClient[((userInfo*)userData)->socket];

    nClient[clientIndex] = -1;
    std::cout << clientID << std::endl;
    int buf_len;
    std::string u = ((userInfo*)userData)->user;
    int idINT = ((userInfo*)userData)->id;
    std::string id = std::to_string(idINT);
    std::string command;
    bool rootUsr;

    if (idINT == 1) {
        rootUsr = true;
    }
    else {
        rootUsr = false;
    }

    while (1) 
    {
        char Buff[256] = { 0, };

        while ((buf_len = (recv(clientID, Buff, sizeof(Buff), 0)))) {

            // Displays the message received:
            std::cout << "[SERVER] Received message: " << Buff << std::endl;

            // Begin parsing the command via the buffer
            command = buildCommand(Buff);
            std::cout << command << std::endl;
            if (command == "LOGIN") {
                send(clientID, "You are already logged in", 27, 0);
            }
            else if (command == "BUY") {
                std::cout << "[BUY] Buy command!" << std::endl;

                // Checks if the client used the command properly
                if (!extractInfo(Buff, infoArr, command)) {
                    send(clientID, "[BUY] 403 message format error: Missing information\n [BUY] EX. Command: BUY stock_symbol amount price userID", sizeof(Buff), 0);
                    std::cout << "extraction Error" << std::endl;
                }
                else {
                    // Check if the user exists within the user table 
                    std::string sql = "SELECT IIF(EXISTS(SELECT 1 FROM users WHERE users.ID=" + (std::string)id + "), 'PRESENT', 'NOT_PRESENT') result;";
                    rc = sqlite3_exec(db, sql.c_str(), callback, ptr, &zErrMsg);

                    //  Check SQL status
                    if (rc != SQLITE_OK) {
                        fprintf(stderr, "[BUY] SQL error: %s\n", zErrMsg);
                        sqlite3_free(zErrMsg);
                    }
                    else if (resultant == "PRESENT") {
                        // Confirm user exists
                        fprintf(stdout, "[User] User Exists in Users Table.\n");

                        // Calculate stock price
                        double stockPrice = std::stod(infoArr[1]) * std::stod(infoArr[2]);
                        std::cout << "[BUY] Stock Price: " << stockPrice << std::endl;

                        // Get the usd balance of the user
                        sql = "SELECT usd_balance FROM users WHERE users.ID=" + (std::string)id;
                        rc = sqlite3_exec(db, sql.c_str(), callback, ptr, &zErrMsg);
                        std::string usd_balance = resultant;
                        std::cout << "[BUY] User Balance: " << usd_balance << std::endl;

                        //  Check SQL status
                        if (rc != SQLITE_OK) {
                            fprintf(stderr, "[BUY] SQL error: %s\n", zErrMsg);
                            sqlite3_free(zErrMsg);
                        }
                        else if (stod(usd_balance) >= stockPrice) {

                            // If the user had enough balance, update the new balance.
                            double difference = stod(usd_balance) - stockPrice;
                            std::string sql = "UPDATE users SET usd_balance=" + std::to_string(difference) + " WHERE ID =" + id + ";";
                            rc = sqlite3_exec(db, sql.c_str(), callback, 0, &zErrMsg);
                            std::cout << "[BUY] User Balance Updated: " << difference << std::endl;

                            //  Check SQL status
                            if (rc != SQLITE_OK) {
                                fprintf(stderr, "[BUY] SQL error: %s\n", zErrMsg);
                                sqlite3_free(zErrMsg);
                            }

                            // Check if record exists in table, then updates or creates a new one
                            sql = "SELECT IIF(EXISTS(SELECT 1 FROM stocks WHERE stocks.stock_symbol='" + infoArr[0] + "' AND stocks.user_id='" + id + "'), 'RECORD_PRESENT', 'RECORD_NOT_PRESENT') result;";
                            rc = sqlite3_exec(db, sql.c_str(), callback, ptr, &zErrMsg);

                            if (rc != SQLITE_OK) {
                                fprintf(stderr, "[BUY] SQL error: %s\n", zErrMsg);
                                sqlite3_free(zErrMsg);
                            }
                            else if (resultant == "RECORD_PRESENT") {
                                // Record exists in the table - update it
                                sql = "UPDATE stocks SET stock_balance= stock_balance +" + infoArr[1] + " WHERE stocks.stock_symbol='" + infoArr[0] + "' AND stocks.user_id='" + id + "';";
                                rc = sqlite3_exec(db, sql.c_str(), NULL, NULL, &zErrMsg);
                                std::cout << "[BUY] Added " << infoArr[1] << " stock to " << infoArr[0] << " for " << id << std::endl;

                                //  Check SQL status
                                if (rc != SQLITE_OK) {
                                    fprintf(stderr, "[BUY] SQL error: %s\n", zErrMsg);
                                    sqlite3_free(zErrMsg);
                                }
                            }
                            else {
                                // A record does not exist, so add a record
                                sql = "INSERT INTO stocks(stock_symbol, stock_balance, user_id) VALUES ('" + infoArr[0] + "', '" + infoArr[1] + "', '" + id + "');";
                                rc = sqlite3_exec(db, sql.c_str(), NULL, NULL, &zErrMsg);
                                std::cout << "[BUY] New record created:\n\tStock Symbol: " << infoArr[0] << "\n\tStock Balance: " << infoArr[1] << "\n\tUserID: " << id << std::endl;

                                //  Check SQL status
                                if (rc != SQLITE_OK) {
                                    fprintf(stderr, "[BUY] SQL error: %s\n", zErrMsg);
                                    sqlite3_free(zErrMsg);
                                }
                            }

                            // Balance
                            sql = "SELECT usd_balance FROM users WHERE users.ID=" + id;
                            rc = sqlite3_exec(db, sql.c_str(), callback, ptr, &zErrMsg);
                            usd_balance = resultant;

                            //  Check SQL status
                            if (rc != SQLITE_OK) {
                                fprintf(stderr, "[BUY] SQL error: %s\n", zErrMsg);
                                sqlite3_free(zErrMsg);
                            }

                            // Get the new stock_balance
                            sql = "SELECT stock_balance FROM stocks WHERE stocks.stock_symbol='" + infoArr[0] + "' AND stocks.user_id='" + id + "';";
                            rc = sqlite3_exec(db, sql.c_str(), callback, ptr, &zErrMsg);

                            // Check SQL status
                            if (rc != SQLITE_OK) {
                                fprintf(stderr, "[BUY] SQL error: %s\n", zErrMsg);
                                sqlite3_free(zErrMsg);
                                //send(clientID, "SQL error", 10, 0);
                            }
                            std::string stock_balance = resultant;

                            // The command completed successfully, return 200 OK, the new usd_balance and new stock_balance
                            std::string tempStr = "[BUY] 200 OK\n   [BOUGHT] New balance: " + stock_balance + " " + infoArr[0] + ". USD Balance $" + usd_balance;
                            send(clientID, tempStr.c_str(), sizeof(buf), 0);
                        }
                        else {
                            std::cout << "[SERVER] Not enough balance. Purchase Aborted." << std::endl;
                            send(clientID, "[BUY] 403 message format error: not enough balance!", sizeof(Buff), 0);
                        }
                    }
                    else {
                        // USER DOES NOT EXIST
                        fprintf(stdout, "[SERVER] User Does Not Exist in Users Table. Aborting Buy\n");
                        std::string tempStr = "[BUY] 403 message format error: user " + id + " does not exist";
                        send(clientID, tempStr.c_str(), sizeof(Buff), 0);
                    }
                }

                std::cout << "[SERVER] Successfully executed BUY command\n\n";
            }
            else if (command == "SELL") {
                std::cout << "[SELL] Sell command!" << std::endl;
                // Check if the client used the command properly
                if (!extractInfo(Buff, infoArr, command)) {
                    std::cout << "[SELL] Invalid command: Missing information" << std::endl;
                    send(clientID, "[SELL] 403 message format error: Missing information\n EX. Command: SELL stock_symbol amount price userID", sizeof(Buff), 0);
                }
                else {
                    // Check if the user exists in users table 
                    std::string sql = "SELECT IIF(EXISTS(SELECT 1 FROM users WHERE users.ID=" + id + "), 'PRESENT', 'NOT_PRESENT') result;";
                    rc = sqlite3_exec(db, sql.c_str(), callback, ptr, &zErrMsg);

                    if (rc != SQLITE_OK) {
                        fprintf(stderr, "[SELL] SQL error: %s\n", zErrMsg);
                        sqlite3_free(zErrMsg);
                    }
                    else if (resultant == "PRESENT") {
                        // Check if the user owns the selected stock
                        sql = "SELECT IIF(EXISTS(SELECT 1 FROM stocks WHERE stocks.stock_symbol='" + infoArr[0] + "' AND stocks.user_id='" + id + "'), 'RECORD_PRESENT', 'RECORD_NOT_PRESENT') result;";
                        rc = sqlite3_exec(db, sql.c_str(), callback, ptr, &zErrMsg);

                        if (rc != SQLITE_OK) {
                            fprintf(stderr, "[SELL] SQL error: %s\n", zErrMsg);
                            sqlite3_free(zErrMsg);
                        }
                        else if (resultant == "RECORD_NOT_PRESENT") {
                            std::cout << "[SERVER] User doesn't own the selected stock. Aborting Sell\n";
                            send(clientID, "[SELL] 403 message format error: User does not own this stock.", sizeof(Buff), 0);
                        }
                        else {
                            // Check if the user has enough of the selected stock to sell
                            double stocksToSell = std::stod(infoArr[1]);
                            // Get the numbers of stock user has of a specific symbol
                            sql = "SELECT stock_balance FROM stocks WHERE stocks.stock_symbol='" + infoArr[0] + "' AND stocks.user_id='" + id + "';";
                            rc = sqlite3_exec(db, sql.c_str(), callback, ptr, &zErrMsg);

                            if (rc != SQLITE_OK) {
                                fprintf(stderr, "[SELL] SQL error: %s\n", zErrMsg);
                                sqlite3_free(zErrMsg);
                            }

                            double stock_balance = std::stod(resultant);
                            // Not enough stocks in balance to sell
                            if (stock_balance < stocksToSell) {
                                std::cout << "[SERVER] Attempting to sell more stocks than the user has. Aborting sell.\n";
                                send(clientID, "[SELL] 403 message format error: Attempting to sell more stocks than the user has.", sizeof(Buff), 0);
                            }
                            else {
                                // Get dollar amount to sell
                                double stockPrice = std::stod(infoArr[1]) * std::stod(infoArr[2]);

                                // Update the user's balance
                                sql = "UPDATE users SET usd_balance= usd_balance +" + std::to_string(stockPrice) + " WHERE users.ID='" + id + "';";
                                rc = sqlite3_exec(db, sql.c_str(), NULL, NULL, &zErrMsg);

                                if (rc != SQLITE_OK) {
                                    fprintf(stderr, "[SELL] SQL error: %s\n", zErrMsg);
                                    sqlite3_free(zErrMsg);
                                }

                                // Updates Stocks table and removes the sold stock from table
                                sql = "UPDATE stocks SET stock_balance= stock_balance -" + std::to_string(stocksToSell) + " WHERE stocks.stock_symbol='" + infoArr[0] + "' AND stocks.user_id='" + id + "';";
                                rc = sqlite3_exec(db, sql.c_str(), NULL, NULL, &zErrMsg);

                                if (rc != SQLITE_OK) {
                                    fprintf(stderr, "[SELL] SQL error: %s\n", zErrMsg);
                                    sqlite3_free(zErrMsg);
                                }


                                // Get new usd_balance
                                sql = "SELECT usd_balance FROM users WHERE users.ID=" + id;
                                rc = sqlite3_exec(db, sql.c_str(), callback, ptr, &zErrMsg);
                                std::string usd_balance = resultant;

                                // Get new stock_balance
                                sql = "SELECT stock_balance FROM stocks WHERE stocks.stock_symbol='" + infoArr[0] + "' AND stocks.user_id='" + id + "';";
                                rc = sqlite3_exec(db, sql.c_str(), callback, ptr, &zErrMsg);
                                std::string stock_balance = resultant;

                                // Sell command completed successfully
                                std::string tempStr = "[SELL] 200 OK\n   [SOLD] New balance: " + stock_balance + " " + infoArr[0] + ". USD $" + usd_balance;
                                send(clientID, tempStr.c_str(), sizeof(Buff), 0);
                            }
                        }
                    }
                    else {
                        fprintf(stdout, "[SERVER] User Does Not Exist in Users Table. Aborting Sell.\n");
                        send(clientID, "[SELL] 403 message format error: user does not exist.", sizeof(Buff), 0);
                    }
                }
                std::cout << "[SERVER] Successfully executed SELL command\n\n";
            }
            else if (command == "LIST") {
                if (idINT == 1) {
                    std::cout << "[LIST] List command." << std::endl;
                    resultant = "";

                    // List all records in stocks table for user_id = 1
                    std::string sql = "SELECT * FROM stocks";

                    // Execute SQL statement 
                    rc = sqlite3_exec(db, sql.c_str(), callback, ptr, &zErrMsg);

                    if (rc != SQLITE_OK) {
                        fprintf(stderr, "[LIST] SQL error: %s\n", zErrMsg);
                        sqlite3_free(zErrMsg);
                    }

                    std::string sendStr;

                    if (resultant == "") {
                        sendStr = "[LIST] 200 OK\n   [LIST] No records in the Stock Database.";
                    }
                    else {
                        sendStr = "[LIST] 200 OK\n   [LIST] The list of records in the Stock database:\nStockID  stock_symbol stock_amount  UserID\n   " + resultant;
                    }
                    send(clientID, sendStr.c_str(), sizeof(Buff), 0);
                }
                else {
                    std::cout << "[LIST] List command." << std::endl;
                    resultant = "";

                    // List all records in stocks table for user_id = 1
                    std::string sql = "SELECT * FROM stocks WHERE stocks.user_id=" + id;

                    // Execute SQL statement
                    rc = sqlite3_exec(db, sql.c_str(), callback, ptr, &zErrMsg);

                    if (rc != SQLITE_OK) {
                        fprintf(stderr, "[LIST] SQL error: %s\n", zErrMsg);
                        sqlite3_free(zErrMsg);
                    }

                    std::string sendStr;

                    if (resultant == "") {
                        sendStr = "[LIST] 200 OK\n   [LIST] No records in the Stocks Database.";
                    }
                    else {
                        sendStr = "[LIST] 200 OK\n   [LIST] The list of records in the Stocks database:\nStockID  stock_symbol stock_amount  UserID\n   " + resultant;
                    }
                    send(clientID, sendStr.c_str(), sizeof(Buff), 0);
                }
            }
            else if (command == "BALANCE") {
                std::cout << "[BALANCE] Balance command!" << std::endl;
                std::string sql = "SELECT IIF(EXISTS(SELECT 1 FROM users WHERE users.ID=" + id + "), 'PRESENT', 'NOT_PRESENT') result;";

                // Execute SQL statement
                rc = sqlite3_exec(db, sql.c_str(), callback, ptr, &zErrMsg);

                if (rc != SQLITE_OK) {
                    fprintf(stderr, "[BALANCE] SQL error: %s\n", zErrMsg);
                    sqlite3_free(zErrMsg);
                }
                else if (resultant == "PRESENT") {
                    // Outputs balance
                    sql = "SELECT usd_balance FROM users WHERE users.ID=" + id;
                    rc = sqlite3_exec(db, sql.c_str(), callback, ptr, &zErrMsg);
                    std::string usd_balance = resultant;

                    // Outputs full name
                    sql = "SELECT first_name FROM users WHERE users.ID=" + id;
                    rc = sqlite3_exec(db, sql.c_str(), callback, ptr, &zErrMsg);
                    std::string user_name = resultant;

                    sql = "SELECT last_name FROM users WHERE users.ID=" + id;
                    rc = sqlite3_exec(db, sql.c_str(), callback, ptr, &zErrMsg);
                    user_name += " " + resultant;

                    std::string tempStr = "[BALANCE] 200 OK\n  [BALANCE]  Balance for user " + user_name + ": $" + usd_balance;
                    send(clientID, tempStr.c_str(), sizeof(Buff), 0);
                }
                else {
                    std::cout << "[SERVER] User does not exist. Aborting Balance.\n";
                    send(clientID, "[BALANCE] User does not exist.", sizeof(Buff), 0);
                }
            }
            else if (command == "QUIT") {
                std::cout << "[CLIENT] Quit command!" << std::endl;
                send(clientID, "200 OK", 27, 0);
                for (int i = 0; i < list.size(); i++) {
                    if (list.at(i).user == u)
                        list.erase(list.begin() + i);
                }
                nClient[clientIndex] = 0;
                close(clientID);
                pthread_exit(userData);

                return userData;
            }
            else if (command == "SHUTDOWN" && rootUsr) {
                send(clientID, "200 OK", 7, 0);
                sqlite3_close(db);
                std::cout << "[DB] Closed DB" << std::endl;
                close(clientID);
                std::cout << "[CLIENT] Closed Client Connection: " << clientID << std::endl;
                for (int i = 0; i < list.size(); i++) {
                    if (list.at(i).user == u)
                        list.erase(list.begin() + i);
                }
                for (int i = 0; i < list.size(); i++) {
                    close(nClient[list.at(i).socket]);
                    pthread_cancel((list.at(i)).userThread);
                }
                close(nSocket);
                std::cout << "[SERVER] Closed Server socket: " << nSocket << std::endl;
                pthread_exit(userData);

                exit(EXIT_SUCCESS);
            }
            else if (command == "LOGOUT") {
                std::cout << "[LOGOUT] Logout command!" << std::endl;
                send(clientID, "200 OK", 7, 0);
                for (int i = 0; i < list.size(); i++) {
                    if (list.at(i).user == u)
                        list.erase(list.begin() + i);
                }
                nClient[clientIndex] = clientID;
                pthread_exit(userData);
                return userData;
            }
            else if (command == "DEPOSIT") {
                std::cout << "[DEPOSIT] Deposit command" << std::endl;
                std::string sql = "SELECT IIF(EXISTS(SELECT 1 FROM users WHERE users.ID=" + id + "), 'PRESENT', 'NOT_PRESENT') result;";

                // Execute SQL statement
                rc = sqlite3_exec(db, sql.c_str(), callback, ptr, &zErrMsg);

                if (rc != SQLITE_OK) {
                    fprintf(stderr, "[DEPOSIT] SQL error: %s\n", zErrMsg);
                    sqlite3_free(zErrMsg);
                }
                else if (resultant == "PRESENT") {
                    // Outputs balance
                    std::string deposit = "";

                    for (int i = (command.length() + 1); i < strlen(Buff); i++) {
                        if (Buff[i] == '\n')
                            break;
                        deposit += Buff[i];
                    }

                    sql = "UPDATE users SET usd_balance= usd_balance +" + deposit + " WHERE users.ID='" + id + "';";
                    rc = sqlite3_exec(db, sql.c_str(), callback, ptr, &zErrMsg);

                    sql = "SELECT usd_balance FROM users WHERE users.ID=" + id;
                    rc = sqlite3_exec(db, sql.c_str(), callback, ptr, &zErrMsg);
                    std::string usd_balance = resultant;

                    // Outputs full name
                    sql = "SELECT first_name FROM users WHERE users.ID=" + id;
                    rc = sqlite3_exec(db, sql.c_str(), callback, ptr, &zErrMsg);
                    std::string user_name = resultant;

                    sql = "SELECT last_name FROM users WHERE users.ID=" + id;
                    rc = sqlite3_exec(db, sql.c_str(), callback, ptr, &zErrMsg);
                    user_name += " " + resultant;

                    std::string tempStr = "[SERVER] 200 OK\n  [BALANCE] New balance for user " + user_name + ": $" + usd_balance;
                    send(clientID, tempStr.c_str(), sizeof(Buff), 0);
                }
                else {
                    std::cout << "[SERVER] User does not exist. Aborting Balance.\n";
                    send(clientID, "[CLIENT] User does not exist.", sizeof(Buff), 0);
                }
            }
            else if (command == "WHO" && rootUsr) {
                std::cout << "[WHO] Who command!" << std::endl;
                std::string result = "[WHO] 200 OK\nThe list of the active users:\n";
                for (int i = 0; i < list.size(); i++) {
                    result += (list.at(i).user + " " + list.at(i).ip + "\n");
                }

                send(clientID, result.c_str(), sizeof(Buff), 0);

            }
            else if (command == "LOOKUP") {
                std::cout << "[LOOKUP] Lookup command!" << std::endl;
                std::string searchTerm = "";
                std::string sendStr;
                resultant = "";
                for (int i = (command.length() + 1); i < strlen(Buff); i++) {
                    if (Buff[i] == '\n')
                        break;
                    searchTerm += Buff[i];
                }
                std::string sql = "SELECT COUNT(stock_symbol) FROM (SELECT * FROM stocks WHERE user_id = " + id + ") WHERE stock_symbol LIKE '%" + searchTerm + "%';";
                rc = sqlite3_exec(db, sql.c_str(), callback, ptr, &zErrMsg);
                std::string count = resultant;
                resultant = "";
                sql = "SELECT stock_symbol, stock_balance FROM (SELECT * FROM stocks WHERE user_id = " + id + ") WHERE stock_symbol LIKE '%" + searchTerm + "%'";
                rc = sqlite3_exec(db, sql.c_str(), callback, ptr, &zErrMsg);

                if (resultant == "") {
                    sendStr = "[LOOKUP] 404 Your search did not match any records";
                }
                else {
                    sendStr = "[LOOKUP] 200 OK\n   Found:" + count + "\nStock_Name Stock_Amount\n   " + resultant;
                }
                send(clientID, sendStr.c_str(), sizeof(Buff), 0);

            }
            // Default response to invalid command
            else {
                std::cout << "[SERVER] Command not recognized" << std::endl;
                send(clientID, "[SERVER] 400 invalid command", 20, 0);
            }
        }

        for (int i = 0; i < list.size(); i++) {
            if (list.at(i).user == u)
                list.erase(list.begin() + i);
        }

        std::cout << std::endl << "[CRITICAL] Client Socket Error!\n";
        nClient[clientIndex] = 0;
        close(clientID);
        pthread_exit(userData);

        return userData;
    }
}

bool extractInfo(char line[], std::string info[], std::string command) {
    int l = command.length();
    int spaceLocation = l + 1;

    for (int i = 0; i < 3; i++) {
        info[i] = "";

        // Parses the information
        for (int j = spaceLocation; j < strlen(line); j++) {

            if (line[j] == 0)
                return false;
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
            std::fill_n(info, 3, 0);
            return false;
        }

        spaceLocation += info[i].length() + 1;

    }

    return true;
}

static int callback(void* ptr, int count, char** data, char** azColName) {

    if (count == 1) {
        resultant = data[0];
    }
    else if (count > 1) {
        for (int i = 0; i < count; i++) {

            if (resultant == "") {
                resultant = data[i];
            }
            else {
                resultant = resultant + " " + data[i];
            }

            // Create a new line btween every record
            if (i == 3)
            {
                resultant += "\n  ";
            }

        }
    }
    return 0;
}

void newConnection()
{
    // Adds a new client under the new file descriptor, allows client communication
    int nNewClient = accept(nSocket, (struct sockaddr*)&srv, &addr_len);

    if (nNewClient < 0) {
        perror("[ERROR] Error accepting connection (newConnnection)");
    }
    else {

        void* temp = &nNewClient;
        std::cout << "[SOCKET] Client: " << nNewClient << std::endl;

        int nIndex;
        for (nIndex = 0; nIndex < 5; nIndex++)
        {
            if (nClient[nIndex] == 0)
            {
                nClient[nIndex] = nNewClient;
                if (nNewClient > nMaxFd)
                {
                    nMaxFd = nNewClient + 1;
                }
                break;
            }
        }

        if (nIndex == 5)
        {
            std::cout << std::endl << "[SERVER] Server Busy!";
        }

        printf("[NEW CONNECTION] Socket FD is %d, IP is: %s, PORT: %d\n", nNewClient, inet_ntoa(srv.sin_addr), ntohs(srv.sin_port));
        send(nClient[nIndex], "[SERVER] You are now connected!", 47, 0);
    }
}

void clientData()
{
    std::string command;
    temp = &u;

    for (int nIndex = 0; nIndex < 5; nIndex++) // Limits to 5 connections at a time
    {
        if (nClient[nIndex] > 0)
        {
            if (FD_ISSET(nClient[nIndex], &fr))
            {
                //Read the data from client
                char sBuff[256] = { 0, };
                int nRet = recv(nClient[nIndex], sBuff, 256, 0);
                if (nRet < 0)
                {
                    //This happens when client closes connection abruptly
                    std::cout << std::endl << "[ERROR] @ Client Socket";
                    for (int i = 0; i < list.size(); i++) {
                        if (list.at(i).user == u.user)
                            list.erase(list.begin() + i);
                    }
                    close(nClient[nIndex]);
                    nClient[nIndex] = 0;
                }
                else
                {

                    command = buildCommand(sBuff);
                    std::cout << command << std::endl;

                    if (command == "LOGIN") {
                        std::string info = extractInfo(sBuff, command);
                        userConnect tempStruct;
                        u.user = info;
                        int passLength = command.length() + info.length();
                        std::string passInfo = getPassword(sBuff, passLength);

                        u.password = passInfo;
                        u.socket = nIndex;
                        tempStruct.socket = nIndex;
                        struct sockaddr_in client_addr;
                        socklen_t addrlen;

                        std::cout << "[LOGIN] Username: " << info << " Socket: " << u.socket << std::endl;

                        std::string commandSql = "SELECT IIF(EXISTS(SELECT * FROM users WHERE user_name = '" + info + "' AND password = '" + passInfo + "') , 'USER_PRESENT', 'USER_NOT_PRESENT') result;";
                        sql = commandSql.c_str();
                        sqlite3_exec(db, sql, callback, 0, &zErrMsg);

                        if (resultant == "USER_PRESENT") {
                            std::cout << "[LOGIN] Attempting Login. " << std::endl;
                            send(nClient[nIndex], "200 OK", 7, 0);

                            getpeername(nClient[nIndex], (struct sockaddr*)&client_addr, &addrlen);
                            tempStruct.ip = "";
                            std::cout << "[LOGIN] IP address: " << inet_ntoa(client_addr.sin_addr) << std::endl;
                            tempStruct.ip = inet_ntoa(client_addr.sin_addr);
                            std::cout << tempStruct.ip << std::endl;
                            
                            tempStruct.user = u.user;

                            list.push_back(tempStruct);

                            commandSql = "SELECT ID FROM users WHERE user_name = '" + info + "' AND password = '" + passInfo + "'";
                            sql = commandSql.c_str();
                            sqlite3_exec(db, sql, callback, 0, &zErrMsg);
                            u.id = stoi(resultant);

                            pthread_create(&(list.at(list.size() - 1).userThread), NULL, serverCommands, temp);
                        }
                        else {
                            std::cout << "[LOGIN] Invalid!" << std::endl;
                        }
                    }
                    else if (command == "QUIT") {
                        std::cout << "[QUIT] Quit command!" << std::endl;
                        send(nClient[nIndex], "200 OK", 27, 0);
                        close(nClient[nIndex]);
                        nClient[nIndex] = 0;
                    }
                    else if (command == "BUY" || command == "SELL" || command == "LOOKUP" || command == "DEPOSIT" || command == "SHUTDOWN" || command == "LIST" || command == "WHO" || command == "BALANCE" || command == "LOGOUT") {
                        send(nClient[nIndex], "[ERROR] Guest users can only LOGIN or QUIT.", 54, 0);
                    }
                    else {
                        std::cout << std::endl << "Received data from:" << nClient[nIndex] << "[Message: " << sBuff << " size of array: " << strlen(sBuff) << "] Error 400" << std::endl;
                        send(nClient[nIndex], "[ERROR] Command does not exist.", 24, 0);
                    }
                    break;
                }
            }
        }
    }
}

std::string getPassword(char line[], int n) {

    int spaceLocation = n + 2;
    int i = spaceLocation;
    std::string info = "";

    while (line[i] != '\n') {
        if (line[i] == 0)
            return "";
        if (line[i] == ' ')
            return info;
        info += line[i];
        i++;
    }
    return info;
}