#include <iostream>
#include "./user/user.h"
#include "./account/account.h"
#include "./bank.h"

std::vector<bankActions> bankOperations{
    bankActions::ADD_ACCOUNT,
    bankActions::ADD_USER,
    bankActions::DEPOSIT,
    bankActions::WIDTHDRAW,
    bankActions::TRANSFER,
    bankActions::GET_ACCOUNT_INFO,
    bankActions::GET_USER_INFO,
    bankActions::EXIT};

void displayMenu()
{

    int displayId = 0;

    for (bankActions action : bankOperations)
    {
        switch (action)
        {
        case bankActions::ADD_ACCOUNT:
            std::cout << ++displayId << " : " << "Create an account" << '\n';
            break;
        case bankActions::ADD_USER:
            std::cout << ++displayId << " : " << "Create a user" << '\n';
            break;
        case bankActions::DEPOSIT:
            std::cout << ++displayId << " : " << "Deposit money" << '\n';
            break;
        case bankActions::WIDTHDRAW:
            std::cout << ++displayId << " : " << "Withdraw money" << '\n';
            break;
        case bankActions::TRANSFER:
            std::cout << ++displayId << " : " << "Transfer money" << '\n';
            break;
        case bankActions::GET_ACCOUNT_INFO:
            std::cout << ++displayId << " : " << "Get account info" << '\n';
            break;
        case bankActions::GET_USER_INFO:
            std::cout << ++displayId << " : " << "Get user info" << '\n';
            break;
        case bankActions::EXIT:
            std::cout << ++displayId << " : " << "Exit menu" << '\n';
            break;
        }
    }
}

int main()
{
        int action;

    while (true)
    {
        displayMenu();
        std::cin >> action;
    }
    return 0;
}