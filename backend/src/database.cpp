#include "database.h"

#include <mysql.h>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <cctype>

namespace
{

    MYSQL *connection(void *handle) { return static_cast<MYSQL *>(handle); }

    std::string envOr(const char *name, const char *fallback)
    {
        const char *value = std::getenv(name);
        return value && *value ? value : fallback;
    }

    std::string escape(MYSQL *db, const std::string &value)
    {
        std::string result(value.size() * 2 + 1, '\0');
        unsigned long length = mysql_real_escape_string(db, &result[0], value.c_str(), static_cast<unsigned long>(value.size()));
        result.resize(length);
        return result;
    }

    std::string text(const json &data, const char *key, const std::string &fallback = "")
    {
        return data.contains(key) && !data[key].is_null() ? data[key].get<std::string>() : fallback;
    }

    int number(const json &data, const char *key, int fallback = 0)
    {
        return data.contains(key) && !data[key].is_null() ? data[key].get<int>() : fallback;
    }

    bool execute(MYSQL *db, const std::string &query)
    {
        if (mysql_query(db, query.c_str()) != 0)
        {
            std::cerr << "MySQL error: " << mysql_error(db) << '\n';
            return false;
        }
        return true;
    }

    json rowToJson(MYSQL_RES *result, MYSQL_ROW row)
    {
        MYSQL_FIELD *fields = mysql_fetch_fields(result);
        json value = json::object();
        for (unsigned int index = 0; index < mysql_num_fields(result); ++index)
        {
            std::string name = fields[index].name;
            if (!row[index])
            {
                value[name] = nullptr;
                continue;
            }
            std::string raw = row[index];
            bool isNumeric = !raw.empty() && std::all_of(raw.begin(), raw.end(), [](unsigned char c)
                                                         { return std::isdigit(c); });
            if (name == "isRead")
                value[name] = raw == "1";
            else if ((name == "id" || name == "year" || name == "semester") && (raw.empty() || isNumeric))
                value[name] = raw.empty() ? 0 : std::stoi(raw);
            else if (name == "totalMarks" || name == "percentage")
                value[name] = raw.empty() ? 0.0 : std::stod(raw);
            else
                value[name] = raw;
        }
        return value;
    }

}

Database &Database::getInstance()
{
    static Database instance;
    return instance;
}

Database::~Database()
{
    if (dbHandle)
        mysql_close(connection(dbHandle));
}

bool Database::init(const std::string &connectionName)
{
    MYSQL *db = mysql_init(nullptr);
    if (!db)
        return false;

    const std::string host = envOr("SRMS_MYSQL_HOST", "127.0.0.1");
    const std::string user = envOr("SRMS_MYSQL_USER", "root");
    const std::string password = envOr("SRMS_MYSQL_PASSWORD", "");
    const std::string database = envOr("SRMS_MYSQL_DATABASE", connectionName.c_str());
    unsigned int port = static_cast<unsigned int>(std::stoul(envOr("SRMS_MYSQL_PORT", "3306")));

    bool reconnect = true;
    mysql_options(db, MYSQL_OPT_RECONNECT, &reconnect);

    if (!mysql_real_connect(db, host.c_str(), user.c_str(), password.c_str(), nullptr, port, nullptr, 0))
    {
        std::cerr << "MySQL connection failed: " << mysql_error(db) << '\n';
        mysql_close(db);
        return false;
    }
    if (!execute(db, "CREATE DATABASE IF NOT EXISTS `" + escape(db, database) + "`"))
    {
        mysql_close(db);
        return false;
    }
    if (mysql_select_db(db, database.c_str()) != 0)
    {
        std::cerr << "MySQL database selection failed: " << mysql_error(db) << '\n';
        mysql_close(db);
        return false;
    }

    dbHandle = db;
    connected = true;
    createTables();
    return true;
}

