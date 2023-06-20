#pragma once

#include "Table/Table.h"

#include <regex>

class CoolDB final {
private:
    std::vector<Table*> table_list_;

    // FILES
    void save_to_file(const std::string& path);
    void load_from_file(const std::string& path);

    // Queries
    void create_query(const std::string& line);
    void insert_query(const std::string& line);
    void drop_query(const std::string& line);
    void update_query(const std::string& line);
    void delete_query(const std::string& line);
    void select_query(const std::string& line);

    // OTHER
    Table* find_table(const std::string& name);
    std::vector<std::string> tokenize(const std::string& line, const std::regex& reg);
    std::vector<std::forward_list<Condition>> generate_check_list(const std::vector<std::string>& tokens, size_t start_index, Table* table) const;
public:
    CoolDB() = default;
    ~CoolDB();
    void start_console();
};
