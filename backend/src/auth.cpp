#include "auth.h"
#include "database.h"
#include <chrono>
#include <ctime>
#include <sstream>
#include <algorithm>

std::string Auth::generateToken(const json& payload) {
  // Simple token generation (in production, use JWT)
  auto now = std::chrono::system_clock::now();
  auto time = std::chrono::system_clock::to_time_t(now);
  
  std::stringstream ss;
  ss << payload.dump() << "|" << time;
  
  return ss.str();
}

json Auth::validateToken(const std::string& token) {
  // Simple token validation (in production, use JWT)
  try {
    size_t pos = token.rfind('|');
    if (pos == std::string::npos) {
      return json{{"error", "Invalid token"}};
    }
    
    std::string payload_str = token.substr(0, pos);
    json payload = json::parse(payload_str);
    
    return payload;
  } catch (...) {
    return json{{"error", "Token validation failed"}};
  }
}

bool Auth::isTokenValid(const std::string& token) {
  json result = validateToken(token);
  return !result.contains("error");
}

json Auth::loginStudent(const std::string& roll, const std::string& password) {
  Database& db = Database::getInstance();
  
  if (!db.verifyStudentPassword(roll, password)) {
    return json{{"error", "Invalid credentials"}};
  }
  
  json student = db.getStudent(roll);
  
  json payload = {
    {"role", "student"},
    {"roll", roll},
    {"id", student["id"]},
    {"name", student["name"]},
    {"branch", student["branch"]}
  };
  
  std::string token = generateToken(payload);
  
  return json{
    {"success", true},
    {"token", token},
    {"student", student}
  };
}

json Auth::loginFaculty(const std::string& id, const std::string& password) {
  Database& db = Database::getInstance();
  
  if (!db.verifyFacultyPassword(id, password)) {
    return json{{"error", "Invalid credentials"}};
  }
  
  json faculty = db.getFaculty(id);
  
  json payload = {
    {"role", "faculty"},
    {"id", id},
    {"name", faculty["name"]},
    {"department", faculty["department"]}
  };
  
  std::string token = generateToken(payload);
  
  return json{
    {"success", true},
    {"token", token},
    {"faculty", faculty}
  };
}

std::string Auth::hashPassword(const std::string& password) {
  // Simple hash (in production, use bcrypt)
  // This is just a placeholder - use a real password hashing library
  return password;
}

bool Auth::verifyPassword(const std::string& password, const std::string& hash) {
  // Simple verification (in production, use bcrypt)
  return password == hash;
}
