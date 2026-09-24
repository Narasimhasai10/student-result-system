"""
Database seeding script for Student Result System
Creates initial students, faculty, and sample data
"""

import requests
import json

API_URL = "http://localhost:8080"

# Sample students data
STUDENTS = [
    {
        "roll": "101",
        "password": "student123",
        "name": "Raj Kumar",
        "email": "raj.kumar@college.edu",
        "phone": "9876543210",
        "gender": "Male",
        "branch": "CSE",
        "year": 1,
        "semester": 2,
        "academicYear": "2024-25",
        "dob": "2005-01-15"
    },
    {
        "roll": "102",
        "password": "student123",
        "name": "Neha Sharma",
        "email": "neha.sharma@college.edu",
        "phone": "9876543211",
        "gender": "Female",
        "branch": "ECE",
        "year": 2,
        "semester": 3,
        "academicYear": "2024-25",
        "dob": "2004-06-20"
    },
    {
        "roll": "103",
        "password": "student123",
        "name": "Amit Patel",
        "email": "amit.patel@college.edu",
        "phone": "9876543212",
        "gender": "Male",
        "branch": "IT",
        "year": 1,
        "semester": 2,
        "academicYear": "2024-25",
        "dob": "2005-03-10"
    }
]

# Sample marks data
MARKS = {
    "101": {
        "C++": {"ie1": 24, "sem": 56},
        "DBMS": {"ie1": 23, "sem": 54},
        "Mathematics": {"ie1": 25, "sem": 60},
        "Physics": {"ie1": 22, "sem": 52},
        "English": {"ie1": 21, "sem": 50}
    },
    "102": {
        "C++": {"ie1": 26, "sem": 62},
        "DBMS": {"ie1": 25, "sem": 60},
        "Mathematics": {"ie1": 27, "sem": 64},
        "Physics": {"ie1": 24, "sem": 58},
        "English": {"ie1": 23, "sem": 55}
    },
    "103": {
        "C++": {"ie1": 22, "sem": 48},
        "DBMS": {"ie1": 21, "sem": 46},
        "Mathematics": {"ie1": 23, "sem": 50},
        "Physics": {"ie1": 20, "sem": 44},
        "English": {"ie1": 22, "sem": 49}
    }
}

AIML_BACKFILL = {
    1: {"2501CS01": {"ie1": 15, "ie2": 23}, "2501EC95": {"ie1": 16, "ie2": 24}, "2501EE01": {"ie1": 13, "ie2": 23}, "2501MA01": {"ie1": 16, "ie2": 24}},
    2: {"2501CS03": {"ie1": 16, "ie2": 22}, "2501CS71": {"ie1": 13, "ie2": 16}, "2501IT42": {"ie1": 16, "ie2": 22}, "2501MA02": {"ie1": 16, "ie2": 22}, "2501PH02": {"ie1": 13, "ie2": 21}}
}

def seed_database():
    print("Starting database seeding...")
    
    # Check if server is running
    try:
        response = requests.get(f"{API_URL}/api/health")
        if response.status_code != 200:
            print("❌ Server not running. Start the server first!")
            return
        print("✅ Connected to server")
    except:
        print("❌ Cannot connect to server at {API_URL}")
        print("   Start the server first: .\\build\\bin\\Release\\server.exe")
        return
    
    # Create students
    print("\nCreating students...")
    for student in STUDENTS:
        try:
            response = requests.post(
                f"{API_URL}/api/students",
                json=student,
                headers={"Content-Type": "application/json"}
            )
            if response.status_code == 201:
                print(f"✅ Created student: {student['name']} ({student['roll']})")
            else:
                print(f"⚠️  Failed to create {student['roll']}: {response.text}")
        except Exception as e:
            print(f"❌ Error creating student {student['roll']}: {e}")
    
    # Add marks for each student
    print("\nAdding marks...")
    for roll, marks in MARKS.items():
        try:
            payload = dict(marks)
            payload["semester"] = next(student["semester"] for student in STUDENTS if student["roll"] == roll)
            response = requests.post(
                f"{API_URL}/api/marks/{roll}",
                json=payload,
                headers={"Content-Type": "application/json"}
            )
            if response.status_code == 201:
                print(f"✅ Added marks for roll {roll}")
            else:
                print(f"⚠️  Failed to add marks for {roll}: {response.text}")
        except Exception as e:
            print(f"❌ Error adding marks for {roll}: {e}")

    print("\nAdding AIML historical IE marks...")
    for semester, marks in AIML_BACKFILL.items():
        response = requests.put(f"{API_URL}/api/marks/25B11AI247", json={"semester": semester, **marks}, headers={"Content-Type": "application/json"})
        print(f"{'✅' if response.status_code == 200 else '⚠️ '} Backfill semester {semester}: {response.text}")
    
    print("\n✅ Database seeding complete!")
    print("\nYou can now:")
    print("1. Login as student with roll: 101, password: student123")
    print("2. Check marks through the student dashboard")
    print("3. Faculty can manage all student data")

if __name__ == "__main__":
    seed_database()