void Database::createTables()
{
    MYSQL *db = connection(dbHandle);
    const char *statements[] = {
        "CREATE TABLE IF NOT EXISTS students (id INT AUTO_INCREMENT PRIMARY KEY, roll VARCHAR(64) NOT NULL UNIQUE, password VARCHAR(255) NOT NULL, name VARCHAR(150) NOT NULL, email VARCHAR(255) NOT NULL, phone VARCHAR(32) NOT NULL, gender VARCHAR(32) NOT NULL, branch VARCHAR(100) NOT NULL, year INT NOT NULL, semester INT NOT NULL, academic_year VARCHAR(32) NOT NULL, dob DATE NULL, result_status VARCHAR(32) NOT NULL DEFAULT 'Draft') ENGINE=InnoDB",
        "CREATE TABLE IF NOT EXISTS marks (id INT AUTO_INCREMENT PRIMARY KEY, student_roll VARCHAR(64) NOT NULL, semester INT NOT NULL, subject VARCHAR(150) NOT NULL, ie1 INT NULL, ie2 INT NULL, sem INT NULL, credits DECIMAL(3,1) NULL, grade_points DECIMAL(3,1) NULL, UNIQUE KEY uq_mark (student_roll, semester, subject), CONSTRAINT fk_marks_student FOREIGN KEY (student_roll) REFERENCES students(roll) ON DELETE CASCADE) ENGINE=InnoDB",
        "CREATE TABLE IF NOT EXISTS results (id INT AUTO_INCREMENT PRIMARY KEY, student_roll VARCHAR(64) NOT NULL, semester INT NOT NULL, total_marks DECIMAL(10,2) NOT NULL, percentage DECIMAL(5,2) NOT NULL, grade VARCHAR(8) NOT NULL, status VARCHAR(32) NOT NULL DEFAULT 'Draft', published_date TIMESTAMP NULL DEFAULT NULL, UNIQUE KEY uq_result (student_roll, semester), CONSTRAINT fk_results_student FOREIGN KEY (student_roll) REFERENCES students(roll) ON DELETE CASCADE) ENGINE=InnoDB",
        "CREATE TABLE IF NOT EXISTS faculty (id VARCHAR(64) PRIMARY KEY, password VARCHAR(255) NOT NULL, name VARCHAR(150) NOT NULL, email VARCHAR(255) NOT NULL, department VARCHAR(150) NOT NULL) ENGINE=InnoDB",
        "CREATE TABLE IF NOT EXISTS notifications (id INT AUTO_INCREMENT PRIMARY KEY, recipient_type VARCHAR(16) NOT NULL, recipient_id VARCHAR(64) NOT NULL, title VARCHAR(150) NOT NULL, message TEXT NOT NULL, is_read TINYINT(1) NOT NULL DEFAULT 0, created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP, INDEX idx_notifications_recipient (recipient_type, recipient_id, id)) ENGINE=InnoDB"};
    for (const char *statement : statements)
        execute(db, statement);

    auto hasColumn = [db](const char *column)
    {
        std::string query = "SELECT COUNT(*) FROM information_schema.columns WHERE table_schema=DATABASE() AND table_name='marks' AND column_name='" + std::string(column) + "'";
        if (!execute(db, query))
            return false;
        MYSQL_RES *result = mysql_store_result(db);
        if (!result)
            return false;
        MYSQL_ROW row = mysql_fetch_row(result);
        bool exists = row && row[0] && std::stoi(row[0]) > 0;
        mysql_free_result(result);
        return exists;
    };

    if (hasColumn("internal"))
        execute(db, "ALTER TABLE marks DROP COLUMN internal");
    if (hasColumn("external"))
        execute(db, "ALTER TABLE marks DROP COLUMN external");
    if (!hasColumn("ie1"))
        execute(db, "ALTER TABLE marks ADD COLUMN ie1 INT NULL");
    if (!hasColumn("ie2"))
        execute(db, "ALTER TABLE marks ADD COLUMN ie2 INT NULL");
    if (!hasColumn("sem"))
        execute(db, "ALTER TABLE marks ADD COLUMN sem INT NULL");
    if (hasColumn("grade"))
        execute(db, "ALTER TABLE marks DROP COLUMN grade");
    if (!hasColumn("credits"))
        execute(db, "ALTER TABLE marks ADD COLUMN credits DECIMAL(3,1) NULL");
    if (!hasColumn("grade_points"))
        execute(db, "ALTER TABLE marks ADD COLUMN grade_points DECIMAL(3,1) NULL");
}

