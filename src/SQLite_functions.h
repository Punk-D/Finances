#include <fstream>
#include <iostream>
#include "sqlite3.h"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>


auto logger = spdlog::basic_logger_mt("log", "logs1.txt");


sqlite3* db;
sqlite3* keybase;

sqlite3* create_or_open_db(sqlite3** dbo,std::string dbname)
{
    logger->trace("Create/open database was called");
    const char* enableForeignKeysSQL = "PRAGMA foreign_keys = ON;";
    
    std::ifstream file(dbname);
    if (!file.is_open())
    {
        // The file does not exist, create it
        logger->trace("SQLite database does not exist, create file localdb called");
        std::ofstream createFile(dbname);
        createFile.close();
    }
    else { logger->trace("SQLite {} opened successfuly", dbname); }
    int databaseCode = sqlite3_open(dbname.c_str(), dbo);
    if (databaseCode != SQLITE_OK)
    {
        std::cout << "Database problem code "<<databaseCode << std::endl;
        logger->error("SQLite database {} not opened. Error : {}",dbname, databaseCode);
    }
    else
    {
        logger->trace("Database {} opened successfuly",dbname);
        std::cout << "Database OK" << std::endl;
        return *dbo;
    }
}

bool createTables(sqlite3* db) {
    bool isOk = true;
    const char* createAccountsTableSQL =
        "CREATE TABLE IF NOT EXISTS accounts ("
        "account_id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "account_type INTEGER NOT NULL,"
        "account_reference INTEGER,"
        "email TEXT NOT NULL UNIQUE,"
        "phone_number TEXT NOT NULL UNIQUE,"
        "login TEXT NOT NULL UNIQUE,"
        "password TEXT NOT NULL"
        ");";

    // SQL statements to create tables
    const char* createBrokerTableSQL =
        "CREATE TABLE IF NOT EXISTS broker ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "name TEXT NOT NULL,"
        "surname TEXT NOT NULL,"
        "portfolio_percent REAL NOT NULL,"
        "api_key TEXT NOT NULL"
        ");";

    const char* createClientTableSQL =
        "CREATE TABLE IF NOT EXISTS client ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "name TEXT NOT NULL,"
        "surname TEXT NOT NULL,"
        "cut_percent REAL NOT NULL,"
        "portfolio_percent REAL NOT NULL," 
        "broker_id INT NOT NULL,"
        "FOREIGN KEY (broker_id) REFERENCES broker (id)"
        ");";

    const char* createTransactionTableSQL =
        "CREATE TABLE IF NOT EXISTS transactions("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "broker_id INTEGER NOT NULL,"
        "client_id INTEGER,"
        "transaction_type INTEGER NOT NULL,"
        "transaction_initiator INTEGER NOT NULL,"
        "balance_before_transaction REAL NOT NULL,"
        "balance_after_transaction REAL NOT NULL,"
        "transaction_sum REAL NOT NULL,"
        "FOREIGN KEY (broker_id) REFERENCES broker (id),"
        "FOREIGN KEY (client_id) REFERENCES client (id)"
        ");";

    // Execute the SQL statements to create tables
    int result;

    logger->trace("create broker table called");
    result = sqlite3_exec(db, createBrokerTableSQL, 0, 0, 0);
    if (result != SQLITE_OK) {
        std::cout << "Table broker creation error: " << sqlite3_errmsg(db) << std::endl;
        logger->error("create broker table error {}", sqlite3_errmsg(db));
        isOk = false;
        // Handle error if necessary
    }
    else {
        std::cout << "Table broker is OK" << std::endl;
        logger->trace("Broker table created successfully");
    }

    result = sqlite3_exec(db, createClientTableSQL, 0, 0, 0);
    if (result != SQLITE_OK) {
        std::cout << "Table client creation error: " << sqlite3_errmsg(db) << std::endl;
        logger->error("create client table error {}", sqlite3_errmsg(db));
        isOk = false;
        // Handle error if necessary
    }
    else {
        std::cout << "Table client is OK" << std::endl;
        logger->trace("client table created successfuly");
    }

    result = sqlite3_exec(db, createTransactionTableSQL, 0, 0, 0);
    if (result != SQLITE_OK) {
        std::cout << "Table transaction creation error: " << sqlite3_errmsg(db) << std::endl;
        logger->error("create transaction table error {}", sqlite3_errmsg(db));
        isOk = false;
        // Handle error if necessary
    }
    else {
        std::cout << "Table transaction is OK" << std::endl;
        logger->trace("transaction table created successfully");
    }

    result = sqlite3_exec(db, createAccountsTableSQL, 0, 0, 0);
    if (result != SQLITE_OK) {
        std::cerr << "Table accounts creation error: " << sqlite3_errmsg(db) << std::endl;
        logger->error("create accounts table error {}", sqlite3_errmsg(db));
        isOk = false;
        // Handle error if necessary
    }
    else {
        std::cout << "Table accounts is OK" << std::endl;
        logger->trace("accounts table created successfully");
    }
    return isOk;
}


