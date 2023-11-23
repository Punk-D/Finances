#include "SQLite_functions.h"
#include <winsock2.h>
#include "encrypt.h"
#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <bitset>
#include <chrono>
#include <thread>
#include <queue>
#include <condition_variable>
#include <cpprest/json.h>
#include <mutex>
#include <map>
#include <functional>
#include <future>
#include <pplx/pplxtasks.h>
#include <cstring>
#pragma comment(lib, "ws2_32.lib")

// Constant for salt lenght. Only affects salt that will be generated after changed
// Previously generated salt will remain the same size, the change will not create any trouble to already created lines
// Keep this at sizes below 100
const int saltlen = 10;

// Constant for session token lenght. Better keep higher than 10 and below 200
const int tokenlen = 20;

// Integer to manage how many hours a non-remember session lives
int sess_non_rem_len = 4;

// Integer to manage how many days a remember session lives
int sess_rem_len = 30;
// TODO restructure this to be safer
// superuser password.
std::string supass = "4132";

//debug function to test performance
class Timer {
public:
	Timer() : start_time_point(std::chrono::high_resolution_clock::now()) {}

	void start() {
		start_time_point = std::chrono::high_resolution_clock::now();
	}

	void end() {
		auto end_time_point = std::chrono::high_resolution_clock::now();
		auto start_duration = std::chrono::time_point_cast<std::chrono::microseconds>(start_time_point).time_since_epoch();
		auto end_duration = std::chrono::time_point_cast<std::chrono::microseconds>(end_time_point).time_since_epoch();
		auto duration = end_duration - start_duration;
		double ms = duration.count() * 0.001;

		std::cout << "Time taken: " << ms << " ms" << std::endl;
	}

private:
	std::chrono::time_point<std::chrono::high_resolution_clock> start_time_point;
};

//Verifies password by standart methodology (currently A,a,0 chars = 8)
bool isStrongPassword(const std::string& password) {
	// Check if the password length is at least 8 characters
	if (password.length() < 8) {
		return false;
	}

	bool hasLower = false;
	bool hasUpper = false;
	bool hasDigit = false;

	// Iterate through each character in the password
	for (char c : password) {
		// Check if the character is a lowercase letter
		if (std::islower(c)) {
			hasLower = true;
		}
		// Check if the character is an uppercase letter
		else if (std::isupper(c)) {
			hasUpper = true;
		}
		// Check if the character is a digit
		else if (std::isdigit(c)) {
			hasDigit = true;
		}
	}

	// Check if all three criteria are met
	return hasLower && hasUpper && hasDigit;
}

//verifies if a broker exists in the database by his id
bool does_broker_exist(int& errorcode, std::string& errormsg, int brokerid) {
	const char* selectSQL = "SELECT 1 FROM broker WHERE id = ? LIMIT 1;";

	sqlite3_stmt* stmt = nullptr;
	if (sqlite3_prepare_v2(db, selectSQL, -1, &stmt, nullptr) != SQLITE_OK) {
		sqlite3_finalize(stmt);
		return false;
	}

	if (sqlite3_bind_int(stmt, 1, brokerid) != SQLITE_OK) {
		errorcode = 502;
		errormsg = "Database could not be opened";
		sqlite3_finalize(stmt);
		return false;
	}

	if (sqlite3_step(stmt) == SQLITE_ROW) {
		sqlite3_finalize(stmt);
		errorcode = 0;
		return true;
	}

	sqlite3_finalize(stmt);
	errorcode = 0;
	return false;
}

//Checks if a username is present in the database
bool doesUsernameExist(int& errorcode, std::string& errormsg, const std::string& inputUsername) {
	// Define the SQL query
	const char* selectSQL = "SELECT 1 FROM accounts WHERE login = ? LIMIT 1;";

	// Prepare the statement
	sqlite3_stmt* stmt = nullptr;
	if (sqlite3_prepare_v2(db, selectSQL, -1, &stmt, nullptr) != SQLITE_OK) {
		// Handle the error
		logger->error("SQLite error: {}", sqlite3_errmsg(db));
		errormsg = "SQLite error: ";
		errormsg += sqlite3_errmsg(db);
		errorcode = SQLITE_ERROR;
		sqlite3_finalize(stmt);
		return false;
	}

	// Bind the input parameter (username)
	if (sqlite3_bind_text(stmt, 1, inputUsername.c_str(), -1, SQLITE_STATIC) != SQLITE_OK) {
		// Handle the error
		logger->error("Failed to bind username parameter: {}", sqlite3_errmsg(db));
		errormsg = "Failed to bind username parameter: ";
		errormsg += sqlite3_errmsg(db);
		errorcode = SQLITE_ERROR;
		sqlite3_finalize(stmt);
		return false;
	}

	// Execute the statement
	if (sqlite3_step(stmt) == SQLITE_ROW) {
		// Username exists in the database
		sqlite3_finalize(stmt); // Finalize the statement
		logger->info("Username '{}' exists in the database.", inputUsername);
		return true;
	}

	// Username does not exist in the database
	sqlite3_finalize(stmt); // Finalize the statement
	logger->info("Username '{}' does not exist in the database.", inputUsername);
	return false;
}

//Verifies if a superuser password is correct
bool sudo_reg_ver_API(std::string password)
{
	if (password == supass)
	{
		logger->info("Superuser registration failed, wrong password");
		return true;
	}
	else
	{
		return false;
	}
}

//Server only, do not use
//Deprecated for API usage. Instead use sudo_reg_ver_API
bool sudo_registration_verification()
{
	logger->trace("SU verification entered");
	std::string superuserpassword; bool verify = false;
	while (!verify)
	{
		std::cout << "Enter superuser registration password (ask your manager for it) : "; std::cin >> superuserpassword;
		if (superuserpassword == supass)
		{
			logger->trace("Password asked. Password received is correct");
			std::cout << "Password confirmed, proceeding to registration sequence" << std::endl;
			return true;
		}
		else
		{
			logger->trace("Password asked. Password received is incorrect. Leave or retry asked");
			std::cout << "Password incorrect. Do you want to leave verification sequence? (Y/N) : "; std::cin >> superuserpassword;
			if (superuserpassword == "Y")
			{
				logger->trace("Leave answer received. Returning to registration");
				std::cout << "Returning to registration sequence" << std::endl;
				return false;
			}
			else if (superuserpassword == "N")
			{
				logger->trace("Retry received. Returning to verification");
				std::cout << "Returning to verification sequence" << std::endl;
			}
			else
			{
				logger->trace("Bad input received. Returning to registration");
				std::cout << "Invalid input. It is a yes or no question. Returning to registration sequence" << std::endl;
				return false;
			}
		}
	}
}

// Session creation function
bool create_session(int& errorcode, std::string& errormsg, std::string CSRF, std::string& Token, int accid, bool remember)
{
	std::cout << "Create session called" << std::endl;
	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
	std::tm newTm;
	char buffer[20];

	if (remember)
	{
		std::chrono::hours duration(sess_rem_len * 24);
		std::chrono::system_clock::time_point newTime = now + duration;
		std::time_t newTimeT = std::chrono::system_clock::to_time_t(newTime);

		localtime_s(&newTm, &newTimeT);
		std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &newTm);
	}
	else
	{
		std::chrono::hours duration(sess_non_rem_len);
		std::chrono::system_clock::time_point newTime = now + duration;
		std::time_t newTimeT = std::chrono::system_clock::to_time_t(newTime);

		localtime_s(&newTm, &newTimeT);
		std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &newTm);
	}

	Token = generateSalt(tokenlen);
	bool untiltrue = false;

	while (!untiltrue)
	{
		if (insert_into_session(CSRF, Token, buffer, accid))
		{
			untiltrue = true;
		}
		else
		{
			Token = generateSalt(tokenlen);
		}
	}

	return true;
}

class broker
{
public:
	int id;
	std::string name, surname;
	float portfolio_percent;
	std::string api_key;

	//Uploads a broker object to the database
	int broker_upload(int& errorcode, std::string& errormsg) {
		// Create an INSERT statement for broker
		std::string insertBrokerSQL = "INSERT INTO broker (name, surname, portfolio_percent, api_key) "
			"VALUES (?, ?, ?, ?);";

		sqlite3_stmt* stmt;
		if (sqlite3_prepare_v2(db, insertBrokerSQL.c_str(), -1, &stmt, NULL) == SQLITE_OK) {
			sqlite3_bind_text(stmt, 1, this->name.c_str(), -1, SQLITE_STATIC);
			sqlite3_bind_text(stmt, 2, this->surname.c_str(), -1, SQLITE_STATIC);
			sqlite3_bind_double(stmt, 3, this->portfolio_percent);
			sqlite3_bind_text(stmt, 4, this->api_key.c_str(), -1, SQLITE_STATIC);

			if (sqlite3_step(stmt) == SQLITE_DONE) {
				// Broker inserted successfully, get the last inserted row ID
				int brokerId = sqlite3_last_insert_rowid(db);
				logger->info("Broker inserted successfully. Broker ID: {}", brokerId);
				sqlite3_finalize(stmt);
				return brokerId;
			}
			else {
				// Handle the error
				errorcode = SQLITE_ERROR;
				errormsg = "Failed to insert broker: " + std::string(sqlite3_errmsg(db));
				logger->error(errormsg);
			}

			sqlite3_finalize(stmt);
		}
		else {
			// Handle the error
			errorcode = SQLITE_ERROR;
			errormsg = "Failed to prepare INSERT statement: " + std::string(sqlite3_errmsg(db));
			logger->error(errormsg);
		}

		// Return -1 to indicate failure
		return -1;
	}