json Database::getStudent(const std::string &roll)
{
    std::lock_guard<std::recursive_mutex> lock(dbMutex);
    MYSQL *db = connection(dbHandle);
    std::string query = "SELECT id, roll, name, email, phone, gender, branch, year, semester, academic_year AS academicYear, dob, result_status AS resultStatus FROM students WHERE roll='" + escape(db, roll) + "' LIMIT 1";
    if (!execute(db, query))
        return json::object();
    MYSQL_RES *result = mysql_store_result(db);
    if (!result)
        return json::object();
    MYSQL_ROW row = mysql_fetch_row(result);
    json value = row ? rowToJson(result, row) : json::object();
    mysql_free_result(result);
    return value;
}

json Database::getStudentById(int id)
{
    std::lock_guard<std::recursive_mutex> lock(dbMutex);
    MYSQL *db = connection(dbHandle);
    std::string query = "SELECT id, roll, name, email, phone, gender, branch, year, semester, academic_year AS academicYear, dob, result_status AS resultStatus FROM students WHERE id=" + std::to_string(id) + " LIMIT 1";
    if (!execute(db, query))
        return json::object();
    MYSQL_RES *result = mysql_store_result(db);
    if (!result)
        return json::object();
    MYSQL_ROW row = mysql_fetch_row(result);
    json value = row ? rowToJson(result, row) : json::object();
    mysql_free_result(result);
    return value;
}

json Database::getAllStudents()
{
    std::lock_guard<std::recursive_mutex> lock(dbMutex);
    MYSQL *db = connection(dbHandle);
    if (!execute(db, "SELECT id, roll, name, email, phone, gender, branch, year, semester, academic_year AS academicYear, dob, result_status AS resultStatus FROM students ORDER BY id"))
        return json::array();
    MYSQL_RES *result = mysql_store_result(db);
    if (!result)
        return json::array();
    json values = json::array();
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result)))
        values.push_back(rowToJson(result, row));
    mysql_free_result(result);
    return values;
}

bool Database::addStudent(const json &data)
{
    if (text(data, "roll").empty() || text(data, "name").empty() || number(data, "semester") <= 0)
        return false;
    MYSQL *db = connection(dbHandle);
    std::string initialPassword = text(data, "password", "webcap");
    std::ostringstream query;
    query << "INSERT INTO students (roll,password,name,email,phone,gender,branch,year,semester,academic_year,dob,result_status) VALUES ('"
          << escape(db, text(data, "roll")) << "','" << escape(db, initialPassword) << "','" << escape(db, text(data, "name")) << "','"
          << escape(db, text(data, "email")) << "','" << escape(db, text(data, "phone")) << "','"
          << escape(db, text(data, "gender")) << "','" << escape(db, text(data, "branch")) << "',"
          << number(data, "year") << ',' << number(data, "semester") << ", '"
          << escape(db, text(data, "academicYear")) << "',"
          << (text(data, "dob").empty() ? "NULL" : "'" + escape(db, text(data, "dob")) + "'")
          << ",'Draft')";
    return execute(db, query.str());
}

bool Database::updateStudent(const std::string &roll, const json &data)
{
    std::lock_guard<std::recursive_mutex> lock(dbMutex);
    MYSQL *db = connection(dbHandle);
    std::ostringstream query;
    query << "UPDATE students SET name='" << escape(db, text(data, "name")) << "', email='"
          << escape(db, text(data, "email")) << "', phone='" << escape(db, text(data, "phone"))
          << "', gender='" << escape(db, text(data, "gender")) << "', branch='"
          << escape(db, text(data, "branch")) << "', year=" << number(data, "year")
          << ", semester=" << number(data, "semester") << ", academic_year='"
          << escape(db, text(data, "academicYear")) << "', dob="
          << (text(data, "dob").empty() ? "NULL" : "'" + escape(db, text(data, "dob")) + "'")
          << " WHERE roll='" << escape(db, roll) << "'";
    return execute(db, query.str()) && mysql_affected_rows(db) > 0;
}

bool Database::deleteStudent(const std::string &roll)
{
    std::lock_guard<std::recursive_mutex> lock(dbMutex);
    MYSQL *db = connection(dbHandle);
    return execute(db, "DELETE FROM students WHERE roll='" + escape(db, roll) + "'") && mysql_affected_rows(db) > 0;
}

