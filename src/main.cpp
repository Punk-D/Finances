#include <iostream>
#include <string>
#include <vector>

using namespace std;

class client
{
public:
	int id;
	string name, surname;
	float cut_percent;
	float portfolio_percent;
};

class broker
{
public:
	int id;
	string name, surname;
	float portfolio_percent;
	string api_key;
	vector <client> clients;
	void addBroker(vector<broker>& brokers)
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
		cout << "Input name and surname : ";
		cin >> temp.name >> temp.surname;
		cout << "Input API key : "; cin >> temp.api_key;
		temp.portfolio_percent = 0;
		brokers.push_back(temp);
	}

	void addClient(vector<broker>& brokers)
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
		cout << "Input name and surname : "; cin >> temp.name >> temp.surname;

		cout << "Input broker's cut (%) : "; cin >> temp.cut_percent;
		bool cutIsPossible = 0;
		while (!cutIsPossible)
		{
			if (temp.cut_percent > 100 || temp.cut_percent < 0)
			{
				cout << "Please enter a valid percentage : "; cin >> temp.cut_percent;
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
		cout << "Input placeholding data (portfolio cost in usdt) : ";
		float sum;
		cin >> sum;
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
	client* client1;
	broker* broker1;
	string transaction_type, transaction_initiator;
	float balance_before_transaction, balance_after_transaction, trasactionsum;
	void createtransaction(vector<broker>* brokers, vector<transaction>* transactions)
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

		cout << "Choose your broker from the list. " << endl;
		for (int i = 0; i < (*brokers).size(); i++)
		{
			cout << (*brokers)[i].id << "\t" << (*brokers)[i].name << " " << (*brokers)[i].surname << endl;
		}
		broker* inputbroker = nullptr; int id;
		cout << "Input ID : "; cin >> id; bool idExists = 0;
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
				cout << "Please enter a valid id" << endl;
				for (int i = 0; i < (*brokers).size(); i++)
				{
					cout << (*brokers)[i].id << "\t" << (*brokers)[i].name << " " << (*brokers)[i].surname << endl;
				}
				cout << "Input : "; cin >> id;
			}
			temp.broker1 = inputbroker;
		}

		cout << "Choose your client from the list. " << endl;
		for (int i = 0; i < inputbroker->clients.size(); i++)
		{
			cout << inputbroker->clients[i].id << "\t" << inputbroker->clients[i].name << " " << inputbroker->clients[i].surname << endl;
		}
		client* inputclient = nullptr;
		cout << "Input ID : "; cin >> id;  idExists = 0;
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
				cout << "Please enter a valid id" << endl;
				for (int i = 0; i < inputbroker->clients.size(); i++)
				{
					cout << inputbroker->clients[i].id << "\t" << inputbroker->clients[i].name << " " << inputbroker->clients[i].surname << endl;
				}
				cout << "Input : "; cin >> id;
			}
			temp.client1 = inputclient;
		}

		cout << "Input transaction sum : "; cin >> temp.trasactionsum;
		float broker_sum = (temp.client1->cut_percent / 100) * temp.trasactionsum;
		float clientsum = temp.trasactionsum - broker_sum;
		temp.balance_before_transaction = temp.broker1->portfolio_in_USDT();
		temp.balance_after_transaction = temp.balance_before_transaction + temp.trasactionsum;
		clientsum = clientsum + (temp.client1->portfolio_percent / 100) * temp.balance_before_transaction;
		temp.client1->portfolio_percent = (clientsum / temp.balance_after_transaction) * 100;

		float tempclientsum;
		for (int i = 0; i < temp.broker1->clients.size(); i++)
		{
			if (&temp.broker1->clients[i] != temp.client1) {
				tempclientsum = (broker1->clients[i].portfolio_percent / 100) * temp.balance_before_transaction;
				broker1->clients[i].portfolio_percent = (tempclientsum / temp.balance_after_transaction) * 100;
			}
		}
		broker_sum = broker_sum + (temp.broker1->portfolio_percent / 100) * temp.balance_before_transaction;
		temp.broker1->portfolio_percent = (broker_sum / temp.balance_after_transaction) * 100;
		(*transactions).push_back(temp);
	}

};

void client_init(vector <broker>* brokers)
{
	int temp;
	cout << "Choose your broker from the list. Input the ID : " << endl;
	for (int i = 0; i < brokers->size(); i++)
	{
		cout << (*brokers)[i].id << "\t" << (*brokers)[i].name << " " << (*brokers)[i].surname << endl;
	}
	cout << "Input : "; cin >> temp; bool idExists = 0;
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
			cout << "Please enter a valid id" << endl;
			for (int i = 0; i < (*brokers).size(); i++)
			{
				cout << (*brokers)[i].id << "\t" << (*brokers)[i].name << " " << (*brokers)[i].surname << endl;
			}
			cout << "Input : "; cin >> temp;
		}
	}

}

void outputBrokers(const vector<broker>& brokers) {
	cout << "List of Brokers:" << endl;
	for (const broker& b : brokers) {
		cout << "ID: " << b.id << ", Name: " << b.name << " " << b.surname << ", Number of clients: " << b.clients.size() << endl;
	}
}

// Function to output a list of clients
void outputClients(const vector<broker>& brokers) {
	cout << "List of Clients:" << endl;
	for (int i = 0; i < brokers.size(); i++) {
		cout << "Broker: " << endl;
		cout << "ID: " << brokers[i].id << ", Name: " << brokers[i].name << " " << brokers[i].surname << endl;
		for (int j = 0; j < brokers[i].clients.size(); j++)
		{
			cout << "\tClient ID: " << brokers[i].clients[j].id << ", Name: " << brokers[i].clients[j].name << " " << brokers[i].clients[j].surname << endl;
		}
	}
}

// Function to output a list of transactions
void outputTransactions(const vector<transaction>& transactions, vector<broker> brokers) {
	cout << "List of Transactions:" << endl;
	for (const transaction& t : transactions) {
		cout << "ID: " << t.id << ", Client: " << t.client1->name << " " << t.client1->surname;
		cout << ", Broker: " << t.broker1->name << " " << t.broker1->surname << endl;
		cout << "Balance before transaction : " << t.balance_before_transaction << " \t Transaction sum : " << t.trasactionsum << " \t Balance after transaction : " << t.balance_after_transaction << endl;
		cout << "Broker's percent : " << t.broker1->portfolio_percent << "\t Client percent : " << t.client1->portfolio_percent << endl;
	}
}

int main()
{
	vector<broker> brokers;
	vector<transaction> transactions;
	broker temp; transaction transacc;
	int choice;

	while (true) {
		cout << "Menu:" << endl;
		cout << "1. Input Broker" << endl;
		cout << "2. Input Client" << endl;
		cout << "3. Input Transaction" << endl;
		cout << "4. Output Brokers" << endl;
		cout << "5. Output Clients" << endl;
		cout << "6. Output Transactions" << endl;
		cout << "7. Exit" << endl;
		cout << "Enter your choice: ";

		if (!(cin >> choice)) {
			cerr << "Invalid input. Please enter a valid number." << endl;
			cin.clear();
			cin.ignore(numeric_limits<streamsize>::max(), '\n');
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
			cout << "Exiting the program." << endl;
			return 0;
		default:
			cout << "Invalid choice. Please enter a valid option (1-7)." << endl;
		}
	}
}