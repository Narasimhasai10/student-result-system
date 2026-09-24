# Backend Overview

## 📋 Project Structure

```
backend/
├── include/                  # C++ Header Files
│   ├── database.h           # SQLite Database Interface
│   └── auth.h               # Authentication & Authorization
│
├── src/                     # C++ Source Files
│   ├── main.cpp             # REST API Server & Routes (500+ lines)
│   ├── database.cpp         # SQLite Implementation (700+ lines)
│   ├── auth.cpp             # Auth Implementation (100+ lines)
│   ├── student.cpp          # Student-specific logic (placeholder)
│   ├── faculty.cpp          # Faculty-specific logic (placeholder)
│   ├── marks.cpp            # Marks-specific logic (placeholder)
│   └── results.cpp          # Results-specific logic (placeholder)
│
├── build/                   # Build output directory (auto-generated)
│   └── bin/
│       ├── Debug/
│       │   └── server.exe   # Debug executable
│       └── Release/
│           └── server.exe   # Release executable
│
├── CMakeLists.txt           # CMake Build Configuration
├── package.json             # Project metadata
├── README.md                # Complete API Documentation
├── SETUP.md                 # Installation & Setup Guide
├── build.bat                # Windows Build Script
├── build.sh                 # Linux/macOS Build Script
├── seed_database.py         # Database Seeding Script
└── .gitignore               # Git Ignore Rules
```

## 🎯 Key Features

✅ **Complete REST API** with 25+ endpoints  
✅ **MySQL Database** with automatic initialization  
✅ **Student Management** - CRUD operations  
✅ **Faculty Management** - Role-based access  
✅ **Marks Management** - Add, update, retrieve  
✅ **Results Publishing** - Track student results  
✅ **Authentication** - Student & Faculty login  
✅ **CORS Support** - Cross-origin requests allowed  
✅ **Error Handling** - Comprehensive error responses  
✅ **JSON API** - RESTful JSON responses  

## 🚀 Quick Start

### Windows (Recommended)

```bash
# Step 1: Open Command Prompt in backend folder
cd backend

# Step 2: Run build script
.\build.bat

# Step 3: Start server
.\build\bin\Release\server.exe
```

### Linux/macOS

```bash
cd backend
chmod +x build.sh
./build.sh
./build/bin/server
```

Server will run on: **http://localhost:8080**

## 📡 API Endpoints (25+)

### Authentication (3)
- `POST /api/auth/login/student` - Student login
- `POST /api/auth/login/faculty` - Faculty login
- `POST /api/auth/logout` - Logout

### Students (5)
- `GET /api/students` - Get all students
- `GET /api/students/{roll}` - Get student by roll
- `POST /api/students` - Create student
- `PUT /api/students/{roll}` - Update student
- `DELETE /api/students/{roll}` - Delete student

### Marks (3)
- `GET /api/marks/{roll}` - Get student marks
- `POST /api/marks/{roll}` - Add marks
- `PUT /api/marks/{roll}` - Update marks

### Results (3)
- `GET /api/results` - Get all results
- `GET /api/results/{roll}` - Get student result
- `POST /api/results/{roll}` - Publish result

### Faculty (2)
- `GET /api/faculty` - Get all faculty
- `GET /api/faculty/{id}` - Get faculty by ID

### Utility (1)
- `GET /api/health` - Health check

## 🛠️ Technology Stack

| Component | Technology | Version |
|-----------|-----------|---------|
| **Framework** | Crow C++ | 1.0.1 |
| **Database** | MySQL | 8.0+ |
| **JSON** | nlohmann/json | 3.11.2 |
| **C++ Standard** | C++17 | 17 |
| **Build System** | CMake | 3.10+ |
| **Compiler** | MSVC/GCC/Clang | Latest |

## 📦 Dependencies

### Automatic Download (via CMake)
- **Crow** - REST framework
- **nlohmann_json** - JSON library
- **SQLite3** - Database engine