json Database::getFaculty(const std::string &id)
{
    std::lock_guard<std::recursive_mutex> lock(dbMutex);
    MYSQL *db = connection(dbHandle);
    if (!execute(db, "SELECT id,name,email,department FROM faculty WHERE id='" + escape(db, id) + "' LIMIT 1"))
        return json::object();
    MYSQL_RES *result = mysql_store_result(db);
    if (!result)
        return json::object();
    MYSQL_ROW row = mysql_fetch_row(result);
    json value = row ? rowToJson(result, row) : json::object();
    mysql_free_result(result);
    return value;
}

json Database::getAllFaculty()
{
    std::lock_guard<std::recursive_mutex> lock(dbMutex);
    MYSQL *db = connection(dbHandle);
    if (!execute(db, "SELECT id,name,email,department FROM faculty ORDER BY id"))
        return json::array();
    MYSQL_RES *result = mysql_store_result(db);
    if (!result)
        return json::array();
    json values = json::array();
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result)))
        values.push_back(rowToJson(result, row));
    mysql_free_result(result);
    return values;
}

json Database::getMarks(const std::string &studentRoll, int semester)
{
    std::lock_guard<std::recursive_mutex> lock(dbMutex);
    MYSQL *db = connection(dbHandle);
    std::string condition = "student_roll='" + escape(db, studentRoll) + "'";
    if (semester > 0)
        condition += " AND semester=" + std::to_string(semester);
    if (!execute(db, "SELECT semester,subject,ie1,ie2,sem,credits,grade_points FROM marks WHERE " + condition + " ORDER BY semester,subject"))
        return json::object();
    MYSQL_RES *result = mysql_store_result(db);
    if (!result)
        return json::object();
    json values = json::object();
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result)))
    {
        json mark = json::object();
        if (row[2])
            mark["ie1"] = std::stoi(row[2]);
        if (row[3])
            mark["ie2"] = std::stoi(row[3]);
        if (row[4])
            mark["sem"] = std::stoi(row[4]);
        if (row[5])
            mark["credits"] = std::stod(row[5]);
        if (row[6])
            mark["grade_points"] = std::stod(row[6]);
        if (semester > 0)
            values[row[1]] = mark;
        else
            values[row[0]][row[1]] = mark;
    }
    mysql_free_result(result);
    return values;
}

json Database::getAvailableSemesters(const std::string &studentRoll)
{
    std::lock_guard<std::recursive_mutex> lock(dbMutex);
    MYSQL *db = connection(dbHandle);
    if (!execute(db, "SELECT DISTINCT semester FROM marks WHERE student_roll='" + escape(db, studentRoll) + "' ORDER BY semester"))
        return json::array();
    MYSQL_RES *result = mysql_store_result(db);
    if (!result)
        return json::array();
    json values = json::array();
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result)))
        values.push_back(std::stoi(row[0]));
    mysql_free_result(result);
    return values;
}

bool Database::addMarks(const std::string &studentRoll, const json &marksData, int semester)
{
    std::lock_guard<std::recursive_mutex> lock(dbMutex);
    MYSQL *db = connection(dbHandle);
    if (semester <= 0)
        semester = number(getStudent(studentRoll), "semester");
    if (semester <= 0 || marksData.empty())
        return false;
    for (auto iterator = marksData.begin(); iterator != marksData.end(); ++iterator)
    {
        const std::string &subject = iterator.key();
        const json &marks = iterator.value();
        std::vector<std::string> columns;
        std::vector<std::string> values;
        std::vector<std::string> updates;
        if (marks.contains("ie1"))
        {
            columns.push_back("ie1");
            values.push_back(marks["ie1"].is_null() ? "NULL" : std::to_string(marks["ie1"].get<int>()));
            updates.push_back("ie1=VALUES(ie1)");
        }
        if (marks.contains("ie2"))
        {
            columns.push_back("ie2");
            values.push_back(marks["ie2"].is_null() ? "NULL" : std::to_string(marks["ie2"].get<int>()));
            updates.push_back("ie2=VALUES(ie2)");
        }
        if (marks.contains("sem"))
        {
            columns.push_back("sem");
            values.push_back(marks["sem"].is_null() ? "NULL" : std::to_string(marks["sem"].get<int>()));
            updates.push_back("sem=VALUES(sem)");
        }
        if (marks.contains("credits"))
        {
            columns.push_back("credits");
            values.push_back(marks["credits"].is_null() ? "NULL" : std::to_string(marks["credits"].get<double>()));
            updates.push_back("credits=VALUES(credits)");
        }
        if (marks.contains("grade_points"))
        {
            columns.push_back("grade_points");
            values.push_back(marks["grade_points"].is_null() ? "NULL" : std::to_string(marks["grade_points"].get<double>()));
            updates.push_back("grade_points=VALUES(grade_points)");
        }
        if (columns.empty())
            continue;

        std::ostringstream query;
        query << "INSERT INTO marks (student_roll,semester,subject,";
        for (size_t index = 0; index < columns.size(); ++index)
            query << (index ? "," : "") << columns[index];
        query << ") VALUES ('" << escape(db, studentRoll) << "'," << semester << ",'" << escape(db, subject) << "',";
        for (size_t index = 0; index < values.size(); ++index)
            query << (index ? "," : "") << values[index];
        query << ") ON DUPLICATE KEY UPDATE ";
        for (size_t index = 0; index < updates.size(); ++index)
            query << (index ? "," : "") << updates[index];
        if (!execute(db, query.str()))
            return false;
    }
    return true;
}

