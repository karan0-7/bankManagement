#include "./user.h"
#include <iostream>
#include <string>

int User::nextId = 0;

User addUser()
{
    std::cout << "Please enter your name" << '\n';
    std::string name{};
    std::getline(std::cin, name);
    User user{name};
    return user;
}