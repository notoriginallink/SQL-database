add_library(CoolDB CoolDB.h CoolDB.cpp)
add_subdirectory(Table)
target_link_libraries(CoolDB PRIVATE Table)