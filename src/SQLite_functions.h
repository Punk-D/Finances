#include <fstream>
#include <iostream>
#include "sqlite3.h"



sqlite3* db;

sqlite3* create_or_open_db()
{
    const char* enableForeignKeysSQL = "PRAGMA foreign_keys = ON;";

    std::ifstream file("localdb.db");
    if (!file.is_open())
    {
        // The file does not exist, create it
        std::ofstream createFile("localdb.db");
        createFile.close();
    }
    int databaseCode = sqlite3_open("localdb.db", &db);
    if (databaseCode != SQLITE_OK)
    {
        std::cout << "Database problem code " << databaseCode << std::endl;

    }
    else
    {
        std::cout << "Database OK" << std::endl;
        return db;
    }
}

void createTables(sqlite3* db) {
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

    result = sqlite3_exec(db, createBrokerTableSQL, 0, 0, 0);
    if (result != SQLITE_OK) {
        std::cout << "Table broker creation error: " << sqlite3_errmsg(db) << std::endl;
        // Handle error if necessary
    }
    else {
        std::cout << "Table broker is OK" << std::endl;
    }

    result = sqlite3_exec(db, createClientTableSQL, 0, 0, 0);
    if (result != SQLITE_OK) {
        std::cout << "Table client creation error: " << sqlite3_errmsg(db) << std::endl;
        // Handle error if necessary
    }
    else {
        std::cout << "Table client is OK" << std::endl;
    }

    result = sqlite3_exec(db, createTransactionTableSQL, 0, 0, 0);
    if (result != SQLITE_OK) {
        std::cout << "Table transaction creation error: " << sqlite3_errmsg(db) << std::endl;
        // Handle error if necessary
    }
    else {
        std::cout << "Table transaction is OK" << std::endl;
    }

    result = sqlite3_exec(db, createAccountsTableSQL, 0, 0, 0);
    if (result != SQLITE_OK) {
        std::cerr << "Table accounts creation error: " << sqlite3_errmsg(db) << std::endl;
        // Handle error if necessary
    }
    else {
        std::cout << "Table accounts is OK" << std::endl;
    }
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