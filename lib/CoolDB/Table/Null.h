#pragma once

#include <iostream>

class Null {
public:
    Null() = default;

    bool operator==(const Null& other) const;
    bool operator!=(const Null& other) const;

    template<class T>
    bool operator==(const T& other) const { return false; }

    template<class T>
    bool operator!=(const T& other) const { return true; }

    template<class T>
    bool operator>(const T& other) const {return false; }

    template<class T>
    bool operator>=(const T& other) const {return false; }

    template<class T>
    bool operator<(const T& other) const {return false; }

    template<class T>
    bool operator<=(const T& other) const {return false; }

    friend std::ostream& operator<<(std::ostream& os, const Null& n);
};

