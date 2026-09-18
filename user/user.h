#include <string>
#pragma once

class User
{
private:
    int id;
    static int nextId;
    std::string name;

public:
    User(std::string p_name) : name{p_name}
    {
        id = ++nextId;
    }

    int getId()
    {
        return id;
    }

    std::string getName()
    {
        return name;
    }
};

User addUser();