	//TODO Add error handling
	int addBrokerAPI(int& errorcode, std::string& errormsg)
	{
		std::string inputstring, iv, pass;
		iv = generateRandomKey(16); pass = generateRandomKey(32);
		this->api_key = aes256CBCEncrypt(inputstring, pass, iv);
		this->portfolio_percent = 0;
		int brokerid = this->broker_upload(errorcode, errormsg);

		insertKeysTable(brokerid, pass, iv);
		return brokerid;
	}

	//Server only, do not use
	int addBroker(int& errorcode, std::string& errormsg)
	{
		std::string inputstring, iv, pass;
		broker temp;
		std::cout << "Input name and surname : ";
		std::cin >> temp.name >> temp.surname;
		std::cout << "Input API key : "; std::cin >> inputstring;
		iv = generateRandomKey(16); pass = generateRandomKey(32);
		temp.api_key = aes256CBCEncrypt(inputstring, pass, iv);
		temp.portfolio_percent = 0;
		int brokerid = temp.broker_upload(errorcode, errormsg);

		insertKeysTable(brokerid, pass, iv);
		return brokerid;
	}

	//Server only, do not use
	void coutallbrokers()
	{
		broker cycle;
		for (int i = 0; i < countRows("broker"); i++)
		{
			cycle = broker_fetch(i);
			std::cout << "ID : " << cycle.id << " " << "Name : " << cycle.name << " " << cycle.surname << std::endl;
		}
	}

	//TODO Add error handling
	std::pair<std::string, std::string> getIVAndKey(int brokerid) {
		std::string iv;
		std::string key;

		// Prepare the SQL statement
		const char* selectSQL = "SELECT iv, key FROM keys WHERE brokerid = ?;";
		sqlite3_stmt* stmt;

		if (sqlite3_prepare_v2(keybase, selectSQL, -1, &stmt, nullptr) != SQLITE_OK) {
			std::cerr << "Error preparing SQL statement: " << sqlite3_errmsg(keybase) << std::endl;
			return std::make_pair("", "");
		}

		// Bind the brokerid parameter
		if (sqlite3_bind_int(stmt, 1, brokerid) != SQLITE_OK) {
			std::cerr << "Error binding brokerid parameter: " << sqlite3_errmsg(keybase) << std::endl;
			sqlite3_finalize(stmt);
			return std::make_pair("", "");
		}

		// Execute the SQL statement
		int result = sqlite3_step(stmt);

		if (result == SQLITE_ROW) {
			// Retrieve IV and key from the database
			const unsigned char* ivResult = sqlite3_column_text(stmt, 0);
			const unsigned char* keyResult = sqlite3_column_text(stmt, 1);

			if (ivResult && keyResult) {
				iv = reinterpret_cast<const char*>(ivResult);
				key = reinterpret_cast<const char*>(keyResult);
			}
		}
		else if (result != SQLITE_DONE) {
			std::cerr << "Error fetching data: " << sqlite3_errmsg(keybase) << std::endl;
		}

		// Finalize the statement
		sqlite3_finalize(stmt);

		return std::make_pair(iv, key);
	}

	//TODO Add error handling
	std::string decrypt_api_key()
	{
		std::pair<std::string, std::string>IvKey = this->getIVAndKey(this->id);
		//std::cout << IvKey.first << " "<<IvKey.second << std::endl;
		std::string res = aes256CBCDecrypt(this->api_key, IvKey.second, IvKey.first);
		//std::cout << res << std::endl;
		return res;
	}

	//TODO
	float portfolio_in_USDT()
	{
		std::cout << "API key " << this->decrypt_api_key() << std::endl;
		std::cout << "Input placeholding data (portfolio cost in usdt) : ";
		float sum = 100;
		//std::cin >> sum;
		return sum;
	}

	//Fetches a broker's portfolio percent. Will probably be deprecated soon due to no need
	float portfolio_percent_USDT()
	{
		return (this->portfolio_percent / 100) * portfolio_in_USDT();
	}

	//TODO Add error handling
	broker broker_fetch(int position) {
		broker result;
		std::string selectBrokerSQL = "SELECT id, name, surname, portfolio_percent, api_key FROM broker LIMIT 1 OFFSET ?;";

		sqlite3_stmt* stmt;
		if (sqlite3_prepare_v2(db, selectBrokerSQL.c_str(), -1, &stmt, NULL) == SQLITE_OK) {
			sqlite3_bind_int(stmt, 1, position);

			if (sqlite3_step(stmt) == SQLITE_ROW) {
				int brokerId = sqlite3_column_int(stmt, 0);
				const char* brokerName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
				const char* brokerSurname = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
				float portfolioPercent = static_cast<float>(sqlite3_column_double(stmt, 3));
				const char* apiKey = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));

				// Populate the result broker object
				result.id = brokerId;
				result.name = brokerName;
				result.surname = brokerSurname;
				result.portfolio_percent = portfolioPercent;
				result.api_key = apiKey;
			}
			else {
				std::cerr << "No broker found at position " << position << std::endl;
			}

			sqlite3_finalize(stmt);
		}
		else {
			std::cerr << "Failed to prepare SELECT statement for broker: " << sqlite3_errmsg(db) << std::endl;
		}

		return result;
	}

	//TODO Add error handling
	broker getBrokerById(int desiredBrokerId) {
		std::string query = "SELECT name, surname, portfolio_percent, api_key FROM broker WHERE id = " + std::to_string(desiredBrokerId);

		sqlite3_stmt* stmt;
		if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, 0) == SQLITE_OK) {
			if (sqlite3_step(stmt) == SQLITE_ROW) {
				const unsigned char* name = sqlite3_column_text(stmt, 0);
				const unsigned char* surname = sqlite3_column_text(stmt, 1);
				float portfolio_percent = sqlite3_column_double(stmt, 2);
				const unsigned char* api_key = sqlite3_column_text(stmt, 3);

				// Create a Broker object and set all the fields
				broker broker;
				broker.name = std::string(reinterpret_cast<const char*>(name));
				broker.surname = std::string(reinterpret_cast<const char*>(surname));
				broker.portfolio_percent = portfolio_percent;
				broker.api_key = std::string(reinterpret_cast<const char*>(api_key));
				broker.id = desiredBrokerId;

				// Finalize the statement
				sqlite3_finalize(stmt);

				return broker;
			}
			else {
				// No rows matched the query, meaning the broker with the given ID was not found
				// Handle this case, e.g., throw an exception or return a default Broker object
				sqlite3_finalize(stmt);
			}
		}
		else {
			// Handle the SQL query preparation error
			sqlite3_finalize(stmt);
			// You may want to throw an exception or return an error status in case of an error
		}

		// If no broker is found or an error occurs, you can return a default Broker object or throw an exception
	}

	//TODO Add error handling
	bool updateBrokerById(const broker& updatedBroker) {
		// Prepare the SQL UPDATE statement
		std::string query = "UPDATE broker SET name=?, surname=?, portfolio_percent=?, api_key=? WHERE id=?";
		sqlite3_stmt* stmt;

		if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, 0) == SQLITE_OK) {
			// Bind values to placeholders
			sqlite3_bind_text(stmt, 1, updatedBroker.name.c_str(), -1, SQLITE_STATIC);
			sqlite3_bind_text(stmt, 2, updatedBroker.surname.c_str(), -1, SQLITE_STATIC);
			sqlite3_bind_double(stmt, 3, updatedBroker.portfolio_percent);
			sqlite3_bind_text(stmt, 4, updatedBroker.api_key.c_str(), -1, SQLITE_STATIC);
			sqlite3_bind_int(stmt, 5, updatedBroker.id);

			// Execute the SQL statement
			int result = sqlite3_step(stmt);
			sqlite3_finalize(stmt);

			if (result == SQLITE_DONE) {
				// Update was successful
				return true;
			}
			else {
				// Handle the error if needed
				std::cerr << "Error updating broker: " << sqlite3_errmsg(db) << std::endl;
				return false;
			}
		}
		else {
			// Handle the SQL query preparation error
			sqlite3_finalize(stmt);
			std::cerr << "Error preparing update statement: " << sqlite3_errmsg(db) << std::endl;
			return false;
		}
	}
};

class client
{
public:
	int id, broker_id;
	std::string name, surname;
	float cut_percent;
	float portfolio_percent;

