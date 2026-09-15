#include <string>

class User
{
private:
    static int id;
    std::string name;

public:
    User(std::string p_name) : name{p_name}
    {
    }
};