bool Database::updateMarks(const std::string &studentRoll, const json &marksData, int semester)
{
    return addMarks(studentRoll, marksData, semester);
}

json Database::getResult(const std::string &studentRoll, int semester)
{
    std::lock_guard<std::recursive_mutex> lock(dbMutex);
    MYSQL *db = connection(dbHandle);
    if (semester <= 0)
        semester = number(getStudent(studentRoll), "semester");
    std::string query = "SELECT id,student_roll AS studentRoll,semester,total_marks AS totalMarks,percentage,grade,status,published_date AS publishedDate FROM results WHERE student_roll='" + escape(db, studentRoll) + "' AND semester=" + std::to_string(semester) + " LIMIT 1";
    if (!execute(db, query))
        return json::object();
    MYSQL_RES *result = mysql_store_result(db);
    if (!result)
        return json::object();
    MYSQL_ROW row = mysql_fetch_row(result);
    json value = row ? rowToJson(result, row) : json::object();
    mysql_free_result(result);
    return value;
}

bool Database::publishResult(const std::string &studentRoll, const json &resultData, int semester)
{
    std::lock_guard<std::recursive_mutex> lock(dbMutex);
    MYSQL *db = connection(dbHandle);
    if (semester <= 0)
        semester = number(getStudent(studentRoll), "semester");
    std::ostringstream query;
    query << "INSERT INTO results (student_roll,semester,total_marks,percentage,grade,status,published_date) VALUES ('"
          << escape(db, studentRoll) << "'," << semester << ',' << resultData.value("totalMarks", 0.0)
          << ',' << resultData.value("percentage", 0.0) << ",'" << escape(db, resultData.value("grade", "F"))
          << "','Published',NOW()) ON DUPLICATE KEY UPDATE total_marks=VALUES(total_marks),percentage=VALUES(percentage),grade=VALUES(grade),status='Published',published_date=NOW()";
    if (!execute(db, query.str()))
        return false;
    return execute(db, "UPDATE students SET result_status='Published' WHERE roll='" + escape(db, studentRoll) + "'");
}

json Database::getAllResults()
{
    std::lock_guard<std::recursive_mutex> lock(dbMutex);
    MYSQL *db = connection(dbHandle);
    if (!execute(db, "SELECT id,student_roll AS studentRoll,semester,total_marks AS totalMarks,percentage,grade,status,published_date AS publishedDate FROM results ORDER BY student_roll,semester"))
        return json::array();
    MYSQL_RES *result = mysql_store_result(db);
    if (!result)
        return json::array();
    json values = json::array();
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result)))
        values.push_back(rowToJson(result, row));
    mysql_free_result(result);
    return values;
}

bool Database::verifyStudentPassword(const std::string &roll, const std::string &password)
{
    std::lock_guard<std::recursive_mutex> lock(dbMutex);
    MYSQL *db = connection(dbHandle);
    if (!execute(db, "SELECT id FROM students WHERE roll='" + escape(db, roll) + "' AND password='" + escape(db, password) + "' LIMIT 1"))
        return false;
    MYSQL_RES *result = mysql_store_result(db);
    bool valid = result && mysql_num_rows(result) > 0;
    if (result)
        mysql_free_result(result);
    return valid;
}