	//fetches client by his id
	//TODO Add error handling
	client getClientById(int desiredClientId) {
		std::string query = "SELECT name, surname, cut_percent, portfolio_percent, broker_id FROM client WHERE id = " + std::to_string(desiredClientId);

		sqlite3_stmt* stmt;
		if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, 0) == SQLITE_OK) {
			if (sqlite3_step(stmt) == SQLITE_ROW) {
				const unsigned char* name = sqlite3_column_text(stmt, 0);
				const unsigned char* surname = sqlite3_column_text(stmt, 1);
				float cut_percent = sqlite3_column_double(stmt, 2);
				float portfolio_percent = sqlite3_column_double(stmt, 3);
				int broker_id = sqlite3_column_int(stmt, 4);

				// Create a Client object and set all the fields
				client client;
				client.name = std::string(reinterpret_cast<const char*>(name));
				client.surname = std::string(reinterpret_cast<const char*>(surname));
				client.cut_percent = cut_percent;
				client.portfolio_percent = portfolio_percent;
				client.broker_id = broker_id;
				client.id = desiredClientId;

				// Finalize the statement
				sqlite3_finalize(stmt);

				return client;
			}
			else {
				// No rows matched the query, meaning the client with the given ID was not found
				// Handle this case, e.g., throw an exception or return a default Client object
				sqlite3_finalize(stmt);
			}
		}
		else {
			// Handle the SQL query preparation error
			sqlite3_finalize(stmt);
			// You may want to throw an exception or return an error status in case of an error
		}

		// If no client is found or an error occurs, you can return a default Client object or throw an exception
	}

	//inserts to the database everything about the client that was received via args
	//TODO Add error handling
	int insertClientData() {
		std::string insertClientSQL = "INSERT INTO client (name, surname, cut_percent, portfolio_percent, broker_id) "
			"VALUES (?, ?, ?, ?, ?);";

		sqlite3_stmt* stmt;
		if (sqlite3_prepare_v2(db, insertClientSQL.c_str(), -1, &stmt, NULL) == SQLITE_OK) {
			sqlite3_bind_text(stmt, 1, this->name.c_str(), -1, SQLITE_STATIC);
			sqlite3_bind_text(stmt, 2, this->surname.c_str(), -1, SQLITE_STATIC);
			sqlite3_bind_double(stmt, 3, this->cut_percent);
			sqlite3_bind_double(stmt, 4, this->portfolio_percent);
			sqlite3_bind_int(stmt, 5, this->broker_id);

			if (sqlite3_step(stmt) == SQLITE_DONE) {
				// Client inserted successfully, get the last inserted row ID
				int clientId = sqlite3_last_insert_rowid(db);
				std::cout << "Client inserted successfully. Client ID: " << clientId << std::endl;
				sqlite3_finalize(stmt);
				return clientId;
			}
			else {
				std::cerr << "Failed to insert client: " << sqlite3_errmsg(db) << std::endl;
			}

			sqlite3_finalize(stmt);
		}
		else {
			std::cerr << "Failed to prepare INSERT statement for client: " << sqlite3_errmsg(db) << std::endl;
		}

		// Return -1 to indicate failure
		return -1;
	}

	//fetches client from the database based on the position, is only used for outputs
	//TODO Add error handling
	//Server only, do not use
	client client_fetch(int position) {
		client result;
		std::string selectClientSQL = "SELECT id, name, surname, cut_percent, portfolio_percent, broker_id FROM client LIMIT 1 OFFSET ?;";

		sqlite3_stmt* stmt;
		if (sqlite3_prepare_v2(db, selectClientSQL.c_str(), -1, &stmt, NULL) == SQLITE_OK) {
			sqlite3_bind_int(stmt, 1, position);

			if (sqlite3_step(stmt) == SQLITE_ROW) {
				int clientId = sqlite3_column_int(stmt, 0);
				const char* clientName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
				const char* clientSurname = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
				float cutPercent = static_cast<float>(sqlite3_column_double(stmt, 3));
				float portfolioPercent = static_cast<float>(sqlite3_column_double(stmt, 4));
				int brokerId = sqlite3_column_int(stmt, 5);
				// Populate the result client object
				result.id = clientId;
				result.name = clientName;
				result.surname = clientSurname;
				result.cut_percent = cutPercent;
				result.portfolio_percent = portfolioPercent;
				result.broker_id = brokerId;
			}
			else {
				std::cerr << "No client found at position " << position << std::endl;
			}

			sqlite3_finalize(stmt);
		}
		else {
			std::cerr << "Failed to prepare SELECT statement for client: " << sqlite3_errmsg(db) << std::endl;
		}

		return result;
	}

	//outputs all clients to the console
	//Server only, do not use
	void coutallclients()
	{
		client out;
		for (int i = 0; i < countRows("client"); i++)
		{
			out = client_fetch(i);
			std::cout << "\tClient ID: " << out.id << ", Name: " << out.name << " " << out.surname << ", broker ID : " << out.broker_id << std::endl;
		}
	}

	//Uses the data received from client app to fill the database
	//Returns inserted client ID (from db, id is autoincremented, therefore not present in programm)
	int add_client_API(int& errorcode, std::string& errormsg, std::string name, std::string surname, float cut, int brokerid)
	{
		if (cut > 100 || cut < 0)
		{
			errorcode = 400;
			errormsg = "Impossible cut input";
			return 0;
		}
		if (!does_broker_exist(errorcode, errormsg, brokerid))
		{
			if (errorcode != 0)
			{
				return 0;
			}
			else
			{
				errorcode = 400;
				errormsg = "No broker with the id " + std::to_string(brokerid) + " could be found";
				return 0;
			}
		}
		this->name = name;
		this->surname = surname;
		this->cut_percent = cut;
		this->broker_id = brokerid;
		this->portfolio_percent = 0;
		errorcode = 0;
		return this->insertClientData();
	}

	//Creates a client object, asks user for needed input, passes the data to the insert function
	//Server only, do not use
	int addClient()
	{
		client temp;
		std::cout << "Input name and surname : "; std::cin >> temp.name >> temp.surname;

		std::cout << "Input broker's cut (%) : "; std::cin >> temp.cut_percent;
		bool cutIsPossible = 0;
		while (!cutIsPossible)
		{
			if (temp.cut_percent > 100 || temp.cut_percent < 0)
			{
				std::cout << "Please enter a valid percentage : "; std::cin >> temp.cut_percent;
			}
			else { cutIsPossible = 1; break; }
		}
		temp.portfolio_percent = 0;
		std::cout << std::endl;
		cutIsPossible = 0;
		while (!cutIsPossible) {
			broker a;
			a.coutallbrokers();
			std::cout << "Choose your broker : "; std::cin >> temp.broker_id;
			for (int i = 0; i < countRows("broker"); i++)
			{
				a = a.broker_fetch(i);
				if (temp.broker_id = a.id)
				{
					cutIsPossible = 1;
				}
			}
			if (!cutIsPossible)
			{
				std::cout << "Id not valid" << std::endl;
			}
		}
		return temp.insertClientData();
	}

	//Updates client in the database. Uses client.id to identify in the database, overwrites all the data according to the new object
	bool updateClientById(int errorcode, std::string errormsg) {
		std::string query = "UPDATE client SET name=?, surname=?, cut_percent=?, portfolio_percent=? WHERE id=?";
		sqlite3_stmt* stmt;

		if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, 0) == SQLITE_OK) {
			sqlite3_bind_text(stmt, 1, this->name.c_str(), -1, SQLITE_STATIC);
			sqlite3_bind_text(stmt, 2, this->surname.c_str(), -1, SQLITE_STATIC);
			sqlite3_bind_double(stmt, 3, this->cut_percent);
			sqlite3_bind_double(stmt, 4, this->portfolio_percent);
			sqlite3_bind_int(stmt, 5, this->id);

			int result = sqlite3_step(stmt);
			sqlite3_finalize(stmt);

			if (result == SQLITE_DONE) {
				return true;
			}
			else {
				std::cerr << "Error updating client: " << sqlite3_errmsg(db) << std::endl;
				errorcode = 502; errormsg = "Error updating client: " + std::string(sqlite3_errmsg(db));
				return false;
			}
		}
		else {
			sqlite3_finalize(stmt);
			std::cerr << "Error preparing update statement: " << sqlite3_errmsg(db) << std::endl;
			errorcode = 502; errormsg = "Error preparing update statement: " + std::string(sqlite3_errmsg(db));
			return false;
		}
	}

	//returns a vector of client object where brokerid is equal to the one received as args
	//is used when calculating percent changes and updating users after transactions
	//TODO Add error handling
	std::vector<client> getClientsByBrokerId(int desiredBrokerId) {
		std::vector<client> clients;

		std::string query = "SELECT id, name, surname, cut_percent, portfolio_percent FROM client WHERE broker_id = " + std::to_string(desiredBrokerId);

		sqlite3_stmt* stmt;
		if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, 0) == SQLITE_OK) {
			while (sqlite3_step(stmt) == SQLITE_ROW) {
				int id = sqlite3_column_int(stmt, 0);
				const unsigned char* name = sqlite3_column_text(stmt, 1);
				const unsigned char* surname = sqlite3_column_text(stmt, 2);
				float cut_percent = sqlite3_column_double(stmt, 3);
				float portfolio_percent = sqlite3_column_double(stmt, 4);

				client client;
				client.id = id;
				client.broker_id = desiredBrokerId;
				client.name = std::string(reinterpret_cast<const char*>(name));
				client.surname = std::string(reinterpret_cast<const char*>(surname));
				client.cut_percent = cut_percent;
				client.portfolio_percent = portfolio_percent;

				clients.push_back(client);
			}

			// Finalize the statement
			sqlite3_finalize(stmt);
		}
		else {
			// Handle the SQL query preparation error
			sqlite3_finalize(stmt);
			// You may want to throw an exception or return an error status in case of an error
		}

		// Return the vector of clients
		return clients;
	}
};

class transaction
{
public:
	int id, client_id, broker_id;
	bool transaction_type, transaction_initiator;
	float balance_before_transaction, balance_after_transaction, trasactionsum;

