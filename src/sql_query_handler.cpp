#include "sql_query_handler.h"

/**
 * @brief Adds image's name, its contours' numbers and names to the database.
 *
 * @param conn Connection to the database.
 * @param img_name Image's name.
 * @param contours_to_add Contours' numbers and names.
 */
void addContoursToDb(
    pqxx::connection &conn, const std::string &img_name,
    const std::unordered_map<int, std::string> &contours_to_add) {
  // pqxx allows adding an array of data by passing std::vector as a query
  // parameter. For that reason std::unordered_set is being split.
  std::vector<int> contour_numbers{};
  std::vector<std::string> contour_names{};
  for (const auto &elem : contours_to_add) {
    contour_numbers.push_back(elem.first);
    contour_names.push_back(elem.second);
  }

  pqxx::work work{conn};
  std::string query{"SELECT * FROM contours WHERE image_name LIKE $1"};
  // If the image does NOT exist in the database:
  if (work.exec_params(query, img_name).size() == 0) {
    if (contour_numbers.empty()) {
      return;
    } else {
      query =
          "INSERT INTO contours (image_name, contour_numbers, contour_names) "
          "VALUES ($1, $2, $3)";
    }
  } else {
    if (contour_numbers.empty()) {
      query = "DELETE FROM contours WHERE image_name LIKE $1";
    } else {
      query = "UPDATE contours "
              "SET contour_numbers = $2, contour_names = $3 "
              "WHERE image_name LIKE $1";
    }
  }

  try {
    // DELETE query requires just 1 parameter
    // whereas INSERT and UPDATE queries need 3.
    if (contour_numbers.empty()) {
      work.exec_params(query, img_name);
    } else {
      work.exec_params(query, img_name, contour_numbers, contour_names);
    }
    work.commit();
  } catch (...) {
    work.abort();
  }
}

/**
 * @brief Gets image's saved contours' numbers and names.
 *
 * @param conn Connection to the database.
 * @param img_name Image's name.
 * @return An array of saved contours' numbers and names.
 */
std::vector<std::pair<int, std::string>>
getContoursFromDb(pqxx::connection &conn, const std::string &img_name) {
  pqxx::work work{conn};
  std::string query{"SELECT contour_numbers, contour_names FROM contours "
                    "WHERE image_name LIKE $1"};
  pqxx::result res{};
  try {
    res = work.exec_params(query, img_name);
    work.commit();
  } catch (...) {
    work.abort();
  }

  std::vector<std::pair<int, std::string>> added_contours{};
  if (!res.empty()) {
    // Element type cannot be deduced so it needs to be explicitly stated in the
    // function call
    pqxx::array<int> num_array = res[0][0].as_sql_array<int>();
    pqxx::array<std::string> name_array = res[0][1].as_sql_array<std::string>();

    for (size_t i = 0; i != num_array.size(); ++i) {
      added_contours.push_back({num_array[i], name_array[i]});
    }
  }

  return added_contours;
}
