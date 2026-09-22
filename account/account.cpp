#include "./account.h"
#include <string>
#include <iostream>

int Account::nextId = 0;
int Transaction::nextId = 0;

Account addAccount()
{
    int balance{};
    std::string type{};
    std::cout << "Please enter your starting balance: " << '\n';
    std::cin >> balance;
    std::cout << "Please enter your account type: " << '\n';
    std::cin.ignore();
    std::getline(std::cin, type);
    Account account{balance, type};
    return account;
}