	//TODO Add error handling
	//inserts the data to the database
	void insertTransactionData() {
		std::string insertTransactionSQL = "INSERT INTO transactions (broker_id, client_id, transaction_type, transaction_initiator, "
			"balance_before_transaction, balance_after_transaction, transaction_sum) "
			"VALUES (?, ?, ?, ?, ?, ?, ?);";

		sqlite3_stmt* stmt;
		if (sqlite3_prepare_v2(db, insertTransactionSQL.c_str(), -1, &stmt, NULL) == SQLITE_OK) {
			sqlite3_bind_int(stmt, 1, this->broker_id);
			sqlite3_bind_int(stmt, 2, this->client_id);
			sqlite3_bind_int(stmt, 3, this->transaction_type);
			sqlite3_bind_int(stmt, 4, this->transaction_initiator);
			sqlite3_bind_double(stmt, 5, this->balance_before_transaction);
			sqlite3_bind_double(stmt, 6, this->balance_after_transaction);
			sqlite3_bind_double(stmt, 7, this->trasactionsum);

			if (sqlite3_step(stmt) == SQLITE_DONE) {
				std::cout << "Transaction inserted successfully." << std::endl;
			}
			else {
				std::cerr << "Failed to insert transaction: " << sqlite3_errmsg(db) << std::endl;
			}

			sqlite3_finalize(stmt);
		}
		else {
			std::cerr << "Failed to prepare INSERT statement for transaction: " << sqlite3_errmsg(db) << std::endl;
		}
	}

	//Server only, do not use
	void createtransaction(int& errorcode, std::string& errormsg)
	{
		transaction temp;
		bool idExists = 0;
		while (!idExists)
		{
			std::cout << "Input your transaction type (w for withdrawal, d for deposit) : "; char typet; std::cin >> typet;
			if (typet == 'd')
			{
				temp.transaction_type = 1;
				idExists = 1;
			}
			else if (typet == 'w')
			{
				temp.transaction_type = 0;
				idExists = 1;
			}
			else
			{
				std::cout << "Bad input" << std::endl;
			}
		}

		idExists = 0;
		while (!idExists)
		{
			std::cout << "Input your transaction initiator (b for broker, c for client) : "; char typet; std::cin >> typet;
			if (typet == 'b')
			{
				temp.transaction_initiator = 1;
				idExists = 1;
			}
			else if (typet == 'c')
			{
				temp.transaction_initiator = 0;
				idExists = 1;
			}
			else
			{
				std::cout << "Bad input" << std::endl;
			}
		}
		broker a;
		std::cout << "Choose your broker from the list. " << std::endl;

		broker* inputbroker = nullptr; int id; idExists = 0;

		while (!idExists)
		{
			a.coutallbrokers();
			std::cout << "Input ID : "; std::cin >> id;
			for (int i = 0; i < countRows("broker"); i++)
			{
				a = a.broker_fetch(i);
				if (a.id == id)
				{
					idExists = 1;
					temp.broker_id = a.id;
				}
			}
			if (!idExists)
			{
				std::cout << "Please enter a valid id" << std::endl;
			}
		}

		if (!temp.transaction_initiator)
		{
			client b;
			std::cout << "Choose your client from the list. " << std::endl;

			client* inputclient = nullptr;

			while (!idExists)
			{
				b.coutallclients();
				std::cout << "Input ID : "; std::cin >> id;  idExists = 0;
				for (int i = 0; i < countRows("client"); i++)
				{
					b = b.getClientById(i);
					if (b.id == id)
					{
						idExists = 1;
						temp.id = b.id;
					}
				}
				if (!idExists)
				{
					std::cout << "Please enter a valid id" << std::endl;
				}
			}

			bool sum_is_possible = 0; float clientsum; float broker_sum = 0;
			temp.balance_before_transaction = a.portfolio_in_USDT();
			while (!sum_is_possible) {
				std::cout << "Input transaction sum : "; std::cin >> temp.trasactionsum;

				if (temp.transaction_type)
				{
					broker_sum = (b.cut_percent / 100) * temp.trasactionsum;
					clientsum = temp.trasactionsum - broker_sum + (b.portfolio_percent / 100) * temp.balance_before_transaction;
					temp.balance_after_transaction = temp.balance_before_transaction + temp.trasactionsum;
					sum_is_possible = 1;
				}
				else
				{
					clientsum = (b.portfolio_percent / 100) * temp.balance_before_transaction - temp.trasactionsum;
					if (clientsum < 0)
					{
						std::cout << "You do not have enough funds to withdraw, your current percent is worth " << (a.portfolio_percent / 100) * temp.balance_before_transaction << "$" << std::endl;
					}
					else { temp.balance_after_transaction = temp.balance_before_transaction - temp.trasactionsum; sum_is_possible = 1; }
				}
			}

			b.portfolio_percent = (clientsum / temp.balance_after_transaction) * 100;
			float tempclientsum;
			std::vector<client> vec = b.getClientsByBrokerId(a.id);
			for (int i = 0; i < vec.size(); i++)
			{
				if (vec[i].id != b.id) {
					tempclientsum = (vec[i].portfolio_percent / 100) * temp.balance_before_transaction;
					vec[i].portfolio_percent = (tempclientsum / temp.balance_after_transaction) * 100;
					if (vec[i].updateClientById(errorcode, errormsg) == 0) { errormsg = errormsg + "\nTransaction could not be completed"; }
				}
			}
			broker_sum = broker_sum + (a.portfolio_percent / 100) * temp.balance_before_transaction;
			a.portfolio_percent = (broker_sum / temp.balance_after_transaction) * 100;
			if (b.updateClientById(errorcode, errormsg) == 0) { errormsg = errormsg + "\nTransaction could not be completed"; }
			if (a.updateBrokerById(a) == 0) { errormsg = errormsg + "\nTransaction could not be completed"; }
			temp.insertTransactionData();
		}
		else
		{
			{
				bool sum_is_possible = 0; float broker_sum;
				while (!sum_is_possible) {
					std::cout << "Input transaction sum : "; std::cin >> temp.trasactionsum;

					temp.balance_before_transaction = a.portfolio_in_USDT();
					if (temp.transaction_type)
					{
						broker_sum = temp.trasactionsum + (a.portfolio_percent / 100) * temp.balance_before_transaction;
						temp.balance_after_transaction = temp.balance_before_transaction + temp.trasactionsum;
						sum_is_possible = 1;
					}
					else
					{
						broker_sum = (a.portfolio_percent / 100) * temp.balance_before_transaction - temp.trasactionsum;
						if (broker_sum < 0)
						{
							std::cout << "You do not have enough funds to withdraw, your current percent is worth " << (a.portfolio_percent / 100) * temp.balance_before_transaction << "$" << std::endl;
						}
						else { temp.balance_after_transaction = temp.balance_before_transaction - temp.trasactionsum; sum_is_possible = 1; }
					}
				}
				float tempclientsum;
				client b;
				std::vector<client> vec = b.getClientsByBrokerId(a.id);
				for (int i = 0; i < vec.size(); i++)
				{
					tempclientsum = (vec[i].portfolio_percent / 100) * temp.balance_before_transaction;
					vec[i].portfolio_percent = (tempclientsum / temp.balance_after_transaction) * 100;
					if (vec[i].updateClientById(errorcode, errormsg) == 0) { errormsg = errormsg + "\nTransaction could not be completed"; }
				}
				a.portfolio_percent = (broker_sum / temp.balance_after_transaction) * 100;
			}
			if (a.updateBrokerById(a) == 0) { errormsg = errormsg + "\nTransaction could not be completed"; }
			temp.client_id = NULL;
			temp.insertTransactionData();
		}
	}

	//TODO Add error handling
	transaction transaction_fetch(int position) {
		transaction result;
		std::string selectTransactionSQL = "SELECT id, transaction_type, transaction_initiator, balance_before_transaction, "
			"balance_after_transaction, transaction_sum FROM transactions LIMIT 1 OFFSET ?;";

		sqlite3_stmt* stmt;
		if (sqlite3_prepare_v2(db, selectTransactionSQL.c_str(), -1, &stmt, NULL) == SQLITE_OK) {
			sqlite3_bind_int(stmt, 1, position);

			if (sqlite3_step(stmt) == SQLITE_ROW) {
				int transactionId = sqlite3_column_int(stmt, 0);
				int transactionType = sqlite3_column_int(stmt, 1);
				int transactionInitiator = sqlite3_column_int(stmt, 2);
				float balanceBeforeTransaction = static_cast<float>(sqlite3_column_double(stmt, 3));
				float balanceAfterTransaction = static_cast<float>(sqlite3_column_double(stmt, 4));
				float transactionSum = static_cast<float>(sqlite3_column_double(stmt, 5));

				// Populate the result transaction object
				result.id = transactionId;
				result.transaction_type = transactionType;
				result.transaction_initiator = transactionInitiator;
				result.balance_before_transaction = balanceBeforeTransaction;
				result.balance_after_transaction = balanceAfterTransaction;
				result.trasactionsum = transactionSum;
			}
			else {
				std::cerr << "No transaction found at position " << position << std::endl;
			}

			sqlite3_finalize(stmt);
		}
		else {
			std::cerr << "Failed to prepare SELECT statement for transaction: " << sqlite3_errmsg(db) << std::endl;
		}

		return result;
	}

