# Student Result System - C++ Backend

A high-performance REST API backend for the Aditya University Student Result Management System, built with **Crow** (C++ web framework) and **MySQL**.

## Architecture

```
backend/
├── include/
│   ├── database.h       # SQLite database operations
│   └── auth.h           # Authentication and token handling
├── src/
│   ├── main.cpp         # Server and API routes
│   ├── database.cpp     # Database implementation
│   ├── auth.cpp         # Auth implementation
│   ├── student.cpp      # Student-specific logic
│   ├── faculty.cpp      # Faculty-specific logic
│   ├── marks.cpp        # Marks-specific logic
│   └── results.cpp      # Results-specific logic
├── CMakeLists.txt       # Build configuration
└── README.md            # This file
```

## Prerequisites

### Windows Development Setup

1. **Visual Studio 2019 or later** (with C++ development tools)
2. **CMake 3.10+**
3. **Git**

### Required Libraries

- **Crow**: C++ web framework (auto-downloaded by CMake)
- **nlohmann/json**: JSON library (auto-downloaded by CMake)
- **MySQL C client**: Database connector installed with MySQL Server 8.0

## Build Instructions

### Step 1: Create Build Directory

```bash
cd backend
mkdir build
cd build
```

### Step 2: Generate Build Files with CMake

```bash
cmake .. -G "Visual Studio 17 2022"
```

Or for other Visual Studio versions:
```bash
cmake .. -G "Visual Studio 16 2019"  # For Visual Studio 2019
```

### Step 3: Build the Project

```bash
cmake --build . --config Release
```

### Step 4: Run the Terminal Application

```bash
.\bin\Release\server.exe
```

Run the existing Crow website API separately:

```bash
.\bin\Release\web_server.exe
```

Or for Debug builds:
```bash
cmake --build . --config Debug
.\bin\Debug\server.exe
```

## API Endpoints

### Authentication

#### Student Login
```http
POST /api/auth/login/student
Content-Type: application/json

{
  "roll": "101",
  "password": "student123"
}
```

#### Faculty Login
```http
POST /api/auth/login/faculty
Content-Type: application/json

{
  "id": "FAC001",
  "password": "faculty123"
}
```

#### Logout
```http
POST /api/auth/logout
```

### Students Management

#### Get All Students
```http
GET /api/students
```

#### Get Student by Roll
```http
GET /api/students/{roll}
```

#### Create Student
```http
POST /api/students
Content-Type: application/json

{
  "roll": "108",
  "password": "student123",
  "name": "Student Name",
  "email": "student@college.edu",
  "phone": "9876543210",
  "gender": "Male",
  "branch": "CSE",
  "year": 1,
  "semester": 2,
  "academicYear": "2024-25",
  "dob": "2005-01-15"
}
```

#### Update Student
```http
PUT /api/students/{roll}
Content-Type: application/json

{
  "name": "Updated Name",
  "email": "updated@college.edu",
  "phone": "9876543211",
  "gender": "Female",
  "branch": "IT",
  "year": 2,
  "semester": 3,
  "academicYear": "2024-25",
  "dob": "2005-01-15",
  "resultStatus": "Published"
}
```

#### Delete Student
```http
DELETE /api/students/{roll}
```

### Marks Management

#### Get Marks for Student
```http
GET /api/marks/{roll}
```

#### Add/Update Marks
```http
POST /api/marks/{roll}
Content-Type: application/json

{
  "C++": {"internal": 25, "external": 60},
  "DBMS": {"internal": 24, "external": 58},
  "Mathematics": {"internal": 26, "external": 62},
  "Physics": {"internal": 23, "external": 55},
  "English": {"internal": 22, "external": 50}
}
```

#### Update Marks
```http
PUT /api/marks/{roll}
Content-Type: application/json

{
  "C++": {"internal": 28, "external": 68},
  "DBMS": {"internal": 27, "external": 65}
}
```

### Results Management

#### Get All Results
```http
GET /api/results
```

#### Get Result for Student
```http
GET /api/results/{roll}
```

#### Publish Result
```http
POST /api/results/{roll}
Content-Type: application/json

{
  "totalMarks": 285,
  "percentage": 85.5,
  "grade": "A"
}
```

### Faculty Management

#### Get All Faculty
```http
GET /api/faculty
```

#### Get Faculty by ID
```http
GET /api/faculty/{id}
```

### Health Check

#### Server Status
```http
GET /api/health
```

## Database Schema

