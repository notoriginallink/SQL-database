#include "Varchar.h"

// ..............Varchar...............

Varchar::Varchar(const size_t& size, const std::string & s) : max_size_(size) {
    if (s.length() <= max_size_)
        s_ = s;
    else
        s_.assign(s.begin(), s.begin() + max_size_);
}

//Varchar& Varchar::operator=(const std::string& other) {}

Varchar& Varchar::operator=(const Varchar& other) {
    if (other.str().length() <= max_size_)
        s_ = other.str();
    else {
        std::string temp = other.str();
        s_.assign(temp.begin(), temp.begin() + max_size_);
    }

}

bool Varchar::operator==(const Varchar& other) const { return s_ == other.s_; }
bool Varchar::operator!=(const Varchar& other) const { return !(*this == other); }

std::string Varchar::str() const { return s_; }