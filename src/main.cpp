#include <iostream>
#include <string>
#include <vector>
#include "SQLite_functions.h"

//using namespace std;

class client
{
public:
	int id;
	std::string name, surname;
	float cut_percent;
	float portfolio_percent;
};

class broker
{
public:
	int id;
	std::string name, surname;
	float portfolio_percent;
	std::string api_key;
	std::vector <client> clients;
	void addBroker(std::vector<broker>& brokers)
	{
		broker temp;
		if (brokers.empty())
		{
			temp.id = 0;
		}
		else
		{
			broker back = brokers.back();
			temp.id = back.id + 1;
		}
		std::cout << "Input name and surname : ";
		std::cin >> temp.name >> temp.surname;
		std::cout << "Input API key : "; std::cin >> temp.api_key;
		temp.portfolio_percent = 0;
		brokers.push_back(temp);
	}

	void addClient(std::vector<broker>& brokers)
	{
		client temp;
		if (clients.empty())
		{
			temp.id = 0;
		}
		else
		{
			client back = clients.back();
			temp.id = back.id + 1;
		}
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
		clients.push_back(temp);
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
};

class transaction
{
public:
	int id;
	client* client1=nullptr;
	broker* broker1;
	bool transaction_type, transaction_initiator;
	float balance_before_transaction, balance_after_transaction, trasactionsum;
	void createtransaction(std::vector<broker>* brokers, std::vector<transaction>* transactions)
	{
		transaction temp;
		if ((*transactions).empty())
		{
			temp.id = 0;
		}
		else
		{
			transaction back = (*transactions).back();
			temp.id = back.id + 1;
		}
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

		std::cout << "Choose your broker from the list. " << std::endl;
		for (int i = 0; i < (*brokers).size(); i++)
		{
			std::cout << (*brokers)[i].id << "\t" << (*brokers)[i].name << " " << (*brokers)[i].surname << std::endl;
		}
		broker* inputbroker = nullptr; int id; idExists = 0;
		std::cout << "Input ID : "; std::cin >> id;
		while (!idExists)
		{
			for (int i = 0; i < (*brokers).size(); i++)
			{
				if ((*brokers)[i].id == id)
				{
					idExists = 1;
					inputbroker = &(*brokers)[i];
				}
			}
			if (!idExists)
			{
				std::cout << "Please enter a valid id" << std::endl;
				for (int i = 0; i < (*brokers).size(); i++)
				{
					std::cout << (*brokers)[i].id << "\t" << (*brokers)[i].name << " " << (*brokers)[i].surname << std::endl;
				}
				std::cout << "Input : "; std::cin >> id;
			}
			temp.broker1 = inputbroker;
		}

		if (!temp.transaction_initiator) 
		{
			std::cout << "Choose your client from the list. " << std::endl;
			for (int i = 0; i < inputbroker->clients.size(); i++)
			{
				std::cout << inputbroker->clients[i].id << "\t" << inputbroker->clients[i].name << " " << inputbroker->clients[i].surname << std::endl;
			}
			client* inputclient = nullptr;
			std::cout << "Input ID : "; std::cin >> id;  idExists = 0;
			while (!idExists)
			{
				for (int i = 0; i < (*brokers).size(); i++)
				{
					if (inputbroker->clients[i].id == id)
					{
						idExists = 1;
						inputclient = &inputbroker->clients[i];
					}
				}
				if (!idExists)
				{
					std::cout << "Please enter a valid id" << std::endl;
					for (int i = 0; i < inputbroker->clients.size(); i++)
					{
						std::cout << inputbroker->clients[i].id << "\t" << inputbroker->clients[i].name << " " << inputbroker->clients[i].surname << std::endl;
					}
					std::cout << "Input : "; std::cin >> id;
				}
				temp.client1 = inputclient;
			}

			
			bool sum_is_possible = 0; float clientsum; float broker_sum=0;
			temp.balance_before_transaction = temp.broker1->portfolio_in_USDT();
			while (!sum_is_possible) {
				std::cout << "Input transaction sum : "; std::cin >> temp.trasactionsum;
				
				
				if (temp.transaction_type)
				{
					broker_sum = (temp.client1->cut_percent / 100) * temp.trasactionsum;
					clientsum = temp.trasactionsum-broker_sum + (temp.client1->portfolio_percent / 100) * temp.balance_before_transaction;
					temp.balance_after_transaction = temp.balance_before_transaction + temp.trasactionsum;
					sum_is_possible = 1;
				}
				else
				{
					clientsum = (temp.client1->portfolio_percent / 100) * temp.balance_before_transaction - temp.trasactionsum;
					if (clientsum < 0)
					{
						std::cout << "You do not have enough funds to withdraw, your current percent is worth " << (temp.broker1->portfolio_percent / 100) * temp.balance_before_transaction << "$" << std::endl;
					}
					else { temp.balance_after_transaction = temp.balance_before_transaction - temp.trasactionsum; sum_is_possible = 1; }
				}
			}
			
			
			temp.client1->portfolio_percent = (clientsum / temp.balance_after_transaction) * 100;
			float tempclientsum;
			for (int i = 0; i < temp.broker1->clients.size(); i++)
			{
				if (&temp.broker1->clients[i] != temp.client1) {
					tempclientsum = (broker1->clients[i].portfolio_percent / 100) * temp.balance_before_transaction;
					temp.broker1->clients[i].portfolio_percent = (tempclientsum / temp.balance_after_transaction) * 100;
				}
			}
			broker_sum = broker_sum + (temp.broker1->portfolio_percent / 100) * temp.balance_before_transaction;
			temp.broker1->portfolio_percent = (broker_sum / temp.balance_after_transaction) * 100;
			(*transactions).push_back(temp);
		}
		else
		{
			
			{
				bool sum_is_possible = 0; float broker_sum;
				while (!sum_is_possible) {
					std::cout << "Input transaction sum : "; std::cin >> temp.trasactionsum;
					
					temp.balance_before_transaction = temp.broker1->portfolio_in_USDT();
					if (temp.transaction_type)
					{
						broker_sum = temp.trasactionsum + (temp.broker1->portfolio_percent / 100) * temp.balance_before_transaction;
						temp.balance_after_transaction = temp.balance_before_transaction + temp.trasactionsum;
						sum_is_possible = 1;
					}
					else
					{
						broker_sum = (temp.broker1->portfolio_percent / 100) * temp.balance_before_transaction - temp.trasactionsum;
						if (broker_sum < 0)
						{
							std::cout << "You do not have enough funds to withdraw, your current percent is worth " << (temp.broker1->portfolio_percent / 100) * temp.balance_before_transaction << "$" << std::endl;
						}
						else { temp.balance_after_transaction = temp.balance_before_transaction - temp.trasactionsum; sum_is_possible = 1;}
					}
				}
				float tempclientsum;
				for (int i = 0; i < temp.broker1->clients.size(); i++)
				{
					tempclientsum = (temp.broker1->clients[i].portfolio_percent / 100) * temp.balance_before_transaction;
					temp.broker1->clients[i].portfolio_percent = (tempclientsum / temp.balance_after_transaction) * 100;
				}
				temp.broker1->portfolio_percent = (broker_sum / temp.balance_after_transaction) * 100;
			}
			(*transactions).push_back(temp);
		}
	}
};

void client_init(std::vector <broker>* brokers)
{
	int temp;
	std::cout << "Choose your broker from the list. Input the ID : " << std::endl;
	for (int i = 0; i < brokers->size(); i++)
	{
		std::cout << (*brokers)[i].id << "\t" << (*brokers)[i].name << " " << (*brokers)[i].surname << std::endl;
	}
	std::cout << "Input : "; std::cin >> temp; bool idExists = 0;
	while (!idExists) {
		for (int i = 0; i < (*brokers).size(); i++)
		{
			if ((*brokers)[i].id == temp)
			{
				idExists = 1;
				(*brokers)[i].addClient(*brokers);
			}
		}
		if (!idExists)
		{
			std::cout << "Please enter a valid id" << std::endl;
			for (int i = 0; i < (*brokers).size(); i++)
			{
				std::cout << (*brokers)[i].id << "\t" << (*brokers)[i].name << " " << (*brokers)[i].surname << std::endl;
			}
			std::cout << "Input : "; std::cin >> temp;
		}
	}
}

void outputBrokers(const std::vector<broker>& brokers) {
	std::cout << "List of Brokers:" << std::endl;
	for (const broker& b : brokers) {
		std::cout << "ID: " << b.id << ", Name: " << b.name << " " << b.surname << ", Number of clients: " << b.clients.size() << std::endl;
	}
}

// Function to output a list of clients
void outputClients(const std::vector<broker>& brokers) {
	std::cout << "List of Clients:" << std::endl;
	for (int i = 0; i < brokers.size(); i++) {
		std::cout << "Broker: " << std::endl;
		std::cout << "ID: " << brokers[i].id << ", Name: " << brokers[i].name << " " << brokers[i].surname << std::endl;
		for (int j = 0; j < brokers[i].clients.size(); j++)
		{
			std::cout << "\tClient ID: " << brokers[i].clients[j].id << ", Name: " << brokers[i].clients[j].name << " " << brokers[i].clients[j].surname << std::endl;
		}
	}
}

// Function to output a list of transactions
void outputTransactions(const std::vector<transaction>& transactions, std::vector<broker> brokers) {
	std::cout << "List of Transactions:" << std::endl;
	for (const transaction& t : transactions) {
		std::cout << "ID: " << t.id; if (t.client1 != nullptr) { std::cout <<", Client: " << t.client1->name << " " << t.client1->surname; } std::cout<< "\t";
		std::cout << " Broker: " << t.broker1->name << " " << t.broker1->surname << std::endl;
		std::cout << "Balance before transaction : " << t.balance_before_transaction << " \t Transaction sum : " << t.trasactionsum << " \t Balance after transaction : " << t.balance_after_transaction << std::endl;
		std::cout << "Broker's percent : " << t.broker1->portfolio_percent; if (t.client1 != nullptr) {	std::cout << "\t Client percent : " << t.client1->portfolio_percent;} std::cout<< std::endl;
	}
}

int main()
{
	std::vector<broker> brokers;
	std::vector<transaction> transactions;
	broker temp; transaction transacc;
	int choice;
	broker _11; client _22;
	_11.id = 0; _11.api_key = "787"; _11.name = "1"; _11.surname = "1"; _11.portfolio_percent = 0; 
	_22.id = 0; _22.cut_percent = 10; _22.name = "2"; _22.surname = "2"; _22.portfolio_percent = 0;
	_11.clients.push_back(_22);
	brokers.push_back(_11);
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
			temp.addBroker(brokers);
			break;
		case 2:
			// Input Client logic here
			client_init(&brokers);
			break;
		case 3:
			// Input Transaction logic here
			transacc.createtransaction(&brokers, &transactions);
			break;
		case 4:
			// Output list of brokers
			outputBrokers(brokers);
			break;
		case 5:
			// Output list of clients
			outputClients(brokers);
			break;
		case 6:
			// Output list of transactions
			outputTransactions(transactions, brokers);
			break;
		case 7:
			// Exit the program
			std::cout << "Exiting the program." << std::endl;
			return 0;
		default:
			std::cout << "Invalid choice. Please enter a valid option (1-7)." << std::endl;
		}
	}
}