// Insert a client into the database


// Insert a transaction into the database


int countRows(const char* tableName) {
    const char* countQuery = "SELECT COUNT(*) FROM %s;";
    char query[100]; // Assuming table name won't exceed 90 characters

    // Create the SQL query with the provided table name
    snprintf(query, sizeof(query), countQuery, tableName);

    sqlite3_stmt* stmt;
    int result = sqlite3_prepare_v2(db, query, -1, &stmt, 0);
    if (result != SQLITE_OK) {
        std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
        return result;
    }

    int rowCount = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        rowCount = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return rowCount;
}

bool create_api_key_decrypt_tables(sqlite3* keybase)
{
    bool isOk = true;
    const char* createKeysTableSQL =
        "CREATE TABLE IF NOT EXISTS keys ("
        "brokerid INTEGER PRIMARY KEY,"
        "key TEXT NOT NULL,"
        "iv TEXT NOT NULL"
        ");";
    const char* createSaltTableSQL =
        "CREATE TABLE IF NOT EXISTS salt ("
        "username TEXT PRIMARY KEY,"
        "salt TEXT NOT NULL"
        ");";
    int result; char* errormessage;
    result=sqlite3_exec(keybase, createKeysTableSQL, nullptr, nullptr, &errormessage);
    if (result != SQLITE_OK)
    {
        std::cout << "Table keys creation error: " << sqlite3_errmsg(keybase) << std::endl;
        logger->error("create keys table error {}", sqlite3_errmsg(keybase));
        isOk = false;
    }
    else
    {
        std::cout << "Table keys is OK" << std::endl;
        logger->trace("keys table created successfully");
    }

    result = sqlite3_exec(keybase, createSaltTableSQL, nullptr, nullptr, &errormessage);
    if (result != SQLITE_OK)
    {
        std::cout << "Table salt creation error: " << sqlite3_errmsg(keybase) << std::endl;
        logger->error("create salt table error {}", sqlite3_errmsg(keybase));
        isOk = false;
    }
    else
    {
        std::cout << "Table salt is OK" << std::endl;
        logger->trace("salt table created successfully");
    }
    return isOk;
}
void insertKeysTable(int brokerId, std::string key, std::string iv) {
    // Prepare the SQL statement
    const char* insertSQL = "INSERT INTO keys (brokerid, key, iv) VALUES (?, ?, ?);";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(keybase, insertSQL, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Error preparing SQL statement: " << sqlite3_errmsg(keybase) << std::endl;
        return;
    }

    // Bind the brokerId parameter
    if (sqlite3_bind_int(stmt, 1, brokerId) != SQLITE_OK) {
        std::cerr << "Error binding brokerId parameter: " << sqlite3_errmsg(keybase) << std::endl;
        sqlite3_finalize(stmt);
        return;
    }

    if (sqlite3_bind_text(stmt, 2, key.c_str(), -1, SQLITE_STATIC) != SQLITE_OK) {
        std::cerr << "Error binding key parameter: " << sqlite3_errmsg(keybase) << std::endl;
        sqlite3_finalize(stmt);
        return;
    }

    if (sqlite3_bind_text(stmt, 3, iv.c_str(), -1, SQLITE_STATIC) != SQLITE_OK) {
        std::cerr << "Error binding iv parameter: " << sqlite3_errmsg(keybase) << std::endl;
        sqlite3_finalize(stmt);
        return;
    }

    // Execute the SQL statement
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::cerr << "Error inserting data: " << sqlite3_errmsg(keybase) << std::endl;
    }
    else {
        std::cout << "Data inserted successfully." << std::endl;
    }

    // Finalize the statement
    sqlite3_finalize(stmt);
}

// Function to insert accid and salt into the salt table
bool insertSalt(const std::string& username, const std::string& salt) {
    const char* insertSQL = "INSERT INTO salt (username, salt) VALUES (?, ?);";
    sqlite3_stmt* stmt;

    int result = sqlite3_prepare_v2(keybase, insertSQL, -1, &stmt, NULL);
    if (result != SQLITE_OK) {
        std::cerr << "Failed to prepare INSERT statement: " << sqlite3_errmsg(keybase) << std::endl;
        return false;
    }

    // Bind the accid and salt parameters to the SQL statement
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, salt.c_str(), -1, SQLITE_STATIC);

    // Execute the SQL statement
    result = sqlite3_step(stmt);
    if (result != SQLITE_DONE) {
        std::cerr << "Failed to insert salt: " << sqlite3_errmsg(keybase) << std::endl;
        sqlite3_finalize(stmt);
        return false;
    }

    // Finalize the statement and return true if successful
    sqlite3_finalize(stmt);
    return true;
}