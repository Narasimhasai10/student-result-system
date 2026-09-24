#include <iomanip>
#include <iostream>
#include <limits>
#include <string>

#include "auth.h"
#include "database.h"

namespace
{

    int readChoice()
    {
        int choice;
        while (!(std::cin >> choice))
        {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Enter a valid number: ";
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        return choice;
    }

    std::string readText(const std::string &prompt)
    {
        std::string value;
        std::cout << prompt;
        std::getline(std::cin, value);
        return value;
    }

    void waitForEnter()
    {
        std::cout << "\nPress Enter to continue...";
        std::string ignored;
        std::getline(std::cin, ignored);
    }

    void printStudent(const json &student)
    {
        if (student.empty())
        {
            std::cout << "Student not found.\n";
            return;
        }
        std::cout << "Roll Number: " << student.value("roll", "") << '\n'
                  << "Name: " << student.value("name", "") << '\n'
                  << "Email: " << student.value("email", "") << '\n'
                  << "Branch: " << student.value("branch", "") << '\n'
                  << "Year: " << student.value("year", 0) << '\n'
                  << "Semester: " << student.value("semester", 0) << '\n'
                  << "Academic Year: " << student.value("academicYear", "") << '\n';
    }

    void printMarks(const json &marks)
    {
        if (marks.empty())
        {
            std::cout << "No marks recorded.\n";
            return;
        }
        std::cout << std::left << std::setw(22) << "Subject" << std::setw(12)
                  << "Internal" << std::setw(12) << "External" << "Total\n";
        for (auto iterator = marks.begin(); iterator != marks.end(); ++iterator)
        {
            const std::string &subject = iterator.key();
            const json &value = iterator.value();
            std::string ie1 = value.contains("ie1") && !value["ie1"].is_null() ? std::to_string(value["ie1"].get<int>()) : "—";
            std::string ie2 = value.contains("ie2") && !value["ie2"].is_null() ? std::to_string(value["ie2"].get<int>()) : "—";
            std::string sem = value.contains("sem") && !value["sem"].is_null() ? std::to_string(value["sem"].get<int>()) : "—";
            std::string grade = value.contains("grade") && !value["grade"].is_null() ? value["grade"].get<std::string>() : "Not graded yet";
            std::cout << std::left << std::setw(22) << subject << std::setw(12)
                      << ie1 << std::setw(12) << ie2 << std::setw(12) << sem << grade
                      << '\n';
        }
    }

    void printAllMarks(const json &marksBySemester)
    {
        if (marksBySemester.empty())
        {
            std::cout << "No marks recorded.\n";
            return;
        }
        for (auto iterator = marksBySemester.begin(); iterator != marksBySemester.end(); ++iterator)
        {
            const std::string &semester = iterator.key();
            const json &marks = iterator.value();
            std::cout << "\nSemester " << semester << "\n";
            printMarks(marks);
        }
    }

    json calculatedResult(const json &marks)
    {
        double total = 0;
        bool passed = !marks.empty();
        for (const auto &item : marks.items())
        {
            int subjectTotal = item.value().value("ie1", 0) +
                               item.value().value("sem", 0);
            total += subjectTotal;
            passed = passed && subjectTotal >= 40;
        }
        double percentage = marks.empty() ? 0 : total / (marks.size() * 100) * 100;
        std::string grade = percentage >= 90 ? "A+" : percentage >= 80 ? "A"
                                                  : percentage >= 70   ? "B"
                                                  : percentage >= 60   ? "C"
                                                  : percentage >= 50   ? "D"
                                                  : percentage >= 40   ? "E"
                                                                       : "F";
        return {{"totalMarks", total}, {"percentage", percentage}, {"grade", grade}, {"status", passed ? "Pass" : "Fail"}};
    }

    void printResult(const std::string &roll, int semester, bool requirePublished)
    {
        Database &db = Database::getInstance();
        json student = db.getStudent(roll);
        if (student.empty())
        {
            std::cout << "Student not found.\n";
            return;
        }
        if (semester != student.value("semester", 0))
        {
            std::cout << "No result recorded for semester " << semester << ".\n";
            return;
        }
        json marks = db.getMarks(roll, semester);
        json result = db.getResult(roll, semester);
        if (requirePublished && result.empty())
        {
            std::cout << "No published result for the current semester.\n";
            return;
        }
        if (result.empty())
            result = calculatedResult(marks);
        json outcome = calculatedResult(marks);

        std::cout << "\nSemester " << semester << " Result\n";
        printMarks(marks);
        std::cout << std::fixed << std::setprecision(2)
                  << "Total: " << result.value("totalMarks", 0.0) << '\n'
                  << "Percentage: " << result.value("percentage", 0.0) << "%\n"
                  << "Grade: " << result.value("grade", "F") << '\n'
                  << "Pass/Fail: " << outcome.value("status", "Fail") << '\n';
    }

    void addOrUpdateMarks()
    {
        Database &db = Database::getInstance();
        std::string roll = readText("Roll Number: ");
        if (db.getStudent(roll).empty())
        {
            std::cout << "Student not found.\n";
            return;
        }
        int semester = std::stoi(readText("Semester: "));
        std::cout << "Number of subjects: ";
        int count = readChoice();
        json marks = json::object();
        for (int index = 0; index < count; ++index)
        {
            std::string subject = readText("Subject: ");
            int internal = std::stoi(readText("Internal marks: "));
            int external = std::stoi(readText("External marks: "));
            marks[subject] = {{"internal", internal}, {"external", external}};
        }
        if (db.addMarks(roll, marks, semester))
        {
            db.publishResult(roll, calculatedResult(db.getMarks(roll, semester)), semester);
            std::cout << "Marks saved successfully.\n";
        }
        else
        {
            std::cout << "Unable to save marks.\n";
        }
    }

    void addStudent()
    {
        json student;
        student["roll"] = readText("Roll Number: ");
        student["password"] = "webcap";
        student["name"] = readText("Name: ");
        student["email"] = readText("Email: ");
        student["phone"] = readText("Phone: ");
        student["gender"] = readText("Gender: ");
        student["branch"] = readText("Department/Branch: ");
        student["year"] = std::stoi(readText("Academic year number: "));
        student["semester"] = std::stoi(readText("Current semester: "));
        student["academicYear"] = readText("Academic year: ");
        student["dob"] = readText("Date of birth: ");
        if (Database::getInstance().addStudent(student))
            std::cout << "Student added successfully. Initial password: webcap\n";
        else
            std::cout << "Unable to add student. Roll number may already exist.\n";
    }

    void updateStudent()
    {
        Database &db = Database::getInstance();
        std::string roll = readText("Roll Number: ");
        json student = db.getStudent(roll);
        if (student.empty())
        {
            std::cout << "Student not found.\n";
            return;
        }
        student["name"] = readText("Name: ");
        student["email"] = readText("Email: ");
        student["phone"] = readText("Phone: ");
        student["gender"] = readText("Gender: ");
        student["branch"] = readText("Department/Branch: ");
        student["year"] = std::stoi(readText("Academic year number: "));
        student["semester"] = std::stoi(readText("Current semester: "));
        student["academicYear"] = readText("Academic year: ");
        student["dob"] = readText("Date of birth: ");
        if (db.updateStudent(roll, student))
            std::cout << "Student updated successfully.\n";
        else
            std::cout << "Unable to update student.\n";
    }

    void facultyMenu(const std::string &facultyId)
    {
        Database &db = Database::getInstance();
        for (;;)
        {
            std::cout << "\n========================================\nFACULTY DASHBOARD\nWelcome, Faculty!\n\n"
                      << "1. Add Student\n2. View All Students\n3. Search Student\n"
                      << "4. Update Student\n5. Delete Student\n6. Add Marks\n"
                      << "7. Update Marks\n8. View Student Results\n9. Change Password\n10. Logout\nEnter your choice: ";
            int choice = readChoice();
            if (choice == 1)
                addStudent();
            else if (choice == 2)
            {
                std::cout << "\n========================================\nALL STUDENTS\n";
                for (const auto &student : db.getAllStudents())
                {
                    printStudent(student);
                    std::cout << '\n';
                }
            }
            else if (choice == 3)
                printStudent(db.getStudent(readText("Roll Number: ")));
            else if (choice == 4)
                updateStudent();
            else if (choice == 5)
            {
                std::string roll = readText("Roll Number: ");
                std::cout << (db.deleteStudent(roll) ? "Student deleted successfully.\n" : "Student not found.\n");
            }
            else if (choice == 6 || choice == 7)
                addOrUpdateMarks();
            else if (choice == 8)
                printResult(readText("Roll Number: "), std::stoi(readText("Semester: ")), false);
            else if (choice == 9)
            {
                std::string current = readText("Current Password: ");
                std::string next = readText("New Password: ");
                std::string confirm = readText("Confirm New Password: ");
                if (!db.verifyFacultyPassword(facultyId, current))
                    std::cout << "Current password is incorrect.\n";
                else if (next.empty() || next != confirm)
                    std::cout << "New passwords do not match.\n";
                else if (db.updateFacultyPassword(facultyId, next))
                    std::cout << "Password changed successfully.\n";
                else
                    std::cout << "Unable to change password.\n";
            }
            else if (choice == 10)
                return;
            else
                std::cout << "Invalid choice.\n";
            if (choice != 10)
                waitForEnter();
        }
    }

    bool facultyLogin(std::string &facultyId)
    {
        std::cout << "\n========================================\nFACULTY LOGIN\n";
        facultyId = readText("Faculty ID: ");
        std::string password = readText("Password: ");
        if (Auth::loginFaculty(facultyId, password).contains("error"))
        {
            std::cout << "\nInvalid username or password!\nPlease try again.\n";
            return false;
        }
        return true;
    }

    bool studentLogin(std::string &roll)
    {
        std::cout << "\n========================================\nSTUDENT LOGIN\n";
        roll = readText("Roll Number: ");
        std::string password = readText("Password: ");
        if (Auth::loginStudent(roll, password).contains("error"))
        {
            std::cout << "\nInvalid credentials!\nPlease try again.\n";
            return false;
        }
        return true;
    }

    void studentMenu(const std::string &roll)
    {
        Database &db = Database::getInstance();
        json student = db.getStudent(roll);
        for (;;)
        {
            std::cout << "\n========================================\nSTUDENT DASHBOARD\nWelcome, "
                      << student.value("name", "") << "!\n\n1. View Profile\n"
                      << "2. View Current Semester Result\n3. View Previous Semester Results\n"
                      << "4. View All Marks\n5. Change Password\n6. Logout\nEnter your choice: ";
            int choice = readChoice();
            if (choice == 1)
                printStudent(student);
            else if (choice == 2)
                printResult(roll, student.value("semester", 0), true);
            else if (choice == 3)
            {
                int semester = readChoice();
                if (semester >= student.value("semester", 0))
                    std::cout << "Select a previous semester.\n";
                else
                    printResult(roll, semester, true);
            }
            else if (choice == 4)
                printAllMarks(db.getMarks(roll));
            else if (choice == 5)
            {
                std::string current = readText("Current Password: ");
                std::string next = readText("New Password: ");
                std::string confirm = readText("Confirm New Password: ");
                if (!db.verifyStudentPassword(roll, current))
                    std::cout << "Current password is incorrect.\n";
                else if (next.empty() || next != confirm)
                    std::cout << "New passwords do not match.\n";
                else
                    std::cout << (db.updateStudentPassword(roll, next) ? "Password changed successfully.\n" : "Unable to change password.\n");
            }
            else if (choice == 6)
                return;
            else
                std::cout << "Invalid choice.\n";
            if (choice != 6)
                waitForEnter();
        }
    }

} // namespace

int main()
{
    if (!Database::getInstance().init())
    {
        std::cerr << "Unable to initialize the database.\n";
        return 1;
    }
    for (;;)
    {
        std::cout << "\n========================================\nSTUDENT RESULT MANAGEMENT SYSTEM\n\n"
                  << "1. Faculty\n2. Student\n3. Exit\nEnter your choice: ";
        int choice = readChoice();
        if (choice == 1)
        {
            std::string facultyId;
            if (facultyLogin(facultyId))
                facultyMenu(facultyId);
        }
        else if (choice == 2)
        {
            std::string roll;
            if (studentLogin(roll))
                studentMenu(roll);
        }
        else if (choice == 3)
        {
            std::cout << "Goodbye.\n";
            return 0;
        }
        else
        {
            std::cout << "Invalid choice.\n";
        }
    }
}