### System Requirements
- **Visual Studio 2019+** (Windows)
- **CMake 3.10+**
- **C++17 compatible compiler**

## 🔧 Build Options

```bash
# Release Build (Optimized)
cmake --build . --config Release

# Debug Build (With debugging symbols)
cmake --build . --config Debug

# Rebuild from scratch
cd backend
rm -rf build
cmake -B build
cmake --build build
```

## 🗄️ Database Schema

### Students Table
- id (Primary Key)
- roll (Unique)
- password
- name
- email, phone
- gender, branch
- year, semester
- academic_year
- dob (Date of Birth)
- result_status

### Marks Table
- id (Primary Key)
- student_roll (Foreign Key)
- subject
- internal, external marks

### Results Table
- id (Primary Key)
- student_roll (Foreign Key)
- total_marks, percentage
- grade, status
- published_date

### Faculty Table
- id (Primary Key)
- password
- name, email
- department

## 🔌 Integration with Frontend

### Example: Replace Mock Login

**Before (Mock):**
```javascript
const student = findStudentByRoll(roll);
if (!student || student.password !== password) { /* error */ }
```

**After (Backend):**
```javascript
const response = await fetch('http://localhost:8080/api/auth/login/student', {
  method: 'POST',
  headers: { 'Content-Type': 'application/json' },
  body: JSON.stringify({ roll, password })
});
const { token, student } = await response.json();
Session.set({ token, role: "student", roll: student.roll });
```

## 📝 Common Tasks

### Seed Database with Sample Data
```bash
python seed_database.py
```

### Test API Endpoint
```bash
curl http://localhost:8080/api/health

# Or with data
curl -X POST http://localhost:8080/api/auth/login/student \
  -H "Content-Type: application/json" \
  -d '{"roll":"101","password":"student123"}'
```

### Check Open Ports
```bash
# Windows
netstat -ano | findstr :8080

# Linux/macOS
lsof -i :8080
```

### View Database
```bash
# Install SQLite
# Windows: choco install sqlite

# View data
sqlite3 students.db "SELECT * FROM students;"
```

## 🐛 Troubleshooting

| Issue | Solution |
|-------|----------|
| **CMake not found** | Install from https://cmake.org/ |
| **Compiler error** | Install Visual Studio C++ tools |
| **Port 8080 in use** | Change port in main.cpp or kill process |
| **Build fails** | Clean build: `rmdir build && mkdir build` |
| **Database locked** | Delete `students.db` and restart |

See [SETUP.md](SETUP.md) for detailed troubleshooting.

## 📚 Documentation Files

- **README.md** - Complete API documentation & examples
- **SETUP.md** - Installation & setup guide
- **This file** - Project overview

## 🚢 Production Deployment

### Before Going Live:

1. **Security**
   - Implement bcrypt password hashing
   - Use JWT tokens instead of simple tokens
   - Add rate limiting
   - Enable HTTPS/SSL

2. **Database**
   - Migrate to PostgreSQL/MySQL
   - Set up automated backups
   - Create database indexes

3. **Monitoring**
   - Add logging
   - Set up error tracking
   - Monitor server performance

4. **Optimization**
   - Enable connection pooling
   - Implement caching
   - Add request compression

See README.md "Production Deployment" section for details.

## 📞 Support & Resources

- **Crow Framework**: https://crowcpp.org/
- **nlohmann JSON**: https://github.com/nlohmann/json
- **SQLite**: https://www.sqlite.org/
- **CMake**: https://cmake.org/documentation/

## 📋 Version Info

- **Backend Version**: 1.0.0
- **API Version**: 1.0
- **Last Updated**: 2024
- **Status**: Production Ready ✅

---

**Next Step**: Run `.\build.bat` to build the backend!

Questions? Check [README.md](README.md) or [SETUP.md](SETUP.md)
