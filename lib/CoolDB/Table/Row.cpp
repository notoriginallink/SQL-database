#include "Row.h"

#include <iomanip>
#include <utility>


Condition::Condition() : not_(false), column_(0), data_(0), op_(0) {}

Row::Row(std::vector<tablevar> il)  : items_(std::move(il)) {}

Row::Row(const size_t& n) {
    items_.resize(n, Null());
}

tablevar& Row::operator[](size_t index) { return items_[index]; }
const tablevar& Row::operator[](size_t index) const {return items_[index]; }

void Row::print() const {
    for (const auto& x : items_) {
        try {
            std::cout << std::setw(kPrintWidth) << std::get<int32_t>(x) << kDelimiter;
        } catch (const std::bad_variant_access& e) {
            try {
                std::cout << std::setw(kPrintWidth) << std::get<double>(x) << kDelimiter;
            } catch (const std::bad_variant_access& e) {
                try {
                    std::cout << std::setw(kPrintWidth) << std::get<float>(x) << kDelimiter;
                } catch (const std::bad_variant_access& e) {
                    try {
                        std::cout << std::setw(kPrintWidth) << std::get<bool>(x) << kDelimiter;
                    } catch (const std::bad_variant_access& e) {
                        try {
                            std::cout << std::setw(kPrintWidth) << std::get<std::string>(x) << kDelimiter;
                        } catch (const std::bad_variant_access& e) {
                            std::cout << std::setw(kPrintWidth) << std::get<Null>(x) << kDelimiter;
                        }
                    }
                }
            }
        }
    }
    std::cout << std::endl;
}

void Row::push_back(const tablevar &n) { items_.push_back(n); }

bool Row::check_condition(size_t column_index, const std::string& operation, const tablevar& var) const {
    return check_condition(column_index, kOperationsID[operation], var);
}

bool Row::check_condition(size_t column_index, const uint8_t& operation, const tablevar& var) const {
    switch (operation) {
        case 0:
            return items_[column_index] == var;
        case 1:
            return items_[column_index] != var;
        case 2:
            return items_[column_index] > var;
        case 3:
            return items_[column_index] >= var;
        case 4:
            return items_[column_index] < var;
        case 5:
            return items_[column_index] <= var;
        default:
            return false;
    }
}

bool Row::check_condition_list(const std::vector<std::forward_list<Condition>>& check_list) const {
    for (const auto& conditions : check_list) {
        bool flag = true;
        for (const auto& condition : conditions) {
            if (condition.not_)
                flag = !check_condition(condition.column_, condition.op_, condition.data_);
            else
                flag = check_condition(condition.column_, condition.op_, condition.data_);
            if (!flag)
                break;
        }
        if (flag)
            return true;
    }

    return false;
}

Row Row::align_to(size_t n) {
    Row ret = *this;
    if (n > ret.items_.size()) {
        for (size_t i = 0; i < n - ret.items_.size(); ++i) {
            ret.push_back(Null());
        }
    }

    return ret;
}

tablevar string_to_tablevar(const std::string& s, const kTypeId& type) {
    switch (type) {
        case kTypeId::INT:
            return tablevar{std::stoi(s)};
        case kTypeId::FLOAT:
            return tablevar{std::stof(s)};
        case kTypeId::DOUBLE:
            return tablevar{std::stod(s)};
        case kTypeId::BOOL:
            return tablevar{s == "true"};
        case kTypeId::STRING:
            return tablevar{s};
        default:
            return tablevar{Null()};
    }
}
