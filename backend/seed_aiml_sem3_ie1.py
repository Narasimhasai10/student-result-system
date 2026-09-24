"""One-off Semester 3 IE-1 marks seed for AIML student 25B11AI247."""

import requests

API_URL = "http://localhost:8080"
ROLL = "25B11AI247"
IE1_MARKS = {
    "AI": {"ie1": 16},
    "CPP": {"ie1": 16},
    "DBMS": {"ie1": 15},
    "ADSA": {"ie1": 16},
    "MATH": {"ie1": 14},
    "ASE": {"ie1": 14},
}


def main():
    requests.get(f"{API_URL}/api/health", timeout=10).raise_for_status()
    response = requests.put(
        f"{API_URL}/api/marks/{ROLL}",
        params={"semester": 3},
        json={"semester": 3, **IE1_MARKS},
        timeout=10,
    )
    response.raise_for_status()
    print(f"Inserted IE-1 marks for {ROLL}, Semester 3: {', '.join(IE1_MARKS)}")


if __name__ == "__main__":
    main()
