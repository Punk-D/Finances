#include "SQLite_functions.h"
#include <iostream>
#include <string>
#include <vector>
#include <openssl/sha.h>
#include <openssl/rand.h>
#include <openssl/evp.h>


//superuser password. this shall be changed soon
std::string supass = "4132";

// Create a file logger



bool doesUsernameExist(const std::string& inputUsername) {
	// Define the SQL query
	const char* selectSQL = "SELECT 1 FROM accounts WHERE login = ? LIMIT 1;";

	// Prepare the statement
	sqlite3_stmt* stmt = nullptr;
	if (sqlite3_prepare_v2(db, selectSQL, -1, &stmt, nullptr) != SQLITE_OK) {
		// Handle the error (e.g., print an error message, finalize stmt, and return false)
		sqlite3_finalize(stmt);
		return false;
	}

	// Bind the input parameter (username)
	if (sqlite3_bind_text(stmt, 1, inputUsername.c_str(), -1, SQLITE_STATIC) != SQLITE_OK) {
		// Handle the error (e.g., print an error message, finalize stmt, and return false)
		std::cout << "ERROR : could not check table accounts" << std::endl;
		sqlite3_finalize(stmt);
		return false;
	}

	// Execute the statement
	if (sqlite3_step(stmt) == SQLITE_ROW) {
		// Username exists in the database
		sqlite3_finalize(stmt); // Finalize the statement
		return true;
	}

	// Username does not exist in the database
	sqlite3_finalize(stmt); // Finalize the statement
	return false;
}

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

class broker
{
public:
	int id;
	std::string name, surname;
	float portfolio_percent;
	std::string api_key;
	int broker_upload(broker brokeri) {
		// Create an INSERT statement for broker
		std::string insertBrokerSQL = "INSERT INTO broker (name, surname, portfolio_percent, api_key) "
			"VALUES (?, ?, ?, ?);";

		sqlite3_stmt* stmt;
		if (sqlite3_prepare_v2(db, insertBrokerSQL.c_str(), -1, &stmt, NULL) == SQLITE_OK) {
			sqlite3_bind_text(stmt, 1, brokeri.name.c_str(), -1, SQLITE_STATIC);
			sqlite3_bind_text(stmt, 2, brokeri.surname.c_str(), -1, SQLITE_STATIC);
			sqlite3_bind_double(stmt, 3, brokeri.portfolio_percent);
			sqlite3_bind_text(stmt, 4, brokeri.api_key.c_str(), -1, SQLITE_STATIC);

			if (sqlite3_step(stmt) == SQLITE_DONE) {
				// Broker inserted successfully, get the last inserted row ID
				int brokerId = sqlite3_last_insert_rowid(db);
				std::cout << "Broker inserted successfully. Broker ID: " << brokerId << std::endl;
				sqlite3_finalize(stmt);
				return brokerId;
			}
			else {
				std::cerr << "Failed to insert broker: " << sqlite3_errmsg(db) << std::endl;
			}

			sqlite3_finalize(stmt);
		}
		else {
			std::cerr << "Failed to prepare INSERT statement: " << sqlite3_errmsg(db) << std::endl;
		}

		// Return -1 to indicate failure
		return -1;
	}

	int addBroker()
	{
		broker temp;
		std::cout << "Input name and surname : ";
		std::cin >> temp.name >> temp.surname;
		std::cout << "Input API key : "; std::cin >> temp.api_key;
		temp.portfolio_percent = 0;
		return temp.broker_upload(temp);
	}
	void coutallbrokers()
	{
		broker cycle;
		for (int i = 0; i < countRows("broker"); i++)
		{
			cycle = broker_fetch(i);
			std::cout << "ID : " << cycle.id << " " << "Name : " << cycle.name << " " << cycle.surname << std::endl;
		}
	}

	float portfolio_in_USDT()
	{
		//moreover a placeholder for now as i do not have access to the Binance API
		//this part WILL be updated as soon as i get my hands on a working API key and an account i can test on
		std::cout << "Input placeholding data (portfolio cost in usdt) : ";
		float sum;
		std::cin >> sum;
		return sum;
	}
	float portfolio_percent_USDT()
	{
		return (portfolio_percent / 100) * portfolio_in_USDT();
	}
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