	//Server only, do not use
	void createtransactionclient(int& errorcode, std::string& errormsg, client b)
	{
		this->transaction_initiator = 0;
		bool idExists = 0;
		while (!idExists)
		{
			std::cout << "Input your transaction type (w for withdrawal, d for deposit) : "; char typet; std::cin >> typet;
			if (typet == 'd')
			{
				this->transaction_type = 1;
				idExists = 1;
			}
			else if (typet == 'w')
			{
				this->transaction_type = 0;
				idExists = 1;
			}
			else
			{
				std::cout << "Bad input" << std::endl;
			}
		}
		idExists = 0;
		this->client_id = b.id;
		this->broker_id = b.broker_id;
		broker a;
		a = a.getBrokerById(this->broker_id);
		b = b.getClientById(this->client_id);
		idExists = 1;

		bool sum_is_possible = 0; float clientsum; float broker_sum = 0;
		this->balance_before_transaction = a.portfolio_in_USDT();
		while (!sum_is_possible) {
			std::cout << "Input transaction sum : "; std::cin >> this->trasactionsum;

			if (this->transaction_type)
			{
				broker_sum = (b.cut_percent / 100) * this->trasactionsum;
				clientsum = this->trasactionsum - broker_sum + (b.portfolio_percent / 100) * this->balance_before_transaction;
				this->balance_after_transaction = this->balance_before_transaction + this->trasactionsum;
				sum_is_possible = 1;
			}
			else
			{
				clientsum = (b.portfolio_percent / 100) * this->balance_before_transaction - this->trasactionsum;
				if (clientsum < 0)
				{
					std::cout << "You do not have enough funds to withdraw, your current percent is worth " << (a.portfolio_percent / 100) * this->balance_before_transaction << "$" << std::endl;
				}
				else { this->balance_after_transaction = this->balance_before_transaction - this->trasactionsum; sum_is_possible = 1; }
			}
		}

		b.portfolio_percent = (clientsum / this->balance_after_transaction) * 100;
		float tempclientsum;
		std::vector<client> vec = b.getClientsByBrokerId(a.id);
		for (int i = 0; i < vec.size(); i++)
		{
			if (vec[i].id != b.id) {
				tempclientsum = (vec[i].portfolio_percent / 100) * this->balance_before_transaction;
				vec[i].portfolio_percent = (tempclientsum / this->balance_after_transaction) * 100;
				if (vec[i].updateClientById(errorcode, errormsg) == 0) { errormsg = errormsg + "\nTransaction could not be completed"; }
			}
		}
		broker_sum = broker_sum + (a.portfolio_percent / 100) * this->balance_before_transaction;
		a.portfolio_percent = (broker_sum / this->balance_after_transaction) * 100;
		if (b.updateClientById(errorcode, errormsg) == 0) { errormsg = errormsg + "\nTransaction could not be completed"; }
		if (a.updateBrokerById(a) == 0) { errormsg = errormsg + "\nTransaction could not be completed"; }
		this->insertTransactionData();
	}

	//needs transaction type and transaction sum specified within object
	int create_transaction_client_api(int& errorcode, std::string& errormsg, client b)
	{
		this->transaction_initiator = 0;
		bool idExists = 0;
		this->client_id = b.id;
		this->broker_id = b.broker_id;
		broker a;
		a = a.getBrokerById(this->broker_id);
		b = b.getClientById(this->client_id);
		idExists = 1;
		bool sum_is_possible = 0; float clientsum; float broker_sum = 0;
		this->balance_before_transaction = a.portfolio_in_USDT();

		if (this->transaction_type)
		{
			broker_sum = (b.cut_percent / 100) * this->trasactionsum;
			clientsum = this->trasactionsum - broker_sum + (b.portfolio_percent / 100) * this->balance_before_transaction;
			this->balance_after_transaction = this->balance_before_transaction + this->trasactionsum;
			sum_is_possible = 1;
		}
		else
		{
			clientsum = (b.portfolio_percent / 100) * this->balance_before_transaction - this->trasactionsum;
			if (clientsum < 0)
			{
				errorcode = 400; errormsg = "Not enough funds for withdrawal"; return 400;
			}
			else { this->balance_after_transaction = this->balance_before_transaction - this->trasactionsum; sum_is_possible = 1; }
		}

		b.portfolio_percent = (clientsum / this->balance_after_transaction) * 100;
		float tempclientsum;

		std::vector<client> vec = b.getClientsByBrokerId(a.id);
		for (int i = 0; i < vec.size(); i++)
		{
			if (vec[i].id != b.id) {
				tempclientsum = (vec[i].portfolio_percent / 100) * this->balance_before_transaction;
				vec[i].portfolio_percent = (tempclientsum / this->balance_after_transaction) * 100;
				if (vec[i].updateClientById(errorcode, errormsg) == 0) { errormsg = errormsg + "\nTransaction could not be completed"; }
			}
		}
		broker_sum = broker_sum + (a.portfolio_percent / 100) * this->balance_before_transaction;
		a.portfolio_percent = (broker_sum / this->balance_after_transaction) * 100;
		if (b.updateClientById(errorcode, errormsg) == 0) { errormsg = errormsg + "\nTransaction could not be completed"; }
		if (a.updateBrokerById(a) == 0) { errormsg = errormsg + "\nTransaction could not be completed"; }
		this->insertTransactionData();

		return 0;
	}

	//needs transaction type and transaction sum specified within object
	int create_transaction_broker_api(int& errorcode, std::string& errormsg, broker a)
	{
		Timer timer;
		this->transaction_initiator = 1;
		this->broker_id = a.id;
		{
			bool sum_is_possible = 0; float broker_sum;
			while (!sum_is_possible) {
				this->balance_before_transaction = a.portfolio_in_USDT();
				if (this->transaction_type)
				{
					broker_sum = this->trasactionsum + (a.portfolio_percent / 100) * this->balance_before_transaction;
					this->balance_after_transaction = this->balance_before_transaction + this->trasactionsum;
					sum_is_possible = 1;
				}
				else
				{
					broker_sum = (a.portfolio_percent / 100) * this->balance_before_transaction - this->trasactionsum;
					if (broker_sum < 0)
					{
						errorcode = 400; errormsg = "Not enough funds for withdrawal"; return 400;
					}
					else { this->balance_after_transaction = this->balance_before_transaction - this->trasactionsum; sum_is_possible = 1; }
				}
			}
			float tempclientsum;
			client b;
			timer.start();
			std::vector<client> vec = b.getClientsByBrokerId(a.id);
			for (int i = 0; i < vec.size(); i++)
			{
				tempclientsum = (vec[i].portfolio_percent / 100) * this->balance_before_transaction;
				vec[i].portfolio_percent = (tempclientsum / this->balance_after_transaction) * 100;
				if (vec[i].updateClientById(errorcode, errormsg) == 0) { errormsg = errormsg + "\nTransaction could not be completed"; }
			}
			a.portfolio_percent = (broker_sum / this->balance_after_transaction) * 100;
		}
		if (a.updateBrokerById(a) == 0) { errormsg = errormsg + "\nTransaction could not be completed"; }
		this->client_id = NULL;
		this->insertTransactionData();
		timer.end();
		return 0;
	}

	//Server only, do not use
	void createtransactionbroker(int& errorcode, std::string& errormsg, broker a)
	{
		transaction temp;
		temp.transaction_initiator = 1;
		bool idExists = 0;
		while (!idExists)
		{
			std::cout << "Input your transaction type (w for withdrawal, d for deposit) : "; char typet; std::cin >> typet;
			if (typet == 'd')
			{
				temp.transaction_type = 1;
				idExists = 1;
			}
			else if (typet == 'w')
			{
				temp.transaction_type = 0;
				idExists = 1;
			}
			else
			{
				std::cout << "Bad input" << std::endl;
			}
		}
		idExists = 0;
		temp.broker_id = a.id;
		{
			bool sum_is_possible = 0; float broker_sum;
			while (!sum_is_possible) {
				std::cout << "Input transaction sum : "; std::cin >> temp.trasactionsum;

				temp.balance_before_transaction = a.portfolio_in_USDT();
				if (temp.transaction_type)
				{
					broker_sum = temp.trasactionsum + (a.portfolio_percent / 100) * temp.balance_before_transaction;
					temp.balance_after_transaction = temp.balance_before_transaction + temp.trasactionsum;
					sum_is_possible = 1;
				}
				else
				{
					broker_sum = (a.portfolio_percent / 100) * temp.balance_before_transaction - temp.trasactionsum;
					if (broker_sum < 0)
					{
						std::cout << "You do not have enough funds to withdraw, your current percent is worth " << (a.portfolio_percent / 100) * temp.balance_before_transaction << "$" << std::endl;
					}
					else { temp.balance_after_transaction = temp.balance_before_transaction - temp.trasactionsum; sum_is_possible = 1; }
				}
			}
			float tempclientsum;
			client b;
			std::vector<client> vec = b.getClientsByBrokerId(a.id);
			for (int i = 0; i < vec.size(); i++)
			{
				tempclientsum = (vec[i].portfolio_percent / 100) * temp.balance_before_transaction;
				vec[i].portfolio_percent = (tempclientsum / temp.balance_after_transaction) * 100;
				if (vec[i].updateClientById(errorcode, errormsg) == 0) { errormsg = errormsg + "\nTransaction could not be completed"; }
			}
			a.portfolio_percent = (broker_sum / temp.balance_after_transaction) * 100;
		}
		if (a.updateBrokerById(a) == 0) { errormsg = errormsg + "\nTransaction could not be completed"; }
		temp.client_id = NULL;
		temp.insertTransactionData();
	}
};

class account
{
public:
	int account_type;
	std::string username;
	std::string password;
	std::string email;
	int phonenumber;
	int account_id;
	int account_reference_id;
public:

