#ifndef AUTH_H
#define AUTH_H

#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class Auth
{
public:
  // Token generation and validation
  static std::string generateToken(const json &payload);
  static json validateToken(const std::string &token);
  static bool isTokenValid(const std::string &token);

  // Login handlers
  static json loginStudent(const std::string &roll, const std::string &password);
  static json loginFaculty(const std::string &id, const std::string &password);

  // Password hashing (basic implementation, consider using bcrypt in production)
  static std::string hashPassword(const std::string &password);
  static bool verifyPassword(const std::string &password, const std::string &hash);

private:
  static constexpr const char *SECRET_KEY = "aditya-university-secret-key-change-in-production";
};

#endif // AUTH_H