	int insertClientData(const client& c, int brokerId) {
		std::string insertClientSQL = "INSERT INTO client (name, surname, cut_percent, portfolio_percent, broker_id) "
			"VALUES (?, ?, ?, ?, ?);";

		sqlite3_stmt* stmt;
		if (sqlite3_prepare_v2(db, insertClientSQL.c_str(), -1, &stmt, NULL) == SQLITE_OK) {
			sqlite3_bind_text(stmt, 1, c.name.c_str(), -1, SQLITE_STATIC);
			sqlite3_bind_text(stmt, 2, c.surname.c_str(), -1, SQLITE_STATIC);
			sqlite3_bind_double(stmt, 3, c.cut_percent);
			sqlite3_bind_double(stmt, 4, c.portfolio_percent);
			sqlite3_bind_int(stmt, 5, brokerId);

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
	void coutallclients()
	{
		client out;
		for (int i = 0; i < countRows("client"); i++)
		{
			out = client_fetch(i);
			std::cout << "\tClient ID: " << out.id << ", Name: " << out.name << " " << out.surname << ", broker ID : " << out.broker_id << std::endl;
		}
	}
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
		return temp.insertClientData(temp, temp.broker_id);
	}

	bool updateClientById(const client& updatedClient) {
		// Prepare the SQL UPDATE statement
		std::string query = "UPDATE client SET name=?, surname=?, cut_percent=?, portfolio_percent=? WHERE id=?";
		sqlite3_stmt* stmt;

		if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, 0) == SQLITE_OK) {
			// Bind values to placeholders
			sqlite3_bind_text(stmt, 1, updatedClient.name.c_str(), -1, SQLITE_STATIC);
			sqlite3_bind_text(stmt, 2, updatedClient.surname.c_str(), -1, SQLITE_STATIC);
			sqlite3_bind_double(stmt, 3, updatedClient.cut_percent);
			sqlite3_bind_double(stmt, 4, updatedClient.portfolio_percent);
			sqlite3_bind_int(stmt, 5, updatedClient.id);

			// Execute the SQL statement
			int result = sqlite3_step(stmt);
			sqlite3_finalize(stmt);

			if (result == SQLITE_DONE) {
				// Update was successful
				return true;
			}
			else {
				// Handle the error if needed
				std::cerr << "Error updating client: " << sqlite3_errmsg(db) << std::endl;
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

				// Create a Client object and set all the fields
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
	void insertTransactionData(const transaction& t, int brokerId, int clientId) {
		std::string insertTransactionSQL = "INSERT INTO transactions (broker_id, client_id, transaction_type, transaction_initiator, "
			"balance_before_transaction, balance_after_transaction, transaction_sum) "
			"VALUES (?, ?, ?, ?, ?, ?, ?);";

		sqlite3_stmt* stmt;
		if (sqlite3_prepare_v2(db, insertTransactionSQL.c_str(), -1, &stmt, NULL) == SQLITE_OK) {
			sqlite3_bind_int(stmt, 1, brokerId);
			sqlite3_bind_int(stmt, 2, clientId);
			sqlite3_bind_int(stmt, 3, t.transaction_type);
			sqlite3_bind_int(stmt, 4, t.transaction_initiator);
			sqlite3_bind_double(stmt, 5, t.balance_before_transaction);
			sqlite3_bind_double(stmt, 6, t.balance_after_transaction);
			sqlite3_bind_double(stmt, 7, t.trasactionsum);

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
	void createtransaction()
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
					b = b.client_fetch(i);
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
					vec[i].updateClientById(vec[i]);
				}
			}
			broker_sum = broker_sum + (a.portfolio_percent / 100) * temp.balance_before_transaction;
			a.portfolio_percent = (broker_sum / temp.balance_after_transaction) * 100;
			b.updateClientById(b);
			a.updateBrokerById(a);
			temp.insertTransactionData(temp, a.id, b.id);
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
					b.updateClientById(vec[i]);
				}
				a.portfolio_percent = (broker_sum / temp.balance_after_transaction) * 100;
			}
			a.updateBrokerById(a);
			temp.insertTransactionData(temp, a.id, NULL);
		}
	}
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

	void createtransactionclient(client b)
	{
		transaction temp;
		temp.transaction_initiator = 0;
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
		temp.client_id = b.id;
		temp.broker_id = b.broker_id;
		broker a;
		a = a.getBrokerById(temp.broker_id);
		b = b.getClientById(temp.client_id);
		idExists = 1;


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
				vec[i].updateClientById(vec[i]);
			}
		}
		broker_sum = broker_sum + (a.portfolio_percent / 100) * temp.balance_before_transaction;
		a.portfolio_percent = (broker_sum / temp.balance_after_transaction) * 100;
		b.updateClientById(b);
		a.updateBrokerById(a);
		temp.insertTransactionData(temp, a.id, b.id);
	}

	void createtransactionbroker(broker a)
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
				b.updateClientById(vec[i]);
			}
			a.portfolio_percent = (broker_sum / temp.balance_after_transaction) * 100;
		}
		a.updateBrokerById(a);
		temp.insertTransactionData(temp, a.id, NULL);
	}
};

class account
{
	int account_type;
	std::string username;
	std::string password;
	std::string email;
	int phonenumber;
	int account_id;
	int account_reference_id;
public:
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