	//TODO Add error handling
	bool addaccountdata(account temp) {
		std::string insertAccountSQL = "INSERT INTO accounts (account_type, account_reference, email, phone_number, login, password) "
			"VALUES (?, ?, ?, ?, ?, ?);";

		sqlite3_stmt* stmt;
		if (sqlite3_prepare_v2(db, insertAccountSQL.c_str(), -1, &stmt, NULL) == SQLITE_OK) {
			sqlite3_bind_int(stmt, 1, temp.account_type);
			sqlite3_bind_int(stmt, 2, temp.account_reference_id);
			sqlite3_bind_text(stmt, 3, temp.email.c_str(), -1, SQLITE_STATIC);
			sqlite3_bind_int(stmt, 4, temp.phonenumber);
			sqlite3_bind_text(stmt, 5, temp.username.c_str(), -1, SQLITE_STATIC);
			sqlite3_bind_text(stmt, 6, temp.password.c_str(), -1, SQLITE_STATIC);

			if (sqlite3_step(stmt) == SQLITE_DONE) {
				std::cout << "Account inserted successfully." << std::endl;
				sqlite3_finalize(stmt);
				return true;
			}
			else {
				std::cerr << "Failed to insert account: " << sqlite3_errmsg(db) << std::endl;
				sqlite3_finalize(stmt);
				return false;
			}
		}
		else {
			std::cerr << "Failed to prepare INSERT statement: " << sqlite3_errmsg(db) << std::endl;
			return false;
		}
	}

	// Function to fetch an account by account_id and populate the current instance (this)
	void fetchAccountById(int account_id) {
		// Open the SQLite database
		sqlite3* db;
		if (sqlite3_open("your_database.db", &db) == SQLITE_OK) {
			std::string query = "SELECT * FROM accounts WHERE account_id = " + std::to_string(account_id);

			// Execute the query
			sqlite3_stmt* stmt;
			if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, 0) == SQLITE_OK) {
				if (sqlite3_step(stmt) == SQLITE_ROW) {
					// Fill the current instance with data from the query
					this->account_id = sqlite3_column_int(stmt, 0);
					this->account_type = sqlite3_column_int(stmt, 1);
					// Set other fields similarly

					// Finalize the statement and close the database
					sqlite3_finalize(stmt);
					sqlite3_close(db);
				}
				else {
					// No results found
					sqlite3_finalize(stmt);
					sqlite3_close(db);
					this->account_id = -1; // Or set to another appropriate value
				}
			}
			else {
				std::cerr << "Error executing query: " << sqlite3_errmsg(db) << std::endl;
				this->account_id = -1; // Or set to another appropriate value
			}
		}
		else {
			std::cerr << "Failed to open the database." << std::endl;
			this->account_id = -1; // Or set to another appropriate value
		}
	}


	//TODO Add error handling
	//Server only, do not use
	bool register_account(int& errorcode, std::string& errormsg)
	{
		account temp; bool verify = false;
		while (!verify) {
			std::cout << "Choose the type of account to register \n1.Superuser\n2.Client\n3.Broker\nInput : "; std::cin >> temp.account_type;
			logger->trace("Type of account input asked, input received : {}", temp.account_type);
			if (temp.account_type == 2 || temp.account_type == 3)
			{
				verify = true;
				logger->trace("Input verified as client/broker");
			}
			else if (temp.account_type == 1)
			{
				logger->trace("Type of account input received as superuser, proceeding to sudo password check");
				if (sudo_registration_verification())
				{
					verify = true;
					logger->trace("SU password confirmed");
				}
			}
		}
		verify = false;
		while (!verify) {
			logger->trace("Username prompted");
			std::cout << "Input username : "; std::cin.ignore(); std::getline(std::cin, temp.username);
			if (!doesUsernameExist(errorcode, errormsg, temp.username))
			{
				logger->trace("Username {} confirmed as non-existent in the database", temp.username);
				verify = true;
			}
			else
			{
				logger->trace("Username {} already taken, asking for re-input", temp.username);
				std::cout << "Username already taken" << std::endl;
			}
		}
		logger->trace("Password prompted");
		std::cout << "Input password : "; std::getline(std::cin, temp.password);
		while (!isStrongPassword(temp.password)) {
			std::cout << "Password is not strong enough. Use at least 1 uppercase, 1 lowercase and 1 number with a length of at least 8 characters\nInput password : "; std::getline(std::cin, temp.password);
		}
		std::string salt = generateSalt(saltlen);
		temp.password = hashString(temp.password + salt);
		logger->trace("Email prompted");
		std::cout << "Input email : "; std::cin >> temp.email;
		logger->trace("Phone number prompted");
		std::cout << "Input phone number : "; std::cin >> temp.phonenumber;
		if (temp.account_type == 2)
		{
			client clienttemp;
			if (temp.addaccountdata(temp) && insertSalt(temp.username, salt))
			{
				temp = temp.fetchAccountByUsername(temp.username);
				temp.account_reference_id = clienttemp.addClient();
				return 1;
			}
			else { return 0; }
		}
		else if (temp.account_type == 3)
		{
			broker brokertemp;
			if (temp.addaccountdata(temp) && insertSalt(temp.username, salt))
			{
				temp = temp.fetchAccountByUsername(temp.username);
				temp.account_reference_id = brokertemp.addBroker(errorcode, errormsg);
				temp.update();
				return 1;
			}
			else { return 0; }
		}
		else {
			logger->info("Superuser {} created", temp.username);
			temp.account_reference_id = NULL;
			return temp.addaccountdata(temp) && insertSalt(temp.username, salt);
		}
	}

	//Server only, do not use
	bool typeclient(account currentaccount)
	{
		client currentuser; currentuser = currentuser.getClientById(currentaccount.account_reference_id); std::string errormsg;
		transaction temp;
		int choice; int errorcode;
		bool exitAccount = false;

		while (!exitAccount) {
			std::cout << "Menu Options:" << std::endl;
			std::cout << "1. Create Transaction" << std::endl;
			std::cout << "2. Exit Application" << std::endl;
			std::cout << "3. Exit Account" << std::endl;
			std::cout << "Enter your choice (1-3): ";

			std::cin >> choice;

			switch (choice) {
			case 1:
				temp.createtransactionclient(errorcode, errormsg, currentuser);
				break;
			case 2:
				std::cout << "Exiting the application." << std::endl;
				return 1;  // Exit the application
			case 3:
				std::cout << "Exiting the account." << std::endl;
				exitAccount = true;
				break;
			default:
				std::cout << "Invalid choice. Please select a valid option (1-3)." << std::endl;
				break;
			}
		}
		return 0;
	}

	//Server only, do not use
	bool typebroker(account currentaccount)
	{
		currentaccount = currentaccount.fetchAccountByUsername(currentaccount.username);
		broker currentuser; currentuser = currentuser.getBrokerById(currentaccount.account_reference_id);
		transaction temp;
		int choice, errorcode;
		std::string errormsg;
		bool exitAccount = false;
		Timer timer;

		while (!exitAccount) {
			std::cout << "Menu Options:" << std::endl;
			std::cout << "1. Create Transaction" << std::endl;
			std::cout << "2. Exit Application" << std::endl;
			std::cout << "3. Exit Account" << std::endl;
			std::cout << "Enter your choice (1-4): ";

			std::cin >> choice;

			switch (choice) {
			case 1:
				temp.createtransactionbroker(errorcode, errormsg, currentuser);
				break;
			case 2:
				std::cout << "Exiting the application." << std::endl;
				return 1;  // Exit the application
			case 3:
				std::cout << "Exiting the account." << std::endl;
				exitAccount = true;
				break;
			case 4:
				temp.transaction_type = 1;
				temp.trasactionsum = 50;

				temp.create_transaction_broker_api(errorcode, errormsg, currentuser);

			default:
				std::cout << "Invalid choice. Please select a valid option (1-3)." << std::endl;
				break;
			}
		}
		return 0;
	}

	//Server only, do not use
	bool superuser(account currentuser)
	{
		logger->info("Superuser {} signed in", currentuser.username);
		std::vector<transaction> transactions;
		broker temp; transaction transacc; client tempp; account acc;
		int choice, errorcode = 0;
		std::string errormsg;

		while (true) {
			std::cout << "Menu:" << std::endl;
			std::cout << "1. Input Broker" << std::endl;
			std::cout << "2. Input Client" << std::endl;
			std::cout << "3. Input Transaction" << std::endl;
			std::cout << "4. Output Brokers" << std::endl;
			std::cout << "5. Output Clients" << std::endl;
			std::cout << "6. Output Transactions" << std::endl;
			std::cout << "7. Exit" << std::endl;
			std::cout << "Enter your choice: ";

			if (!(std::cin >> choice)) {
				std::cerr << "Invalid input. Please enter a valid number." << std::endl;
				std::cin.clear();
				std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
				continue;
			}

			switch (choice) {
			case 1:
				// Input Broker logic here
				choice = temp.addBroker(errorcode, errormsg);
				logger->info("Broker with id {} was created by superuser {}", choice, currentuser.username);
				break;
			case 2:
				// Input Client logic here
				choice = tempp.addClient();
				logger->info("Client with id {} was created by superuser {}", choice, currentuser.username);
				break;
			case 3:
				// Input Transaction logic here
				transacc.createtransaction(errorcode, errormsg);
				break;
			case 4:
				// Output list of brokers
				temp.coutallbrokers();
				break;
			case 5:
				// Output list of clients
				tempp.coutallclients();
				break;
			case 6:
				// Output list of transactions
				//outputTransactions(transactions, brokers);
				break;
			case 7:
				// Exit the program
				std::cout << "Exiting the program." << std::endl;
				return 0;
			case 8:
				std::cout << "Account registration sequence starting" << std::endl << std::endl;

				acc.register_account(errorcode, errormsg);
			default:
				std::cout << "Invalid choice. Please enter a valid option (1-7)." << std::endl;
			}
		}
	}

	//TODO Add error handling
	int CheckPass() {
		// SQL statement to select an account based on login
		const char* selectSQL = "SELECT account_type, password FROM accounts WHERE login = ? LIMIT 1;";
		sqlite3_stmt* stmt;

		// Prepare the SQL statement
		if (sqlite3_prepare_v2(db, selectSQL, -1, &stmt, nullptr) == SQLITE_OK) {
			// Bind the login parameter to the statement
			sqlite3_bind_text(stmt, 1, this->username.c_str(), -1, SQLITE_STATIC);

			// Execute the statement
			if (sqlite3_step(stmt) == SQLITE_ROW) {
				// Account with the given login exists
				// Retrieve account_type and storedPassword
				int accountType = sqlite3_column_int(stmt, 0);
				const char* storedPassword = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));

				// Now, check if the provided password matches
				const char* selectSaltSQL = "SELECT salt FROM salt WHERE username = ?;";
				sqlite3_stmt* stmt2;

				if (sqlite3_prepare_v2(keybase, selectSaltSQL, -1, &stmt2, 0) == SQLITE_OK) {
					// Bind the username parameter
					sqlite3_bind_text(stmt2, 1, this->username.c_str(), -1, SQLITE_STATIC);

					// Execute the query
					if (sqlite3_step(stmt2) == SQLITE_ROW) {
						// Retrieve the salt value from the result
						const char* salt = reinterpret_cast<const char*>(sqlite3_column_text(stmt2, 0));

						// Now, call the checkmatch function with the correct arguments
						if (checkmatch(this->password, storedPassword, salt)) {
							// Password matches, return the account type
							sqlite3_finalize(stmt);
							sqlite3_finalize(stmt2);
							return accountType;
						}
					}

					// Finalize the statement
					sqlite3_finalize(stmt2);
				}
			}

			// Finalize the statement
			sqlite3_finalize(stmt);
		}

		// Account does not exist or password does not match
		return 0;
	}

