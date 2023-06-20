#include "Table.h"

#include <exception>
#include <iomanip>
#include <regex>

Table::Table(const std::string& name) : name_(name) {}

Table::~Table() { drop_table(); }

// ..................COPY

Table::Table(const Table* other)
        : name_(other->name_), column_names_(other->column_names_),
          column_types_(other->column_types_)
          {}

[[maybe_unused]]void Table::copy(const Table* other) { table_ = other->get_rows(); }

// ..................CREATE TABLE

void Table::add_column(const std::string &type, const std::string &name) {
    if (type == "int")
        column_types_.push_back(kTypeId::INT);
    else if (type == "float")
        column_types_.push_back(kTypeId::FLOAT);
    else if (type == "double")
        column_types_.push_back(kTypeId::DOUBLE);
    else if (type == "bool")
        column_types_.push_back(kTypeId::BOOL);
    else if (std::regex_match(type, std::regex("varchar(\\([0-9]+\\))*")))
        column_types_.push_back(kTypeId::STRING);
    else
        throw std::runtime_error{"Wrong type name"};
    column_names_.push_back(name);
}

void Table::add_column(const kTypeId& type, const std::string& name) {
    column_types_.push_back(type);
    column_names_.push_back(name);
}

void Table::add_primary_index(const size_t& index) { primary_key_indexes_.insert(index); }

void Table::add_primary_index(const std::string& column_name) {
    primary_key_indexes_.insert(get_index_by_name(column_name));
}

// ..............INSERT INTO

[[maybe_unused]]void Table::insert_row(const std::vector<tablevar>& v) {
    // check for unique primary keys
    Row ins(v);
    for (auto& row : table_) {
        bool fl = false;
        for (const auto& key_index : primary_key_indexes_)
            if (row[key_index] != ins[key_index])
                fl = true;
        if (!fl)
            throw std::runtime_error{"Already there's row with this primary key"};
    }

    // check for types
    for (size_t i = 0; i < size().first; ++i)
        if (v[i].index() != static_cast<int>(column_types_[i]) && v[i].index() != static_cast<int>(kTypeId::NULLOBJ))
            throw std::runtime_error{"Bad arguments order"};

    table_.emplace_back(ins.align_to(size().first));
}

void Table::insert_row(const Row& ins) {
    for (auto& row : table_) {
        bool fl = true;
        for (const auto& key_index : primary_key_indexes_)
            if (row[key_index] == ins[key_index])
                fl = false;
        if (!fl)
            throw std::runtime_error{"Already there's row with this primary key"};
    }

    for (int i = 0; i < size().first; ++i)
        if (ins[i].index() != static_cast<int>(column_types_[i]) && ins[i].index() != static_cast<int>(kTypeId::NULLOBJ))
            throw std::runtime_error{"Bad arguments order"};

    table_.emplace_back(ins);
}

// .................UPDATE

void Table::update(size_t row_index, size_t column_index, const tablevar& new_data) {
    if (static_cast<int>(column_types_[column_index]) != new_data.index()) {
        throw std::runtime_error{"Wrong type of new data"};
    } else {
        if (primary_key_indexes_.contains(column_index)) {
            for (Row& row : table_) {
                if (row[column_index] == new_data) {
                    if (primary_key_indexes_.size() == 1)
                        throw std::runtime_error{"Update failed, primary key repeats"};
                    bool fl = false;
                    for (size_t index : primary_key_indexes_) {
                        if (index == column_index) continue;
                        if (row[index] != table_[row_index][index]) {
                            fl = true;
                            break;
                        }
                    }
                    if (fl)
                        break;
                    else
                        throw std::runtime_error{"Update failed, primary key repeats"};

                }

            }
        }
        table_[row_index][column_index] = new_data;
    }
}

// ................DELETE

void Table::clear_table() {
    table_.clear();
}

void Table::drop_table() {
    table_.clear();
    column_names_.clear();
    column_types_.clear();
    primary_key_indexes_.clear();
}

void Table::delete_row(size_t row_index) {
    if (row_index >= table_.size())
        return;
    table_.erase(table_.begin() + row_index);
}

