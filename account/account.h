#pragma once
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

    int getAmount()
    {
        return amount;
    }

    std::string getOperation()
    {
        switch (operation)
        {
        case Operation::deposit:
            return "desposit";
            break;
        case Operation::withdraw:
            return "withdrawal";
            break;
        case Operation::transfer:
            return "transfer";
            break;

        default:
            break;
        }
    }
};

class Account
{
private:
    static int nextId;
    int id;
    int balance{};
    std::string type{};
    std::vector<Transaction> transactionHistory;

public:
    Account(int p_balance, std::string p_type) : balance{p_balance}, type{p_type}
    {
        id = ++nextId;
    }

    int getId()
    {
        return id;
    }

    int getBalance()
    {
        return balance;
    }

    std::vector<Transaction> getTransactions()
    {
        return transactionHistory;
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

Account addAccount();