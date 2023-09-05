#include <iostream>
#include <string>
#include <vector>

using namespace std;

class broker
{
public:
	int id;
	string name, surname;
	float portfolio_percent;
	string api_key;
    void addBroker(vector<broker>&brokers)
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
        brokers.push_back(temp);
    }
    int portfolio_in_USDT(broker a)
	{
		//moreover a placeholder for now as i do not have access to the Binance API
		//this part WILL be updated as soon as i get my hands on a working API key and an account i can test on
		int sum;
		cin >> sum;
		return sum;
	}
    int portfolio_percent_USDT(broker a)
    {
        return (a.portfolio_percent / 100)* portfolio_in_USDT(a);
    }
};
class client
{
public:
    int id;
    string name, surname;
    int broker_id;
    float cut_percent;
    float portfolio_percent;
    void addClient(vector<client>& clients, vector<broker>& brokers)
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
        cout << "Choose your broker from the list. Input the ID : " << endl;
        for (int i = 0; i < brokers.size(); i++)
        {
            cout << brokers[i].id << "\t" << brokers[i].name << " " << brokers[i].surname << endl;
        }
        cout << "Input : "; cin >> temp.id; bool idExists = 0;
        while (!idExists) {
            for (int i = 0; i < brokers.size(); i++)
            {
                if (brokers[i].id == temp.id)
                {
                    idExists = 1;
                    
                }
            }
            if (!idExists)
            {
                cout << "Please enter a valid id" << endl;
                for (int i = 0; i < brokers.size(); i++)
                {
                    cout << brokers[i].id << "\t" << brokers[i].name << " " << brokers[i].surname << endl;
                }
                cout << "Input : "; cin >> temp.id;
            }
        }
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
        clients.push_back(temp);
    }
};
class transaction
{
public:
	int id;
	client* client1;
	broker* broker1;
	string transaction_type, transaction_initiator;
	float balance_before_transaction, balance_after_transaction;
	void createtransaction(vector<broker> brokers, vector<transaction> transactions, vector<client> clients)
	{
        transaction temp;
        if (transactions.empty())
        {
            temp.id = 0;
        }
        else
        {           
            transaction back = transactions.back();
            temp.id = back.id + 1;
        }
        cout << "Choose the client out of the list. Input the client's ID " << endl;
        for (int i = 0; i < clients.size(); i++)
        {
            cout << clients[i].id << "\t" << clients[i].name << " " << clients[i].surname << endl;
        }
	}
private:
	
};
int main()
{
	vector<broker> brokers;
	vector<transaction> transactions;
	vector<client> clients;
    int choice;
    while (true) {
        std::cout << "Menu:" << std::endl;
        std::cout << "1. Input Broker" << std::endl;
        std::cout << "2. Input Client" << std::endl;
        std::cout << "3. Input Transaction" << std::endl;
        std::cout << "4. Exit" << std::endl;
        std::cout << "Enter your choice: ";

        if (!(std::cin >> choice)) {
            std::cerr << "Invalid input. Please enter a valid number." << std::endl;
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            continue;
        }
        broker temp;
        client tempp;
        transaction transac;
        switch (choice) {
        case 1:
            
            temp.addBroker(brokers);
            break;
        case 2:
            tempp.addClient(clients,brokers);
            // Call a function to input a client
            break;
        case 3:
            // Input Transaction logic here
            std::cout << "You chose Input Transaction." << std::endl;
            // Call a function to input a transaction
            break;
        case 4:
            // Exit the program
            std::cout << "Exiting the program." << std::endl;
            return 0;
        default:
            std::cout << "Invalid choice. Please enter a valid option (1-4)." << std::endl;
        }
    }
}