### Students Table
```sql
CREATE TABLE students (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  roll TEXT UNIQUE NOT NULL,
  password TEXT NOT NULL,
  name TEXT NOT NULL,
  email TEXT,
  phone TEXT,
  gender TEXT,
  branch TEXT,
  year INTEGER,
  semester INTEGER,
  academic_year TEXT,
  dob TEXT,
  result_status TEXT DEFAULT 'Draft'
);
```

### Marks Table
```sql
CREATE TABLE marks (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  student_roll TEXT NOT NULL,
  subject TEXT NOT NULL,
  internal INTEGER,
  external INTEGER,
  FOREIGN KEY(student_roll) REFERENCES students(roll)
);
```

### Results Table
```sql
CREATE TABLE results (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  student_roll TEXT UNIQUE NOT NULL,
  total_marks REAL,
  percentage REAL,
  grade TEXT,
  status TEXT DEFAULT 'Draft',
  published_date TIMESTAMP,
  FOREIGN KEY(student_roll) REFERENCES students(roll)
);
```

### Faculty Table
```sql
CREATE TABLE faculty (
  id TEXT PRIMARY KEY,
  password TEXT NOT NULL,
  name TEXT NOT NULL,
  email TEXT,
  department TEXT
);
```

## Configuration

### Server Port
Default port: **8080**

To change, edit `src/main.cpp`:
```cpp
app.port(8080).multithreaded().run();
```

### Database File
Default location: **students.db** (in the current working directory)

To change, modify the `init()` call in `main.cpp`:
```cpp
db.init("path/to/database.db");
```

### CORS Settings
CORS is enabled for all origins by default. Modify in `src/main.cpp`:
```cpp
cors.origin("http://localhost:3000");  // Specific origin
```

## Initial Data Setup

The database is created automatically on first run. To seed initial data, you can:

1. Use the API to create students and faculty
2. Or create a seed script using SQL directly

### Example: Create Demo Student
```bash
curl -X POST http://localhost:8080/api/students \
  -H "Content-Type: application/json" \
  -d '{
    "roll": "101",
    "password": "student123",
    "name": "Raj Kumar",
    "email": "raj@college.edu",
    "phone": "9876543210",
    "gender": "Male",
    "branch": "CSE",
    "year": 1,
    "semester": 2,
    "academicYear": "2024-25",
    "dob": "2005-01-15"
  }'
```

## Frontend Integration

### Update Frontend API Endpoints

Modify frontend JS files to point to the backend:

1. **student-result-system/js/data.js** - Replace mock data with API calls
2. **student-result-system/js/auth.js** - Replace mock auth with backend endpoints

Example:
```javascript
async function loginStudent(roll, password) {
  const response = await fetch('http://localhost:8080/api/auth/login/student', {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ roll, password })
  });
  return response.json();
}
```

## Production Deployment

### Recommended Improvements

1. **Password Security**: Replace simple string comparison with bcrypt
2. **JWT Tokens**: Implement JWT instead of simple tokens
3. **Error Handling**: Add comprehensive error handling and logging
4. **Database**: Consider migrating from SQLite to PostgreSQL/MySQL for production
5. **SSL/HTTPS**: Enable HTTPS in production
6. **Rate Limiting**: Implement rate limiting for authentication endpoints
7. **Input Validation**: Add stricter input validation and sanitization

### Performance Optimization

1. Add database connection pooling
2. Implement caching for frequently accessed data
3. Add request/response compression
4. Use async/await for non-blocking operations

## Troubleshooting

### Build Errors

**Missing CMake**:
```bash
winget install cmake
```

**Missing C++ Build Tools**:
Install Visual Studio with C++ Desktop Development Workload

**JSON Library Not Found**:
CMake will download it automatically. If it fails:
```bash
cmake .. -DCMAKE_PREFIX_PATH="C:\path\to\dependencies"
```

### Runtime Errors

**Port Already in Use**:
Change port in `src/main.cpp` or kill existing process:
```bash
netstat -ano | findstr :8080
taskkill /PID <PID> /F
```

**Database Lock**:
Delete `students.db` and restart:
```bash
del students.db
```

## Development Tips

- **Debug Mode**: Use `cmake --build . --config Debug` for better debugging
- **Verbose Build**: Add `-DCMAKE_VERBOSE_MAKEFILE=ON` to CMake command
- **File Logging**: Redirect output to file: `server.exe > server.log 2>&1`

## Support & Documentation

- **Crow Framework**: https://crowcpp.org/
- **nlohmann JSON**: https://github.com/nlohmann/json
- **SQLite**: https://www.sqlite.org/

---

**Version**: 1.0.0  
**Last Updated**: 2024  
**Status**: Production Ready