private:

	//TODO Add error handling
	account fetchAccountByUsername(const std::string& username) {
		account aaccount;

		// SQL statement to select an account based on username
		const char* selectSQL = "SELECT * FROM accounts WHERE login = ? LIMIT 1;";

		sqlite3_stmt* stmt;

		// Prepare the SQL statement
		if (sqlite3_prepare_v2(db, selectSQL, -1, &stmt, nullptr) == SQLITE_OK) {
			// Bind the username parameter to the statement
			sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);

			// Execute the statement
			if (sqlite3_step(stmt) == SQLITE_ROW) {
				// Account with the given username exists
				aaccount.account_id = sqlite3_column_int(stmt, 0);
				aaccount.account_type = sqlite3_column_int(stmt, 1);
				aaccount.username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
				aaccount.password = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
				aaccount.email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
				aaccount.phonenumber = sqlite3_column_int(stmt, 4);
				aaccount.account_reference_id = sqlite3_column_int(stmt, 2); // Populate account_reference_id
			}

			// Finalize the statement
			sqlite3_finalize(stmt);
		}

		return aaccount;
	}
public:
	//requires Username and Password predefined in account object
	bool API_auth(int& errorcode, std::string& errormsg, std::string& CSRF, std::string& Token, bool rem)
	{
		if (this->CheckPass() != 0)
		{
			this->account_id = this->fetchAccountByUsername(this->username).account_id;
			create_session(errorcode, errormsg, CSRF, Token, this->account_id, rem);
			return true;
		}
		else
		{
			errorcode = 500; errormsg = "Wrong password or login";
			return false;
		}
	}

	//requires username and password
	//requires email and phone number
	//
	bool register_account_api_step1(int& errorcode, std::string& errormsg, std::string& CSRF, std::string& Token)
	{
		account temp;

		logger->trace("Username prompted");
		if (!doesUsernameExist(errorcode, errormsg, temp.username))
		{
			logger->trace("Username {} confirmed as non-existent in the database", temp.username);
		}
		else
		{
			logger->trace("Username {} already taken, asking for re-input", temp.username);
			std::cout << "Username already taken" << std::endl;
			errorcode = 409; errormsg = "Username already taken";
			return 0;
		}

		logger->trace("Password prompted");
		std::cout << "Input password : "; std::getline(std::cin, temp.password);
		if (!isStrongPassword(temp.password)) {
			std::cout << "Password is not strong enough. Use at least 1 uppercase, 1 lowercase and 1 number with a length of at least 8 characters\nInput password : "; std::getline(std::cin, temp.password);
			errorcode = 422; errormsg = "Weak password";
			return 0;
		}
		std::string salt = generateSalt(saltlen);
		temp.password = hashString(temp.password + salt);
		logger->trace("Email prompted");
		logger->trace("Phone number prompted");
		if (temp.addaccountdata(temp)) {
			temp.fetchAccountByUsername(temp.username);
			create_session(errorcode, errormsg, CSRF, Token, temp.account_id,0); return 1; 
		}
		else { errorcode = 500; errormsg = "database error"; return 0; }
		
	}

	void session_fetch(int&errorcode, std::string&errormsg ,std::string CSRF, std::string Token)
	{
		int c = fetchAccidAndDeleteExpiredSession(Token, CSRF);
		if(c!=422)
		{
			if (c != 409)
			{
				this->fetchAccountById(c);
			}
			else
			{
				errorcode = 409; errormsg = "Session non existent or expired";
			}
		}
		
	}

	bool register_account_api_step2(int& errorcode, std::string& errormsg, std::string& CSRF, std::string& Token)
	{
		this->session_fetch(errorcode, errormsg, CSRF, Token);

	}

	//Server only, do not use
	bool auth()
	{
		account temp;
		std::cout << "Input your login : "; std::cin >> temp.username;
		std::cout << "Input your password : "; std::cin >> temp.password;
		if (temp.CheckPass() == 1)
		{
			return temp.superuser(temp);
		}
		else if (temp.CheckPass() == 2)
		{
			return temp.typeclient(temp);
		}
		else if (temp.CheckPass() == 3)
		{
			return temp.typebroker(temp);
		}
		else
		{
			std::cout << "Login or Password missmatch" << std::endl;
		}
	}

private:

	//TODO Add error handling
	bool update()
	{
		// Construct the SQL query
		const char* updateSQL = "UPDATE accounts SET "
			"account_type = ?,"
			"account_reference = ?,"
			"email = ?,"
			"phone_number = ?,"
			"login = ?,"
			"password = ? "
			"WHERE account_id = ?";

		// Prepare the SQL statement
		sqlite3_stmt* stmt;
		if (sqlite3_prepare_v2(db, updateSQL, -1, &stmt, nullptr) != SQLITE_OK)
		{
			// Handle SQL statement preparation error
			return false;
		}

		// Bind the parameters to the SQL statement
		sqlite3_bind_int(stmt, 1, this->account_type);
		sqlite3_bind_int(stmt, 2, this->account_reference_id);
		sqlite3_bind_text(stmt, 3, this->email.c_str(), -1, SQLITE_STATIC);
		sqlite3_bind_int(stmt, 4, this->phonenumber);
		sqlite3_bind_text(stmt, 5, this->username.c_str(), -1, SQLITE_STATIC);
		sqlite3_bind_text(stmt, 6, this->password.c_str(), -1, SQLITE_STATIC);
		sqlite3_bind_int(stmt, 7, this->account_id);

		// Execute the SQL statement
		if (sqlite3_step(stmt) != SQLITE_DONE)
		{
			// Handle SQL execution error
			sqlite3_finalize(stmt); // Finalize the statement
			return false;
		}

		// Finalize the statement and return success
		sqlite3_finalize(stmt);
		return true;
	}
};

void PrintJson(const web::json::value& json) {
	// Serialize the JSON value to a string
	std::wstringstream ss;
	json.serialize(ss);

	// Output the JSON as a string to the console
	std::wcout << L"Received JSON: " << ss.str() << std::endl;
}

class RequestHandler {
public:
	web::json::value handleRequest(const web::json::value& data) {
		if (data.has_field(U("FUNCTION"))) {
			utility::string_t function_name = data.at(U("FUNCTION")).as_string();

			if (function_name == U("auth")) {
				return auth(data);
			}
			else {
				// Handle an invalid function name
				return CreateErrorResponse(U("Invalid function name: ") + function_name);
			}
		}
		else {
			// Handle the case where "FUNCTION" is missing
			return CreateErrorResponse(U("Missing FUNCTION parameter"));
		}
	}

