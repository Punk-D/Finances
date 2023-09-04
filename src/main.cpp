#include <iostream>
#include <string>

using namespace std;
class client
{
public:
	int id;
	string name, surname;
	int broker_id;
	float balance;
	float cut_percent;
	float portfolio_percent;
};
class broker
{
public:
	int id;
	string name, surname;
	float balance;
	float portfolio_percent;
	string api_key;
	int portfolio_in_USDT()
	{
		//moreover a placeholder for now as i do not have access to the Binance API
		//this part WILL be updated as soon as i get my hands on a working API key and an account i can test on
	}
};
class transaction
{
public:
	int id;
	client * client1;
	broker* broker1;
	string transaction_type, transaction_initiator;
	float balance_before_transaction, balance_after_transaction;
	void createtransaction()
	{

	}
private:
	
};
int main()
{

}