#include <iostream>

class Varchar final {
private:
    std::string s_;
    size_t max_size_;
public:
    Varchar(const size_t& size, const std::string & s);
    Varchar(const Varchar& other) = default;
//    Varchar& operator=(const std::string& other);
    Varchar& operator=(const Varchar& other);

    std::string str() const;

    bool operator==(const Varchar& other) const;
    bool operator!=(const Varchar& other) const;

};