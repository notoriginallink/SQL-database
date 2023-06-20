#include "CoolDB.h"

#include <fstream>
#include <list>

// Query regexps
const std::regex kCreateReg(R"(\s*CREATE\s+TABLE\s+\w+\s+\((\s*\w+\s+(int|float|double|bool|varchar(\([0-9]+\))?)\s*,)+\s*PRIMARY\s+KEY\s*\((\s*\w+,?)+\)\s*\);)");
const std::regex kInsertReg(R"(\s*INSERT\s+INTO\s+\w+\s+(\(\s*(\w+,\s*)*\w+\s*\))?\s*VALUES\s*(\(\s*((\d+\.?\d*,\s*)|('[^']*',\s*))*((\s*\d+\.?\d*\s*)|('[^']*'\s*))\s*\),\s*)*(\(\s*((\d+\.?\d*,\s*)|('[^']*',\s*))*((\s*\d+\.?\d*\s*)|('[^']*'\s*))\s*\)\s*);)");
const std::regex kDropReg(R"(\s*DROP\s+TABLE\s+\w+;)");
const std::regex kUpdateReg(R"(\s*UPDATE\s+\w+\s+SET\s+\w+\s+=\s+(\w+|'[^']+')(\s+WHERE\s+((NOT)?\s*(\w+\s*(=|!=|>|<|>=|<=){1}\s*([\d\.]+|'[^']+'))\s+(OR|AND){1}\s+)*((NOT)?\s*\w+\s*(=|!=|>|<|>=|<=){1}\s*([\d\.]+|'[^']+')))?;)");
const std::regex kDeleteReg(R"(\s*DELETE\s+FROM\s+\w+\s*(\s*WHERE\s+((NOT)?\s*(\w+\s*(=|!=|>|<|>=|<=){1}\s*([\d\.]+|'[^']+'))\s+(OR|AND){1}\s+)*((NOT)?\s*\w+\s*(=|!=|>|<|>=|<=){1}\s*([\d\.]+|'[^']+')))?;)");
const std::regex kSelectReg(R"(\s*SELECT\s+(\*|(\w+,\s*)*(\w+))\s+FROM\s+\w+\s*(\s+(INNER\s+|LEFT\s+|RIGHT\s+)?JOIN\s+\w+\s+ON\s+[\w\.]+\s+=\s+[\w\.]+\s*)?(\s+WHERE\s+((NOT)?\s*(\w+\s*(=|!=|>|<|>=|<=){1}\s*([\d\.]+|'[^']+'))\s+(OR|AND){1}\s+)*((NOT)?\s*\w+\s*(=|!=|>|<|>=|<=){1}\s*([\d\.]+|'[^']+')))?;)");

// tokenization regexps
const std::regex kSplit(R"(([\w]+)[\s,\(\)]*)");
const std::regex kSplitNumbers(R"(([\w\.]+)[\s,\(\)]*)");
const std::regex kSplitOperations(R"(([\w\.=!><\*]+)[\s,\(\)]*)");

// database commands
const std::string kCloseCommand = "@close";
const std::string kInfoCommand = "@info";
const std::regex kSaveCommand(R"(\s*@save\s+\w+\.[a-zA-z]+\s*)");
const std::regex kLoadCommand(R"(\s*@load\s+\w+\.[a-zA-z]+\s*)");

// .................DESTRUCTOR

CoolDB::~CoolDB() {
    for (Table* table : table_list_)
        delete table;
}

// .................FILES

void CoolDB::load_from_file(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open())
        throw std::runtime_error{"Can't open the file"};
    int number_of_tables;
    file >> number_of_tables;
    for (int i = 0; i < number_of_tables; ++i) {
        std::string name;
        size_t number_of_columns;
        file >> name;
        auto table = new Table(name);

        file >> number_of_columns;
        for (size_t j = 0; j < number_of_columns; ++j) {
            std::string type;
            std::string column_name;
            file >> type >> column_name;
            table->add_column(type, column_name);
        }

        size_t primary_key_columns;
        file >> primary_key_columns;
        for (size_t j = 0; j < primary_key_columns; ++j) {
            size_t col_num;
            file >> col_num;
            table->add_primary_index(col_num);
        }

        size_t number_of_rows;
        file >> number_of_rows;
        for (size_t j = 0; j < number_of_rows; ++j) {
            Row ins;
            for (size_t k = 0; k < number_of_columns; ++k) {
                std::string temp;
                file >> temp;
                if (temp != "NULL")
                    switch (table->get_types()[k]) {
                        case kTypeId::INT:
                            ins.push_back(string_to_tablevar(temp, kTypeId::INT));
                            break;
                        case kTypeId::FLOAT:
                            ins.push_back(string_to_tablevar(temp, kTypeId::FLOAT));
                            break;
                        case kTypeId::DOUBLE:
                            ins.push_back(string_to_tablevar(temp, kTypeId::DOUBLE));
                            break;
                        case kTypeId::BOOL:
                            ins.push_back(string_to_tablevar(temp, kTypeId::BOOL));
                            break;
                        case kTypeId::STRING:
                            ins.push_back(std::string{temp.cbegin() + 1, temp.cend() - 1});
                            break;
                        case kTypeId::NULLOBJ:
                            ins.push_back(Null());
                    }
                else
                    ins.push_back(Null());
            }
            table->insert_row(ins);
        }
        table_list_.push_back(table);
    }
    file.close();
}

