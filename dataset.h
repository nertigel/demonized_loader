#pragma once
#include <string>

struct Product {
    std::string name;
    std::string status;
    std::string expiry;
};

struct ChangeLog {
    std::string type;
    std::string description;
    std::string date;
};