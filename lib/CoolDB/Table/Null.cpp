#include "Null.h"

bool Null::operator==(const Null& other) const { return true; }
bool Null::operator!=(const Null& other) const { return false; }

std::ostream& operator<<(std::ostream& os, const Null& n) {
    os << "NULL";
    return os;
}