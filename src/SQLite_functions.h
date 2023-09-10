#include <fstream>
#include "sqlite3.h"

sqlite3* db;

void create_or_open_db()
{
    
    
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
        std::cout << "Database problem code "<<databaseCode << std::endl;
    }
    else
    {

    }
}