bool Database::verifyFacultyPassword(const std::string &id, const std::string &password)
{
    std::lock_guard<std::recursive_mutex> lock(dbMutex);
    MYSQL *db = connection(dbHandle);
    if (!execute(db, "SELECT id FROM faculty WHERE id='" + escape(db, id) + "' AND password='" + escape(db, password) + "' LIMIT 1"))
        return false;
    MYSQL_RES *result = mysql_store_result(db);
    bool valid = result && mysql_num_rows(result) > 0;
    if (result)
        mysql_free_result(result);
    return valid;
}

bool Database::updateStudentPassword(const std::string &roll, const std::string &password)
{
    std::lock_guard<std::recursive_mutex> lock(dbMutex);
    MYSQL *db = connection(dbHandle);
    return execute(db, "UPDATE students SET password='" + escape(db, password) + "' WHERE roll='" + escape(db, roll) + "'") && mysql_affected_rows(db) > 0;
}

bool Database::updateFacultyPassword(const std::string &id, const std::string &password)
{
    std::lock_guard<std::recursive_mutex> lock(dbMutex);
    MYSQL *db = connection(dbHandle);
    return execute(db, "UPDATE faculty SET password='" + escape(db, password) + "' WHERE id='" + escape(db, id) + "'") && mysql_affected_rows(db) > 0;
}

bool Database::addFaculty(const json &data)
{
    std::lock_guard<std::recursive_mutex> lock(dbMutex);
    MYSQL *db = connection(dbHandle);
    std::string query = "INSERT INTO faculty (id,password,name,email,department) VALUES ('" + escape(db, text(data, "id")) + "','" + escape(db, text(data, "password")) + "','" + escape(db, text(data, "name")) + "','" + escape(db, text(data, "email")) + "','" + escape(db, text(data, "department")) + "')";
    return execute(db, query);
}

bool Database::updateFaculty(const std::string &id, const json &data)
{
    std::lock_guard<std::recursive_mutex> lock(dbMutex);
    MYSQL *db = connection(dbHandle);
    std::string query = "UPDATE faculty SET name='" + escape(db, text(data, "name")) + "',email='" + escape(db, text(data, "email")) + "',department='" + escape(db, text(data, "department")) + "' WHERE id='" + escape(db, id) + "'";
    return execute(db, query) && mysql_affected_rows(db) > 0;
}

bool Database::addNotification(const std::string &recipientType, const std::string &recipientId, const std::string &title, const std::string &message)
{
    std::lock_guard<std::recursive_mutex> lock(dbMutex);
    MYSQL *db = connection(dbHandle);
    std::ostringstream query;
    query << "INSERT INTO notifications (recipient_type,recipient_id,title,message) VALUES ('"
          << escape(db, recipientType) << "','" << escape(db, recipientId) << "','"
          << escape(db, title) << "','" << escape(db, message) << "')";
    return execute(db, query.str());
}

json Database::getNotifications(const std::string &recipientType, const std::string &recipientId)
{
    std::lock_guard<std::recursive_mutex> lock(dbMutex);
    MYSQL *db = connection(dbHandle);
    std::string query = "SELECT id,recipient_type AS recipientType,recipient_id AS recipientId,title,message,is_read AS isRead,created_at AS createdAt FROM notifications WHERE recipient_type='" + escape(db, recipientType) + "' AND recipient_id='" + escape(db, recipientId) + "' ORDER BY id DESC LIMIT 50";
    if (!execute(db, query))
        return json::array();
    MYSQL_RES *result = mysql_store_result(db);
    if (!result)
        return json::array();
    json values = json::array();
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result)))
        values.push_back(rowToJson(result, row));
    mysql_free_result(result);
    return values;
}

bool Database::markNotificationRead(int id)
{
    std::lock_guard<std::recursive_mutex> lock(dbMutex);
    MYSQL *db = connection(dbHandle);
    return execute(db, "UPDATE notifications SET is_read=1 WHERE id=" + std::to_string(id));
}

bool Database::markAllNotificationsRead(const std::string &recipientType, const std::string &recipientId)
{
    std::lock_guard<std::recursive_mutex> lock(dbMutex);
    MYSQL *db = connection(dbHandle);
    return execute(db, "UPDATE notifications SET is_read=1 WHERE recipient_type='" + escape(db, recipientType) + "' AND recipient_id='" + escape(db, recipientId) + "'");
}