void CoolDB::save_to_file(const std::string& path) {
    std::ofstream file(path);
    if (!file.is_open())
        throw std::runtime_error{"Can't open the file"};

    file << table_list_.size() << '\n';
    for (const auto& table : table_list_) {
        size_t number_of_columns = table->size().first;
        size_t number_of_rows = table->size().second;
        file << table->name() << '\n' << table->size().first << '\n';

        for (size_t i = 0; i < number_of_columns; ++i) {
            switch (table->get_types()[i]) {
                case kTypeId::INT:
                    file << "int ";
                    break;
                case kTypeId::FLOAT:
                    file << "float ";
                    break;
                case kTypeId::DOUBLE:
                    file << "double ";
                    break;
                case kTypeId::BOOL:
                    file << "bool ";
                    break;
                case kTypeId::STRING:
                    file << "varchar ";
                    break;
                case kTypeId::NULLOBJ:
                    file << "null ";
                    break;
            }
            file << table->get_names()[i] << '\n';
        }
        file << table->get_primary_keys().size();
        for (size_t x : table->get_primary_keys())
            file << ' ' << x;
        file << '\n';
        file << number_of_rows << '\n';
        for (const auto& row : table->get_rows()) {
            for (int i = 0; i < table->size().first; ++i) {
                try {
                    file << std::get<int>(row[i]) << ' ';
                } catch (const std::bad_variant_access& e) {
                    try {
                        file << std::get<double>(row[i]) << ' ';
                    } catch (const std::bad_variant_access& e) {
                        try {
                            file << std::get<float>(row[i]) << ' ';
                        } catch (const std::bad_variant_access& e) {
                            try {
                                file << std::get<bool>(row[i]) << ' ';
                            } catch (const std::bad_variant_access& e) {
                                try {
                                    file << std::get<Null>(row[i]) << ' ';
                                } catch (const std::bad_variant_access& e) {
                                    file << '\'' << std::get<std::string>(row[i]) << '\'' << ' ';
                                }
                            }
                        }
                    }
                }
            }
            file << '\n';
        }
    }
}

// ............OTHER

Table* CoolDB::find_table(const std::string& name) {
    Table* ret = nullptr;
    for (Table* table : table_list_) {
        if (table->name() == name) {
            ret = table;
            break;
        }
    }

    return ret;
}

std::vector<std::string> CoolDB::tokenize(const std::string& line, const std::regex& reg) {
    return std::vector<std::string>{std::sregex_token_iterator(std::cbegin(line), std::cend(line), reg, 1),
                                    std::sregex_token_iterator()};
}

std::vector<std::forward_list<Condition>> CoolDB::generate_check_list(const std::vector<std::string>& tokens,
                                                                      size_t start_index, Table* table) const {
    std::vector<std::forward_list<Condition>> check_list(1);
    const std::vector<kTypeId>& column_types = table->get_types();
    size_t& i = start_index;
    for (; i + 2 < tokens.size(); i += 3) {
        Condition cond;
        if (tokens[i] == "NOT") {
            cond.not_ = true;
            ++i;
        }
        size_t ind = table->get_index_by_name(tokens[i]);
        cond.column_ = ind;
        cond.op_ = kOperationsID[tokens[i + 1]];
        try {
            cond.data_ = string_to_tablevar(tokens[i + 2], column_types[ind]);
        } catch (const std::exception& e) {
            std::cout << e.what() << std::endl;
            return std::vector<std::forward_list<Condition>>{};
        }
        check_list[check_list.size() - 1].push_front(cond);
        if (i + 3 < tokens.size()) {
            if (tokens[i + 3] == "OR")
                check_list.resize(check_list.size() + 1);
            ++i;
        }
    }
    return check_list;
}

// ............QUERIES

