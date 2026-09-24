"""One-off AIML historical marks backfill for 25B11AI247."""

import requests

API_URL = "http://localhost:8080"
ROLL = "25B11AI247"

SEMESTER_MARKS = {
    1: {
        "2501CS01": {"ie1": 15, "ie2": 23, "credits": 4.0, "grade_points": 9},
        "2501EC95": {"ie1": 16, "ie2": 24, "credits": 3.0, "grade_points": 8},
        "2501EE01": {"ie1": 13, "ie2": 23, "credits": 4.0, "grade_points": 8},
        "2501EN01": {"credits": 1.0, "grade_points": 10},
        "2501IT01": {"credits": 2.0, "grade_points": 8},
        "2501MA01": {"ie1": 16, "ie2": 24, "credits": 3.0, "grade_points": 9},
        "2501UC08": {"credits": 2.0, "grade_points": 10},
    },
    2: {
        "2501AC01": {"credits": 0.0, "grade_points": 0},
        "2501CS03": {"ie1": 16, "ie2": 22, "credits": 4.0, "grade_points": 8},
        "2501CS71": {"ie1": 13, "ie2": 16, "credits": 3.0, "grade_points": 9},
        "2501EN02": {"credits": 1.0, "grade_points": 10},
        "2501IT42": {"ie1": 16, "ie2": 22, "credits": 4.0, "grade_points": 9},
        "2501MA02": {"ie1": 16, "ie2": 22, "credits": 3.0, "grade_points": 8},
        "2501ME01": {"credits": 3.0, "grade_points": 8},
        "2501PH02": {"ie1": 13, "ie2": 21, "credits": 3.0, "grade_points": 8},
        "2501UC07": {"credits": 1.0, "grade_points": 10},
        "2501UC11": {"credits": 0.0, "grade_points": 0},
    },
}


def ensure_student():
    response = requests.get(f"{API_URL}/api/students/{ROLL}", timeout=10)
    if response.status_code == 200:
        student = response.json()
        print(f"Student exists: {student['roll']} (year {student['year']}, semester {student['semester']})")
        return
    if response.status_code != 404:
        response.raise_for_status()

    student = {
        "roll": ROLL,
        "password": "webcap",
        "name": "D L NARASIMHA SAI",
        "email": "25b11ai247@adityauniversity.in",
        "phone": "1234567890",
        "gender": "Male",
        "branch": "AIML",
        "year": 2,
        "semester": 3,
        "academicYear": "2026-27",
        "dob": "2007-12-10",
    }
    created = requests.post(f"{API_URL}/api/students", json=student, timeout=10)
    created.raise_for_status()
    print(f"Created student {ROLL} with branch AIML")


def backfill_marks():
    for semester, marks in SEMESTER_MARKS.items():
        response = requests.post(
            f"{API_URL}/api/marks/{ROLL}",
            json={"semester": semester, **marks},
            timeout=10,
        )
        response.raise_for_status()
        print(f"Backfilled Semester {semester}: {len(marks)} subjects")


if __name__ == "__main__":
    requests.get(f"{API_URL}/api/health", timeout=10).raise_for_status()
    ensure_student()
    backfill_marks()
    print("AIML grade-point backfill complete")
