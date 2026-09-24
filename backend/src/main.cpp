#include <crow.h>
#include <crow/middlewares/cors.h>
#include <nlohmann/json.hpp>

#include "database.h"
#include "auth.h"

using json = nlohmann::json;

int main()
{
    // ============================================================
    // CREATE CROW APP WITH CORS MIDDLEWARE
    // ============================================================

    crow::App<crow::CORSHandler> app;

    auto &cors = app.get_middleware<crow::CORSHandler>();

    cors
        .global()
        .headers("Content-Type, Accept, Authorization")
        .methods(
            "GET"_method,
            "POST"_method,
            "PUT"_method,
            "DELETE"_method)
        .origin("*");

    // ============================================================
    // DATABASE
    // ============================================================

    Database &db = Database::getInstance();

    if (!db.init())
    {
        CROW_LOG_ERROR << "Failed to initialize database";
        return 1;
    }

    // ============================================================
    // AUTHENTICATION ENDPOINTS
    // ============================================================

    // Student Login
    CROW_ROUTE(app, "/api/auth/login/student")
        .methods("POST"_method)([](const crow::request &req)
                                {
        auto data = crow::json::load(req.body);

        if (!data || !data.has("roll") || !data.has("password"))
        {
            return crow::response(
                400,
                json{
                    {"error", "Missing roll or password"}
                }.dump()
            );
        }

        std::string roll = data["roll"].s();
        std::string password = data["password"].s();

        json response = Auth::loginStudent(roll, password);

        if (response.contains("error"))
        {
            return crow::response(401, response.dump());
        }

        return crow::response(200, response.dump()); });

    // Faculty Login
    CROW_ROUTE(app, "/api/auth/login/faculty")
        .methods("POST"_method)([](const crow::request &req)
                                {
        auto data = crow::json::load(req.body);

        if (!data || !data.has("id") || !data.has("password"))
        {
            return crow::response(
                400,
                json{
                    {"error", "Missing faculty id or password"}
                }.dump()
            );
        }

        std::string id = data["id"].s();
        std::string password = data["password"].s();

        json response = Auth::loginFaculty(id, password);

        if (response.contains("error"))
        {
            return crow::response(401, response.dump());
        }

        return crow::response(200, response.dump()); });

    // Logout
    CROW_ROUTE(app, "/api/auth/logout")
        .methods("POST"_method)([]()
                                { return crow::response(
                                      200,
                                      json{
                                          {"success", true}}
                                          .dump()); });

    // Student Change Password
    CROW_ROUTE(app, "/api/auth/password/student")
        .methods("PUT"_method)([](const crow::request &req)
                               {
        auto data = crow::json::load(req.body);

        if (!data ||
            !data.has("roll") ||
            !data.has("currentPassword") ||
            !data.has("newPassword"))
        {
            return crow::response(
                400,
                json{
                    {"error", "Missing password fields"}
                }.dump()
            );
        }

        std::string roll = data["roll"].s();
        std::string currentPassword = data["currentPassword"].s();
        std::string newPassword = data["newPassword"].s();

        if (!Database::getInstance().verifyStudentPassword(
                roll,
                currentPassword))
        {
            return crow::response(
                401,
                json{
                    {"error", "Current password is incorrect"}
                }.dump()
            );
        }

        if (newPassword.empty())
        {
            return crow::response(
                400,
                json{
                    {"error", "New password cannot be empty"}
                }.dump()
            );
        }

        if (!Database::getInstance().updateStudentPassword(
                roll,
                newPassword))
        {
            return crow::response(
                400,
                json{
                    {"error", "Password update failed"}
                }.dump()
            );
        }

        return crow::response(
            200,
            json{
                {"success", true}
            }.dump()
        ); });

    // Faculty Change Password
    CROW_ROUTE(app, "/api/auth/password/faculty")
        .methods("PUT"_method)([](const crow::request &req)
                               {
        auto data = crow::json::load(req.body);

        if (!data ||
            !data.has("id") ||
            !data.has("currentPassword") ||
            !data.has("newPassword"))
        {
            return crow::response(
                400,
                json{
                    {"error", "Missing password fields"}
                }.dump()
            );
        }

        std::string id = data["id"].s();
        std::string currentPassword = data["currentPassword"].s();
        std::string newPassword = data["newPassword"].s();

        if (!Database::getInstance().verifyFacultyPassword(
                id,
                currentPassword))
        {
            return crow::response(
                401,
                json{
                    {"error", "Current password is incorrect"}
                }.dump()
            );
        }

        if (newPassword.empty())
        {
            return crow::response(
                400,
                json{
                    {"error", "New password cannot be empty"}
                }.dump()
            );
        }

        if (!Database::getInstance().updateFacultyPassword(
                id,
                newPassword))
        {
            return crow::response(
                400,
                json{
                    {"error", "Password update failed"}
                }.dump()
            );
        }

        return crow::response(
            200,
            json{
                {"success", true}
            }.dump()
        ); });

    // ============================================================
    // STUDENT ENDPOINTS
    // ============================================================

    // Get all students
    CROW_ROUTE(app, "/api/students")
        .methods("GET"_method)([]()
                               {
        json students =
            Database::getInstance().getAllStudents();

        return crow::response(
            200,
            students.dump()
        ); });

    // Get student by roll
    CROW_ROUTE(app, "/api/students/<string>")
        .methods("GET"_method)([](std::string roll)
                               {
        json student =
            Database::getInstance().getStudent(roll);

        if (student.empty())
        {
            return crow::response(
                404,
                json{
                    {"error", "Student not found"}
                }.dump()
            );
        }

        return crow::response(
            200,
            student.dump()
        ); });

    // Create student
    CROW_ROUTE(app, "/api/students")
        .methods("POST"_method)([](const crow::request &req)
                                {
        auto data = crow::json::load(req.body);

        if (!data)
        {
            return crow::response(
                400,
                json{
                    {"error", "Invalid JSON"}
                }.dump()
            );
        }

        json jsonData = json::parse(req.body);

        const char* requiredFields[] =
        {
            "roll",
            "name",
            "email",
            "phone",
            "gender",
            "branch",
            "academicYear",
            "dob"
        };

        for (const char* field : requiredFields)
        {
            if (!jsonData.contains(field) ||
                !jsonData[field].is_string() ||
                jsonData[field].get<std::string>().empty())
            {
                return crow::response(
                    400,
                    json{
                        {
                            "error",
                            std::string("Missing field: ") + field
                        }
                    }.dump()
                );
            }
        }

        if (!jsonData.contains("year") ||
            !jsonData.contains("semester") ||
            !jsonData["year"].is_number_integer() ||
            !jsonData["semester"].is_number_integer() ||
            jsonData["year"].get<int>() <= 0 ||
            jsonData["semester"].get<int>() <= 0)
        {
            return crow::response(
                400,
                json{
                    {"error", "Year and semester must be positive"}
                }.dump()
            );
        }

        if (Database::getInstance().addStudent(jsonData))
        {
            Database::getInstance().addNotification("student", jsonData["roll"].get<std::string>(), "Welcome to Aditya University", "Your student account is ready.");
            return crow::response(
                201,
                json{
                    {"success", true},
                    {"message", "Student added"}
                }.dump()
            );
        }

        return crow::response(
            400,
            json{
                {"error", "Failed to add student"}
            }.dump()
        ); });

    // Update student
    CROW_ROUTE(app, "/api/students/<string>")
        .methods("PUT"_method)([](const crow::request &req, std::string roll)
                               {
        auto data = crow::json::load(req.body);

        if (!data)
        {
            return crow::response(
                400,
                json{
                    {"error", "Invalid JSON"}
                }.dump()
            );
        }

        json jsonData = json::parse(req.body);

        if (Database::getInstance().updateStudent(
                roll,
                jsonData))
        {
            Database::getInstance().addNotification("student", roll, "Your details are updated", "Your student details have been updated by faculty.");
            return crow::response(
                200,
                json{
                    {"success", true},
                    {"message", "Student updated"}
                }.dump()
            );
        }

        return crow::response(
            404,
            json{
                {"error", "Student not found"}
            }.dump()
        ); });

    // Delete student
    CROW_ROUTE(app, "/api/students/<string>")
        .methods("DELETE"_method)([](std::string roll)
                                  {
        if (Database::getInstance().deleteStudent(roll))
        {
            return crow::response(
                200,
                json{
                    {"success", true},
                    {"message", "Student deleted"}
                }.dump()
            );
        }

        return crow::response(
            404,
            json{
                {"error", "Student not found"}
            }.dump()
        ); });

    // ============================================================
    // MARKS ENDPOINTS
    // ============================================================

    // Get marks for student
    CROW_ROUTE(app, "/api/marks/<string>")
        .methods("GET"_method)([](const crow::request &req, std::string roll)
                               {
        int semester = 0;

        auto semesterParam =
            req.url_params.get("semester");

        if (semesterParam)
        {
            semester = std::stoi(semesterParam);
        }

        json marks =
            Database::getInstance().getMarks(
                roll,
                semester
            );

        if (marks.empty())
        {
            return crow::response(
                404,
                json{
                    {"error", "Marks not found"}
                }.dump()
            );
        }

        return crow::response(
            200,
            marks.dump()
        ); });

    // Get available semesters
    CROW_ROUTE(app, "/api/marks/<string>/semesters")
        .methods("GET"_method)([](std::string roll)
                               { return crow::response(
                                     200,
                                     Database::getInstance()
                                         .getAvailableSemesters(roll)
                                         .dump()); });

    // Add marks
    CROW_ROUTE(app, "/api/marks/<string>")
        .methods("POST"_method)([](const crow::request &req, std::string roll)
                                {
        auto data = crow::json::load(req.body);

        if (!data)
        {
            return crow::response(
                400,
                json{
                    {"error", "Invalid JSON"}
                }.dump()
            );
        }

        json jsonData = json::parse(req.body);

        int semester =
            jsonData.value("semester", 0);

        jsonData.erase("semester");
        bool includesGradePoints = false;
        for (const auto &item : jsonData.items())
            includesGradePoints = includesGradePoints || item.value().is_object() && item.value().contains("grade_points");

        if (Database::getInstance().addMarks(
                roll,
                jsonData,
                semester))
        {
            Database::getInstance().addNotification("student", roll, includesGradePoints ? "Result Recorded" : "Marks Added", includesGradePoints ? "Final grade points have been recorded." : "New marks have been added to your academic record.");
            return crow::response(
                201,
                json{
                    {"success", true},
                    {"message", "Marks added"}
                }.dump()
            );
        }

        return crow::response(
            400,
            json{
                {"error", "Failed to add marks"}
            }.dump()
        ); });

    // Update marks
    CROW_ROUTE(app, "/api/marks/<string>")
        .methods("PUT"_method)([](const crow::request &req, std::string roll)
                               {
        auto data = crow::json::load(req.body);

        if (!data)
        {
            return crow::response(
                400,
                json{
                    {"error", "Invalid JSON"}
                }.dump()
            );
        }

        json jsonData = json::parse(req.body);

        int semester =
            jsonData.value("semester", 0);

        jsonData.erase("semester");
        bool includesGradePoints = false;
        for (const auto &item : jsonData.items())
            includesGradePoints = includesGradePoints || item.value().is_object() && item.value().contains("grade_points");

        if (Database::getInstance().updateMarks(
                roll,
                jsonData,
                semester))
        {
            Database::getInstance().addNotification("student", roll, includesGradePoints ? "Result Recorded" : "Marks Added", includesGradePoints ? "Final grade points have been recorded." : "Your marks have been updated in the academic record.");
            return crow::response(
                200,
                json{
                    {"success", true},
                    {"message", "Marks updated"}
                }.dump()
            );
        }

        return crow::response(
            404,
            json{
                {"error", "Student not found"}
            }.dump()
        ); });

    // ============================================================
    // RESULTS ENDPOINTS
    // ============================================================

    // Get all results
    CROW_ROUTE(app, "/api/results")
        .methods("GET"_method)([]()
                               {
        json results =
            Database::getInstance().getAllResults();

        return crow::response(
            200,
            results.dump()
        ); });

    // Get result for student
    CROW_ROUTE(app, "/api/results/<string>")
        .methods("GET"_method)([](const crow::request &req, std::string roll)
                               {
        int semester = 0;

        auto semesterParam =
            req.url_params.get("semester");

        if (semesterParam)
        {
            semester = std::stoi(semesterParam);
        }

        json result =
            Database::getInstance().getResult(
                roll,
                semester
            );

        if (result.empty())
        {
            return crow::response(
                404,
                json{
                    {"error", "Result not found"}
                }.dump()
            );
        }

        return crow::response(
            200,
            result.dump()
        ); });

    // Publish result
    CROW_ROUTE(app, "/api/results/<string>")
        .methods("POST"_method)([](const crow::request &req, std::string roll)
                                {
        auto data = crow::json::load(req.body);

        if (!data)
        {
            return crow::response(
                400,
                json{
                    {"error", "Invalid JSON"}
                }.dump()
            );
        }

        json jsonData = json::parse(req.body);

        int semester =
            jsonData.value("semester", 0);

        std::string facultyId = jsonData.value("facultyId", "");

        jsonData.erase("semester");
        jsonData.erase("facultyId");

        if (Database::getInstance().publishResult(
                roll,
                jsonData,
                semester))
        {
            Database::getInstance().addNotification("student", roll, "Result Published", "Your result has been published.");
            if (!facultyId.empty())
            Database::getInstance().addNotification("faculty", facultyId, "Result Published", "You published the result for " + roll + ".");
            return crow::response(
                201,
                json{
                    {"success", true},
                    {"message", "Result published"}
                }.dump()
            );
        }

        return crow::response(
            400,
            json{
                {"error", "Failed to publish result"}
            }.dump()
        ); });

    // ============================================================
    // FACULTY ENDPOINTS
    // ============================================================

    // Get all faculty
    CROW_ROUTE(app, "/api/faculty")
        .methods("GET"_method)([]()
                               {
        json faculty =
            Database::getInstance().getAllFaculty();

        return crow::response(
            200,
            faculty.dump()
        ); });

    // Create faculty
    CROW_ROUTE(app, "/api/faculty")
        .methods("POST"_method)([](const crow::request &req)
                                {
        auto data = crow::json::load(req.body);
        if (!data)
            return crow::response(400, json{{"error", "Invalid JSON"}}.dump());

        json jsonData = json::parse(req.body);
        const char* requiredFields[] = {"id", "password", "name", "email", "department"};
        for (const char* field : requiredFields)
        {
            if (!jsonData.contains(field) || !jsonData[field].is_string() || jsonData[field].get<std::string>().empty())
                return crow::response(400, json{{"error", std::string("Missing field: ") + field}}.dump());
        }

        if (Database::getInstance().addFaculty(jsonData))
        {
            Database::getInstance().addNotification("faculty", jsonData["id"].get<std::string>(), "Welcome to Aditya University", "Your faculty account is ready.");
            return crow::response(201, json{{"success", true}, {"message", "Faculty added"}}.dump());
        }
        return crow::response(400, json{{"error", "Failed to add faculty"}}.dump()); });

    // Get faculty by ID
    CROW_ROUTE(app, "/api/faculty/<string>")
        .methods("GET"_method)([](std::string id)
                               {
        json faculty =
            Database::getInstance().getFaculty(id);

        if (faculty.is_null())
        {
            return crow::response(
                404,
                json{
                    {"error", "Faculty not found"}
                }.dump()
            );
        }

        return crow::response(
            200,
            faculty.dump()
        ); });

    // NOTIFICATION ENDPOINTS
    CROW_ROUTE(app, "/api/notifications/<string>/<string>")
        .methods("GET"_method)([](std::string type, std::string id)
                               {
        if (type != "student" && type != "faculty")
            return crow::response(400, json{{"error", "Invalid recipient type"}}.dump());
        return crow::response(200, Database::getInstance().getNotifications(type, id).dump()); });

    CROW_ROUTE(app, "/api/notifications/<int>/read")
        .methods("PUT"_method)([](int id)
                               {
        if (Database::getInstance().markNotificationRead(id))
            return crow::response(200, json{{"success", true}}.dump());
        return crow::response(404, json{{"error", "Notification not found"}}.dump()); });

    CROW_ROUTE(app, "/api/notifications/<string>/<string>/read-all")
        .methods("PUT"_method)([](std::string type, std::string id)
                               {
        if (type != "student" && type != "faculty")
            return crow::response(400, json{{"error", "Invalid recipient type"}}.dump());
        if (Database::getInstance().markAllNotificationsRead(type, id))
            return crow::response(200, json{{"success", true}}.dump());
        return crow::response(400, json{{"error", "Failed to mark notifications read"}}.dump()); });

    // ============================================================
    // HEALTH CHECK
    // ============================================================

    CROW_ROUTE(app, "/api/health")
        .methods("GET"_method)([]()
                               { return crow::response(
                                     200,
                                     json{
                                         {"status", "healthy"}}
                                         .dump()); });

    // ============================================================
    // START SERVER
    // ============================================================

    CROW_LOG_INFO
        << "Server running on http://0.0.0.0:8080";

    app.port(8080)
        .multithreaded()
        .run();

    return 0;
}