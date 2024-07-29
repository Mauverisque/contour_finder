#ifndef SQL_QUERY_HANDLER_H_
#define SQL_QUERY_HANDLER_H_

#include <pqxx/pqxx>

void addContoursToDb(
    pqxx::connection &conn, const std::string &image_name,
    const std::unordered_map<int, std::string> &contours_to_add);
std::vector<std::pair<int, std::string>>
getContoursFromDb(pqxx::connection &conn, const std::string &image_name);

#endif // SQL_QUERY_HANDLER_H_
