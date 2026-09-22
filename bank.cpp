#include <iostream>
#include "./user/user.h"
#include "./account/account.h"
#include "./bank.h"
#include <unordered_map>

std::vector<bankActions> bankOperations{
    bankActions::ADD_ACCOUNT,
    bankActions::ADD_USER,
    bankActions::DEPOSIT,
    bankActions::WIDTHDRAW,
    bankActions::TRANSFER,
    bankActions::GET_ACCOUNT_INFO,
    bankActions::GET_USER_INFO,
    bankActions::EXIT

};

std::unordered_map<int, Account> accounts;
std::unordered_map<int, User> users;

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

void accountCreate()
{
    Account account = addAccount();
    std::cout << "IDD " << account.getId() << '\n';
    accounts.insert({account.getId(), account});
}

void userCreate()
{
    User user = addUser();
    users.insert({user.getId(), user});
}

void accountDeposit()
{
    int accountId, depositAmount;
    std::cout << "Please enter your account id: " << '\n';
    std::cin >> accountId;
    std::cout << "Please enter amount to be deposited" << '\n';
    std::cin >> depositAmount;
    Account &account = accounts.at(accountId);
    std::cout << "despoti" << account.getId() << account.getBalance() << '\n';
    account.deposit(depositAmount);
    std::cout << "despoti" << account.getId() << account.getBalance() << '\n';
}

void accountWithdrawal()
{
    int accountId, withdrawalAmount;
    std::cout << "Please enter your account id: " << '\n';
    std::cin >> accountId;
    std::cout << "Please enter amount to be withdrawed" << '\n';
    std::cin >> withdrawalAmount;
    Account &account = accounts.at(accountId);
    account.widthdrawal(withdrawalAmount);
}

void getAccountInfo()
{
    int accountId;
    std::cout << "Please enter your account id: " << '\n';
    std::cin >> accountId;
    Account &account = accounts.at(accountId);

    std::cout << "Account balance: " << account.getBalance() << '\n';
    std::cout << "Transaction history: " << '\n';
    for (Transaction transaction : account.getTransactions())
    {
        std::cout << transaction.getOperation() << " " << transaction.getAmount() << " " << '\n';
    }
    std::cout << '\n';
}

void transferFunds()
{
    int sourceAccountId, destinationAccountId, transferAmount;
    std::cout << "Please enter the source account id." << '\n';
    std::cin >> sourceAccountId;
    std::cout << "Please enter the destination account id." << '\n';
    std::cin >> destinationAccountId;
    std::cout << "Please enter the transfer amount." << '\n';
    std::cin >> transferAmount;

    Account &sourceAccount = accounts.at(sourceAccountId);
    Account &destinationAccount = accounts.at(destinationAccountId);

    sourceAccount.widthdrawal(transferAmount);
    destinationAccount.deposit(transferAmount);
}

void processAction(int action)
{

    switch (action)
    {

    case static_cast<int>(bankActions::ADD_ACCOUNT):
        accountCreate();
        break;
    case static_cast<int>(bankActions::ADD_USER):
        userCreate();
        break;
    case static_cast<int>(bankActions::DEPOSIT):
        accountDeposit();
        break;
    case static_cast<int>(bankActions::WIDTHDRAW):
        accountWithdrawal();
        break;
    case static_cast<int>(bankActions::GET_ACCOUNT_INFO):
        getAccountInfo();
        break;

    default:
        break;
    }
}

int main()
{
    int action;

    while (true)
    {
        displayMenu();
        std::cin >> action;
        processAction(action);
    }
    return 0;
}