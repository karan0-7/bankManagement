#include <string>
#include <vector>

enum class Operation
{
    deposit,
    withdraw,
    transfer
};

class Transaction
{
private:
    int id;
    int amount;
    Operation operation;

public:
    Transaction(int p_amount, Operation p_operation) : amount{p_amount}, operation{p_operation}
    {
    }
};

class Account
{
private:
    static int id;
    int balance{};
    std::string type{};
    std::vector<Transaction> transactionHistory;

public:
    Account(int p_balance, std::string p_type) : balance{p_balance}, type{p_type}
    {
    }

    int getId()
    {
        return id;
    }

    int getBalance()
    {
        return balance;
    }

    Account *deposit(int amount)
    {
        balance += amount;
        Transaction transaction{amount, Operation::deposit};
        transactionHistory.push_back(transaction);
        return this;
    }

    Account *widthdrawal(int p_amount)
    {
        if (p_amount > balance)
        {
        }
        balance -= p_amount;
        Transaction transaction{p_amount, Operation::withdraw};
        transactionHistory.push_back(transaction);
        return this;
    }
};