	bool register_account()
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
			std::cout << "Input username : "; std::cin >> temp.username;
			if (!doesUsernameExist(temp.username))
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
		std::cout << "Input password : "; std::cin >> temp.password;
		logger->trace("Email prompted");
		std::cout << "Input email : "; std::cin >> temp.email;
		logger->trace("Phone number prompted");
		std::cout << "Input phone number : "; std::cin >> temp.phonenumber;
		if (temp.account_type == 2)
		{
			client clienttemp;
			temp.account_reference_id = clienttemp.addClient();
			return temp.addaccountdata(temp);
		}
		else if (temp.account_type == 3)
		{
			broker brokertemp;
			temp.account_reference_id = brokertemp.addBroker();
			return temp.addaccountdata(temp);
		}
		else {
			logger->info("Superuser {} created", temp.username);
			temp.account_reference_id = NULL;
			return temp.addaccountdata(temp);
		}
	}
	bool typeclient(account currentaccount)
	{
		client currentuser; currentuser = currentuser.client_fetch(currentaccount.account_reference_id);
		transaction temp;
		int choice;
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
				temp.createtransactionclient(currentuser);
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
	bool typebroker(account currentaccount)
	{
		broker currentuser; currentuser = currentuser.broker_fetch(currentaccount.account_reference_id); transaction temp;
		int choice;
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
				temp.createtransactionbroker(currentuser);
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
	bool superuser(account currentuser)
	{
		logger->info("Superuser {} signed in", currentuser.username);
		std::vector<transaction> transactions;
		broker temp; transaction transacc; client tempp; account acc;
		int choice;

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
				choice = temp.addBroker();
				logger->info("Broker with id {} was created by superuser {}", choice, currentuser.username);
				break;
			case 2:
				// Input Client logic here
				choice = tempp.addClient();
				logger->info("Client with id {} was created by superuser {}", choice, currentuser.username);
				break;
			case 3:
				// Input Transaction logic here
				transacc.createtransaction();
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

				acc.register_account();
			default:
				std::cout << "Invalid choice. Please enter a valid option (1-7)." << std::endl;
			}
		}
	}

	int CheckPass(const std::string& login, const std::string& password) {
		// SQL statement to select an account based on login
		const char* selectSQL = "SELECT account_type, password FROM accounts WHERE login = ? LIMIT 1;";

		sqlite3_stmt* stmt;

		// Prepare the SQL statement
		if (sqlite3_prepare_v2(db, selectSQL, -1, &stmt, nullptr) == SQLITE_OK) {
			// Bind the login parameter to the statement
			sqlite3_bind_text(stmt, 1, login.c_str(), -1, SQLITE_STATIC);

			// Execute the statement
			if (sqlite3_step(stmt) == SQLITE_ROW) {
				// Account with the given login exists
				// Check if the provided password matches
				const char* storedPassword = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
				if (password == storedPassword) {
					// Password matches, return the account type
					int accountType = sqlite3_column_int(stmt, 0);
					sqlite3_finalize(stmt);
					return accountType;
				}
			}

			// Finalize the statement
			sqlite3_finalize(stmt);
		}

		// Account does not exist or password does not match
		return 0;
	}

	bool auth()
	{
		account temp;
		std::cout << "Input your login : "; std::cin >> temp.username;
		std::cout << "Input your password : "; std::cin >> temp.password;
		if (temp.CheckPass(temp.username, temp.password) == 1)
		{
			return temp.superuser(temp);
		}
		else if (temp.CheckPass(temp.username, temp.password) == 2)
		{
			return temp.typeclient(temp);
		}
		else if (temp.CheckPass(temp.username, temp.password) == 3)
		{
			return temp.typebroker(temp);
		}
		else
		{
			std::cout << "Login or Password missmatch" << std::endl;
		}
	}
private:
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
};

// Hash a password using bcrypt and return the hash
//std::string hashPassword(const std::string& password) {
//	bcrypt bcrypt;
//	std::string hash;
//
//	if (bcrypt.GenerateHash(password, 12, hash) != 0) {
//		std::cerr << "Error hashing password." << std::endl;
//		// Handle error if necessary
//	}
//
//	return hash;
//}

// Verify a password against a stored hash
//bool verifyPassword(const std::string& password, const std::string& storedHash) {
//	BCrypt bcrypt;
//
//	if (bcrypt.VerifyHash(password, storedHash) == BCrypt::HashResult::Ok) {
//		return true; // Passwords match
//	}
//
//	return false; // Passwords don't match
//}

int main()
{
	logger->set_level(spdlog::level::info);
	logger->trace("The program was started");
	if (createTables(create_or_open_db()))
	{
		logger->trace("All tables created successfully");
		account temp;
		int choice;
		bool exitMenu = false;

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
				temp.register_account(); // Call the register_account function from your 'temp' object
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