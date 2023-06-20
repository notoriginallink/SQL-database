#pragma once

#include "Null.h"

#include <vector>
#include <variant>
#include <forward_list>
#include <unordered_map>
#include <string>

using tablevar = std::variant<int32_t, float, double, bool, std::string, Null>;

enum class kTypeId : uint8_t {INT = 0, FLOAT = 1, DOUBLE = 2, BOOL = 3, STRING = 4, NULLOBJ = 5};

static std::unordered_map<std::string, uint8_t> kOperationsID = {
        {"=", 0}, {"!=", 1}, {">", 2}, {">=", 3}, {"<", 4}, {"<=", 5}
};

const uint16_t kPrintWidth = 15;
const char kDelimiter = ' ';

struct Condition {
    bool not_;
    size_t column_;
    uint8_t op_;
    tablevar data_;
    Condition();
};

class Row final {
private:
    std::vector<tablevar> items_;
public:
    Row() = default;
    Row(std::vector<tablevar> il);
    explicit Row(const size_t& n);
    void print() const;

    tablevar& operator[](size_t index);
    const tablevar& operator[](size_t index) const;

    Row align_to(size_t n);

    void push_back(const tablevar& n);

    bool check_condition(size_t column_index, const std::string& operation, const tablevar& var) const;
    bool check_condition(size_t column_index, const uint8_t& operation, const tablevar& var) const;
    bool check_condition_list(const std::vector<std::forward_list<Condition>>& check_list) const;
};

tablevar string_to_tablevar(const std::string& s, const kTypeId& type);
