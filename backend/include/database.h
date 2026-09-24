#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class Database
{
public:
  static Database &getInstance();

  // Initialize database
  bool init(const std::string &connectionName = "student_result_system");

  // Create tables
  void createTables();

  // Student operations
  json getStudent(const std::string &roll);
  json getStudentById(int id);
  json getAllStudents();
  bool addStudent(const json &studentData);
  bool updateStudent(const std::string &roll, const json &data);
  bool deleteStudent(const std::string &roll);

  // Faculty operations
  json getFaculty(const std::string &id);
  json getAllFaculty();
  bool addFaculty(const json &facultyData);
  bool updateFaculty(const std::string &id, const json &data);

  // Notification operations
  bool addNotification(const std::string &recipientType, const std::string &recipientId, const std::string &title, const std::string &message);
  json getNotifications(const std::string &recipientType, const std::string &recipientId);
  bool markNotificationRead(int id);
  bool markAllNotificationsRead(const std::string &recipientType, const std::string &recipientId);

  // Marks operations
  json getMarks(const std::string &studentRoll, int semester = 0);
  json getAvailableSemesters(const std::string &studentRoll);
  bool addMarks(const std::string &studentRoll, const json &marksData, int semester = 0);
  bool updateMarks(const std::string &studentRoll, const json &marksData, int semester = 0);

  // Results operations
  json getResult(const std::string &studentRoll, int semester = 0);
  bool publishResult(const std::string &studentRoll, const json &resultData, int semester = 0);
  json getAllResults();

  // Authentication
  bool verifyStudentPassword(const std::string &roll, const std::string &password);
  bool verifyFacultyPassword(const std::string &id, const std::string &password);
  bool updateStudentPassword(const std::string &roll, const std::string &password);
  bool updateFacultyPassword(const std::string &id, const std::string &password);

private:
  Database() = default;
  ~Database();

  void *dbHandle = nullptr; // MYSQL handle (void* keeps the client header out of this API)
  bool connected = false;
  mutable std::recursive_mutex dbMutex; // serializes access to the single shared MySQL connection across request threads

  // Helper methods
  std::string escapeSQL(const std::string &input);
  json queryToJson(const std::string &query);
};

#endif // DATABASE_H