	web::json::value auth(const web::json::value& data) {
		// Authentication logic
		web::json::value response;
		int errorcode; std::string errormsg;
		std::string Username = utility::conversions::to_utf8string(data.at(U("LOGIN")).as_string());
		std::string Password = utility::conversions::to_utf8string(data.at(U("PASSWORD")).as_string());
		std::string CSRF = utility::conversions::to_utf8string(data.at(U("CSRF")).as_string());
		bool remember = data.at(U("REMEMBER_ME")).as_bool();
		account temp;
		temp.username = Username;
		temp.password = Password;
		std::string token;
		if (temp.API_auth(errorcode, errormsg, CSRF, token, remember)) {
			std::cout << "Auth success" << std::endl;

			utility::string_t CSRFT = utility::conversions::to_string_t(CSRF);
			response[U("CSRF")] = web::json::value::string(CSRFT);
			response[U("TOKEN")] = web::json::value::string(utility::conversions::to_string_t(token));
			response[U("Response")] = web::json::value::string(U("Authentication successful"));
			return response;
		}
		else {
			std::cout << "Auth fail" << std::endl;
			response[U("Response")] = web::json::value::string(U("Authentication failed"));
			return response;
		}
	}

private:
	web::json::value CreateErrorResponse(const utility::string_t& errorMessage) {
		web::json::value errorResponse;
		errorResponse[U("Error")] = web::json::value::string(errorMessage);
		return errorResponse;
	}
};

class ThreadPool {
public:
	ThreadPool(size_t numThreads) : stop(false) {
		for (size_t i = 0; i < numThreads; ++i) {
			workers.emplace_back([this] {
				while (true) {
					std::function<void()> task;

					{
						std::unique_lock<std::mutex> lock(queueMutex);
						condition.wait(lock, [this] { return stop || !tasks.empty(); });

						if (stop && tasks.empty()) {
							return;
						}

						task = std::move(tasks.front());
						tasks.pop();
					}

					task();
				}
				});
		}
	}

	template <class F>
	void enqueue(F&& f) {
		{
			std::unique_lock<std::mutex> lock(queueMutex);
			tasks.emplace(std::forward<F>(f));
		}
		condition.notify_one();
	}

	~ThreadPool() {
		{
			std::unique_lock<std::mutex> lock(queueMutex);
			stop = true;
		}
		condition.notify_all();

		for (std::thread& worker : workers) {
			worker.join();
		}
	}

private:
	std::vector<std::thread> workers;
	std::queue<std::function<void()>> tasks;

	std::mutex queueMutex;
	std::condition_variable condition;
	bool stop;
};

void handle_request(RequestHandler& handler, SOCKET client_socket) {
	char buffer[1024] = { 0 };

	// Receive data from Python
	recv(client_socket, buffer, sizeof(buffer), 0);
	std::string request_str = buffer;

	// Convert the received data to web::json::value
	web::json::value request_data = web::json::value::parse(request_str);

	// Handle the request
	web::json::value response_data = handler.handleRequest(request_data);

	// Convert the response to a string
	std::string response_str = utility::conversions::to_utf8string(response_data.serialize());

	// Send the response back to Python
	send(client_socket, response_str.c_str(), response_str.length(), 0);

	closesocket(client_socket);
}

int main() {
	if (createTables(create_or_open_db(&db, "localdb.db")) && create_api_key_decrypt_tables(create_or_open_db(&keybase, "keys.db")) && sessions_table(create_or_open_db(&sessions, "sessions.db"))) {
		WSADATA wsaData;
		SOCKET server_fd, new_socket;
		struct sockaddr_in address;
		int addrlen = sizeof(address);

		if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
			std::cerr << "WSAStartup failed" << std::endl;
			return 1;
		}

		if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
			std::cerr << "socket failed" << std::endl;
			WSACleanup();
			return 1;
		}

		address.sin_family = AF_INET;
		address.sin_addr.s_addr = INADDR_ANY;
		address.sin_port = htons(12345); // Adjust the port as needed

		if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) == SOCKET_ERROR) {
			std::cerr << "bind failed" << std::endl;
			closesocket(server_fd);
			WSACleanup();
			return 1;
		}

		if (listen(server_fd, 3) == SOCKET_ERROR) {
			std::cerr << "listen" << std::endl;
			closesocket(server_fd);
			WSACleanup();
			return 1;
		}

		ThreadPool pool(4); // Adjust the number of threads as needed
		RequestHandler requestHandler;

		try {
			while (true) {
				if ((new_socket = accept(server_fd, (struct sockaddr*)&address, &addrlen)) == INVALID_SOCKET) {
					int error_code = WSAGetLastError();
					if (error_code == WSAEINTR) {
						// The accept operation was interrupted, you can continue waiting for connections
						continue;
					}
					else if (error_code == WSAEWOULDBLOCK || error_code == WSAECONNRESET) {
						// Handle specific errors as needed
						// For example, handle WSAEWOULDBLOCK by sleeping for a short time and then retrying
						// For WSAECONNRESET, log the error and continue accepting connections
						// You can add more specific error handling here.
						continue;
					}
					else {
						// Handle other errors
						std::cerr << "Error: " << error_code << std::endl;
						closesocket(server_fd);
						WSACleanup();
						return 1;
					}
				}

				// Use the thread pool to handle the request
				pool.enqueue([&requestHandler, new_socket] {
					handle_request(requestHandler, new_socket);
					closesocket(new_socket);
					});
			}
		}
		catch (const std::exception& e) {
			// Handle exceptions (e.g., log the error)
			std::cerr << "Exception: " << e.what() << std::endl;
		}

		// Cleanup and exit
		closesocket(server_fd);
		WSACleanup();

		return 0;
	}
}

//Failed cpprestsdk attempt
/*
int main() {
	logger->set_level(spdlog::level::info);
	logger->trace("The program was started");
	if (!createTables(create_or_open_db(&db, "localdb.db"))) {
		throw std::runtime_error("Failed to create tables for local database.");
	}
	if (!create_api_key_decrypt_tables(create_or_open_db(&keybase, "keys.db"))) {
		throw std::runtime_error("Failed to create tables for keys database.");
	}
	if (!sessions_table(create_or_open_db(&sessions, "sessions.db"))) {
		throw std::runtime_error("Failed to create sessions table.");
	}

	// Set up an HTTP listener
	web::http::experimental::listener::http_listener listener(L"http://localhost:8080");

	listener.support(web::http::methods::POST, [](web::http::http_request request) {
		std::cout << "request received" << std::endl;
		if (request.headers().content_type() != U("application/json")) {
			request.reply(web::http::status_codes::UnsupportedMediaType, U("Expected JSON data."));
			return;
		}

		request.extract_json().then([request](pplx::task<web::json::value> dataTask) {
			// Create a RequestHandler instance
			RequestHandler requestHandler;

			// Handle the request asynchronously using .then
			dataTask.then([request, &requestHandler](web::json::value data) {
				web::json::value response = requestHandler.handleRequest(data);

				web::http::http_response httpResponse(web::http::status_codes::OK);
				httpResponse.headers().set_content_type(U("application/json"));
				httpResponse.set_body(response);

				// Send the response
				std::cout << "Response sent" << std::endl;
				request.reply(200,response);
				std::cout << "A hundred percent sent" << std::endl;
				PrintJson(response);
				});
			});
		});

	try {
		listener.open().wait();
		std::wcout << L"Listening on http://localhost:8080" << std::endl;

		// Block the main thread to keep the listener running
		std::this_thread::sleep_for(std::chrono::minutes(10 ^ 20));

		listener.close().wait();
	}
	catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
	}

	return 0;
}
*/
//Debug version of main

int mainw()
{
	logger->set_level(spdlog::level::info);
	logger->trace("The program was started");
	if (createTables(create_or_open_db(&db, "localdb.db")) && create_api_key_decrypt_tables(create_or_open_db(&keybase, "keys.db")) && sessions_table(create_or_open_db(&sessions, "sessions.db")))
	{
		const char* sql = "PRAGMA database_list;";
		sqlite3_stmt* stmt;

		if (sqlite3_prepare_v2(keybase, sql, -1, &stmt, nullptr) == SQLITE_OK) {
			while (sqlite3_step(stmt) == SQLITE_ROW) {
				const char* databaseName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
				std::cout << "Connected to database: " << databaseName << std::endl;
			}

			sqlite3_finalize(stmt);
		}
		else {
			std::cerr << "Failed to prepare PRAGMA statement: " << sqlite3_errmsg(keybase) << std::endl;
		}

		logger->trace("All tables created successfully");
		account temp;
		int choice, errorcode;
		bool exitMenu = false;
		std::string errormsg;

		while (!exitMenu) {
			logger->trace("Menu called");
			std::cout << "Menu Options:" << std::endl;
			std::cout << "1. Register" << std::endl;
			std::cout << "2. Authenticate (Auth)" << std::endl;
			std::cout << "3. Exit" << std::endl;
			std::cout << "Enter your choice (1-3): ";

			std::cin >> choice;
			logger->trace("Choice integer equal to {}", choice);
			switch (choice) {
			case 1:
				logger->trace("Case 1 entered, register account called");
				temp.register_account(errorcode, errormsg); // Call the register_account function from your 'temp' object
				break;
			case 2:
				logger->trace("Case 2 entered, auth called");
				temp.auth(); // Call the auth function from your 'temp' object
				break;
			case 3:
				logger->trace("Case 3 entered, exiting program");
				std::cout << "Exiting the program." << std::endl;
				exitMenu = true;
				break;
			default:
				logger->trace("Case default entered, asking for choice re-enter");
				std::cout << "Invalid choice. Please select a valid option (1-3)." << std::endl;
				break;
			}
		}

		return 0;
	}
	else {
		std::cout << "Database critical error. Exiting program" << std::endl;
		logger->critical("Database critical error. Database could not be opened");
		system("pause");
		return 0;
	}
}