// ...............INFO

std::pair<size_t, size_t> Table::size() const { return std::make_pair(column_names_.size(), table_.size()); }

[[maybe_unused]]void Table::rename(const std::string& new_name) { name_ = new_name; }

const std::string& Table::name() const { return name_; }

const std::vector<kTypeId>& Table::get_types() const { return column_types_; }

const std::vector<std::string>& Table::get_names() const { return column_names_; }

const std::unordered_set<size_t>& Table::get_primary_keys() const { return primary_key_indexes_; }

const std::deque<Row>& Table::get_rows() const { return table_; }

size_t Table::get_index_by_name(const std::string& name) const {
    for (size_t i = 0; i < size().first; ++i)
        if (column_names_[i] == name)
            return i;
    return -1;
}

// ...................SHOW TABLE

void Table::print() const {
    std::cout << "Table: " << name_ << ", " << column_names_.size() << " cols " << table_.size() << " rows" << std::endl;
    for (const auto& s : column_names_)
        std::cout << std::setw(kPrintWidth) << s << '|';
    std::cout << std::endl;
    for (const auto& row : table_)
        row.print();

}

// ....................FIND ROWS

 [[maybe_unused]]Table* Table::find(size_t column_index, const std::string& operation, const tablevar& var) const {
    auto new_table = new Table(this);
    for (const Row& row : table_)
        if (row.check_condition(column_index, operation, var))
            new_table->insert_row(row);

    return new_table;
}

Table* Table::find(const std::vector<std::forward_list<Condition>>& check_list) const {
    auto new_table = new Table(this);
    for (const Row& row : table_)
        if (row.check_condition_list(check_list))
            new_table->insert_row(row);

    return new_table;
}

// ....................SELECT COLS

Table* Table::select(const std::vector<size_t>& column_indexes) const {
    auto new_table = new Table(name_);
    for (size_t ind : column_indexes) {
        new_table->column_types_.push_back(column_types_[ind]);
        new_table->column_names_.push_back(column_names_[ind]);
    }

    for (const Row& row : table_) {
        Row r;
        for (size_t ind : column_indexes)
            r.push_back(row[ind]);
        new_table->insert_row(r);
    }

    return new_table;
}

// ..........................JOIN

Table* Table::inner_join(const Table* other, size_t ind1, size_t ind2) const {
    auto new_table = new Table(this);
    for (size_t i = 0; i < other->size().first; ++i)
        new_table->add_column(other->get_types()[i], other->get_names()[i]);

    for (size_t i = 0; i < size().second; ++i) {
        Row ins = table_[i];
        for (size_t j = 0; j < other->size().second; ++j) {
            if (table_[i][ind1] == other->get_rows()[j][ind2]) {
                for (size_t k = 0; k < other->size().first; ++k) {
                    ins.push_back(other->get_rows()[j][k]);
                }
                new_table->insert_row(ins);
                ins = table_[i];
            }
        }
    }

    return new_table;
}

Table* Table::left_join(const Table* other, size_t ind1, size_t ind2) const {
    auto new_table = new Table(this);
    for (int i = 0; i < other->size().first; ++i)
        new_table->add_column(other->get_types()[i], other->get_names()[i]);

    for (size_t i = 0; i < size().second; ++i) {
        Row ins = table_[i];
        bool fl = false;
        for (size_t j = 0; j < other->size().second; ++j) {
            if (table_[i][ind1] == other->get_rows()[j][ind2]) {
                fl = true;
                for (size_t k = 0; k < other->size().first; ++k)
                    ins.push_back(other->get_rows()[j][k]);
                new_table->insert_row(ins);
                ins = table_[i];
            }
        }
        if (!fl) {
            for (size_t j = this->size().first; j < new_table->size().first; ++j)
                ins.push_back(Null());
            new_table->insert_row(ins);
        }
    }

    return new_table;
}

Table* Table::right_join(const Table* other, size_t ind1, size_t ind2) const {
    return other->left_join(this, ind2, ind1);
}