void CoolDB::create_query(const std::string& line) {
    auto tokens = tokenize(line, kSplitNumbers);
    Table* new_table = find_table(tokens[2]);
    if (new_table == nullptr) {
        new_table = new Table(tokens[2]);
        size_t i = 3;
        for (; i < tokens.size() - 1 && tokens[i] != "PRIMARY"; i += 2) {
            new_table->add_column(tokens[i + 1], tokens[i]);
            if (tokens[i + 1] == "varchar" &&
                std::all_of(tokens[i + 2].cbegin(), tokens[i + 2].cend(), [](char c) { return std::isdigit(c); }))
                size_t len = std::stoi(tokens[i++ + 2]);
        }
        i += 2;
        for (; i < tokens.size(); ++i)
            new_table->add_primary_index(tokens[i]);
        table_list_.push_back(new_table);
    } else
        std::cout << "@Cant create table, already there's table with this name" << std::endl;
}

void CoolDB::insert_query(const std::string& line) {
    auto tokens = tokenize(line, kSplitNumbers);
    Table* table = find_table(tokens[2]);
    if (table == nullptr)
        std::cout << "@Table " << tokens[2] << " not found\n";
    else {
        size_t i = 3;
        if (tokens[i] == "VALUES") {
            const size_t rows_to_insert = std::count(line.begin(), line.end(), '(');
            const size_t elements_to_insert = table->size().first;
            if (rows_to_insert * elements_to_insert != (tokens.size() - i - 1)) {
                std::cout << "@Column count doesn't match value count" << std::endl;
                return;
            }
            const std::vector<kTypeId>& column_types = table->get_types();
            ++i;
            for (; i < tokens.size(); i += elements_to_insert) {
                Row ins(elements_to_insert);
                for (size_t j = 0; j < elements_to_insert; ++j)
                    ins[j] = string_to_tablevar(tokens[i + j], column_types[j]);
                try {
                    table->insert_row(ins);
                } catch (const std::runtime_error& e) {
                    std::cout << e.what() << std::endl;
                    return;
                }
            }
        } else {
            std::vector<size_t> insert_column_indexes;
            for (; tokens[i] != "VALUES"; ++i) {
                size_t ind = table->get_index_by_name(tokens[i]);
                if (ind == -1) {
                    std::cout << "Wrong column name\n";
                    return;
                } else
                    insert_column_indexes.push_back(ind);
            }
            i++;
            const size_t elements_to_insert = insert_column_indexes.size();
            const std::vector<kTypeId>& column_types = table->get_types();
            for (; i < tokens.size(); i += elements_to_insert) {
                Row ins(table->size().first);
                for (size_t j = 0; j < elements_to_insert; ++j)
                    ins[insert_column_indexes[j]] = string_to_tablevar(tokens[i + j], column_types[insert_column_indexes[j]]);
                try {
                    table->insert_row(ins);
                } catch (const std::runtime_error& e) {
                    std::cout << e.what() << std::endl;
                    return;
                }
            }
        }
    }
}

void CoolDB::drop_query(const std::string& line) {
    auto tokens = tokenize(line, kSplit);
    Table* table = find_table(tokens[2]);
    if (table == nullptr) {
        std::cout << "@Table " << tokens[2] << " not found\n";
        return;
    }
    table_list_.erase(std::find(table_list_.begin(), table_list_.end(), table));
    delete table;
}

void CoolDB::update_query(const std::string& line) {
    auto tokens = tokenize(line, kSplitOperations);
    Table* table = find_table(tokens[1]);
    if (table == nullptr) {
        std::cout << "@Table " << tokens[1] << " not found\n";
        return;
    }
    size_t column_index = table->get_index_by_name(tokens[3]);
    tablevar new_data = string_to_tablevar(tokens[5], table->get_types()[column_index]);
    size_t i = 6;

    if (i >= tokens.size()) {
        for (size_t j = 0; j < table->size().second; ++j)
            table->update(j, column_index, new_data);
        return;
    }
    ++i;
    std::vector<std::forward_list<Condition>> check_list = generate_check_list(tokens, i, table);

    for (size_t k = 0; k < table->size().second; ++k)
        if (table->get_rows()[k].check_condition_list(check_list))
            table->update(k, column_index, new_data);
}

void CoolDB::delete_query(const std::string& line) {
    auto tokens = tokenize(line, kSplitOperations);
    Table* table = find_table(tokens[2]);
    if (table == nullptr) {
        std::cout << "@Table " << tokens[2] << " not found" << std::endl;
        return;
    }
    if (tokens.size() <= 3) {
        table->clear_table();
        return;
    } else {
        size_t i = 4;
        const size_t n = table->size().second;

        auto check_list = generate_check_list(tokens, i, table);
        std::cout << check_list.size() << '\n';
        for (int64_t j = n - 1; j >= 0; --j) {
            if (table->get_rows()[j].check_condition_list(check_list))
                table->delete_row(j);

        }
    }
}

