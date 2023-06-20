#pragma once

#include "Row.h"

#include <deque>
#include <unordered_set>

class Table final {
private:
    std::deque<Row> table_;
    std::string name_;
    std::vector<std::string> column_names_;
    std::vector<kTypeId> column_types_;
    std::unordered_set<size_t> primary_key_indexes_;
public:
    explicit Table(const std::string& name);
    ~Table();

    // COPY
    explicit Table(const Table* other);
    void copy(const Table* other);

    // CREATE TABLE
    void add_column(const std::string& type, const std::string& name);
    void add_column(const kTypeId& type, const std::string& name);
    void add_primary_index(const size_t& index);
    void add_primary_index(const std::string& column_name);

    // INSERT INTO
    void insert_row(const std::vector<tablevar>& ins);
    void insert_row(const Row& ins);

    // UPDATE TABLE
    void update(size_t row_index, size_t column_index, const tablevar& new_data);

    // DELETE
    void delete_row(size_t row_index);
    void clear_table();
    void drop_table();

    // INFO
    const std::string& name() const;
    std::pair<size_t, size_t> size() const;
    void rename(const std::string& new_name);
    const std::vector<kTypeId>& get_types() const;
    const std::vector<std::string>& get_names() const;
    const std::unordered_set<size_t>& get_primary_keys() const;
    const std::deque<Row>& get_rows() const;
    size_t get_index_by_name(const std::string& name) const;

    // SHOW TABLE
    void print() const;

    // FIND ROWS
    Table* find(size_t column_index, const std::string& operation, const tablevar& var) const;
    Table* find(const std::vector<std::forward_list<Condition>>& check_list) const;

    // SELECT COLS
    Table* select(const std::vector<size_t>& column_indexes) const;

    // JOIN
    Table* inner_join(const Table* other, size_t ind1, size_t ind2) const;
    Table* left_join(const Table* other, size_t ind1, size_t ind2) const;
    Table* right_join(const Table* other, size_t ind1, size_t ind2) const;

};