void CoolDB::select_query(const std::string& line) {
    auto tokens = tokenize(line, kSplitOperations);
    std::vector<std::string> column_names;
    std::vector<size_t> column_indexes;
    Table* table;
    bool new_table = false;
    bool all_cols = false;
    size_t i = 1; // list of columns index
    if (tokens[i] == "*") {
        all_cols = true;
        ++i;
    } else {
        for (; tokens[i] != "FROM"; ++i)
            column_names.push_back(tokens[i]);
    }
    std::string table_name = tokens[i + 1];
    i += 2; // next token after FROM
    table = find_table(table_name);
    if (table == nullptr) {
        std::cout << "@Table " << table_name << " not found" << std::endl;
        return;
    }
    if (!(i >= tokens.size() || tokens[i] == "WHERE")) {
        size_t join_id = i;
        if (tokens[i] == "JOIN")
            ++i; // name of the second table index
        else
            i += 2; // name of the second table index
        std::string join_name = tokens[i];
        Table* join_table = find_table(join_name);
        if (join_table == nullptr) {
            std::cout << "@Table " << join_name << " not found" << std::endl;
            return;
        }
        i += 2; // first join-column index
        size_t ind[2];
        for (size_t k = 0; k < 2; ++k) {
            auto temp_tokens = tokenize(tokens[i + k * 2], kSplit);
            if (temp_tokens[0] == table_name) {
                ind[0] = table->get_index_by_name(temp_tokens[1]);
                if (ind[0] == -1) {
                    std::cout << "@Column " << temp_tokens[1] << " not found" << std::endl;
                    return;
                }
            } else if (temp_tokens[0] == join_name) {
                ind[1] = join_table->get_index_by_name(temp_tokens[1]);
                if (ind[1] == -1) {
                    std::cout << "@Column " << temp_tokens[1] << " not found" << std::endl;
                    return;
                }
            } else {
                std::cout << "@Table " << temp_tokens[0] << " not found" << std::endl;
                return;
            }
        }
        i += 3; // after second join-column index
        if (tokens[join_id] == "JOIN" || tokens[join_id] == "INNER")
            table = table->inner_join(join_table, ind[0], ind[1]);
        else if (tokens[join_id] == "LEFT")
            table = table->left_join(join_table, ind[0], ind[1]);
        else
            table = table->right_join(join_table, ind[0], ind[1]);
        new_table = true;
    }

    if (all_cols) {
        for (size_t k = 0; k < table->size().first; ++k)
            column_indexes.push_back(k);
    } else {
        for (const auto& name : column_names) {
            size_t col_ind = table->get_index_by_name(name);
            if (col_ind == static_cast<size_t>(-1)) {
                std::cout << "@Column " << name << " not found" << std::endl;
                return;
            }
            column_indexes.push_back(col_ind);
        }
    }

    if (i < tokens.size()) {
        auto check_list = generate_check_list(tokens, i + 1, table);
        if (new_table) {
            Table* temp = table->find(check_list);
            std::swap(temp, table);
            delete temp;
        } else {
            table = table->find(check_list);
            new_table = true;
        }
    }

    Table* result = table->select(column_indexes);
    if (new_table)
        delete table;
    result->print();
    delete result;
}

void CoolDB::start_console() {
    std::string line;
    while (std::getline(std::cin, line)) {
        if (line == kCloseCommand)
            break;
        else if (std::regex_match(line, kCreateReg))
            create_query(line);
        else if (std::regex_match(line, kInsertReg))
            insert_query(line);
        else if (std::regex_match(line, kDropReg))
            drop_query(line);
        else if (std::regex_match(line, kUpdateReg))
            update_query(line);
        else if (std::regex_match(line, kDeleteReg))
            delete_query(line);
        else if (std::regex_match(line, kSelectReg))
            select_query(line);
        else if (line == kInfoCommand) {
            std::cout << "Number of tables: " << table_list_.size() << std::endl;
            for (size_t i = 0; i < table_list_.size(); ++i) {
                std::cout << "------------TABLE " << i + 1 << ":\n";
                table_list_[i]->print();
            }
        } else if (std::regex_match(line, kSaveCommand)) {
            try {
                save_to_file("../Data/"+ tokenize(line, kSplitNumbers)[1]);
            } catch (const std::exception& e) {
                std::cout << '@' << e.what() << '\n';
            }
        } else if (std::regex_match(line, kLoadCommand)) {
            try {
                load_from_file("../Data/"+ tokenize(line, kSplitNumbers)[1]);
            } catch (const std::exception& e) {
                std::cout << '@' << e.what() << '\n';
            }
        }
            
        else
            std::cout << "@Wrong syntax" << std::endl;
    }
}
