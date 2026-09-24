# Installation & Setup Guide

## Quick Start (Windows)

### 1. Install Prerequisites

**Option A: Using Chocolatey (Recommended)**
```powershell
choco install cmake visualstudio2022-complete
```

**Option B: Manual Installation**
- Download CMake from: https://cmake.org/download/
- Install Visual Studio 2022 from: https://visualstudio.microsoft.com/downloads/
  - Make sure to include "Desktop development with C++" workload

### 2. Build the Backend

Open Command Prompt or PowerShell in the `backend` directory and run:

```bash
.\build.bat
```

Or manually:
```bash
cd backend
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022"
cmake --build . --config Release
cd ..
```

### 3. Run the Terminal Application

```bash
.\build\bin\Release\server.exe
```

Run the existing website API separately with:

```bash
.\build\bin\Release\web_server.exe
```

It starts on `http://localhost:8080`.

---

## Quick Start (Linux/macOS)

### 1. Install Prerequisites

**Ubuntu/Debian:**
```bash
sudo apt-get update
sudo apt-get install cmake build-essential git
```

**macOS:**
```bash
brew install cmake
```

### 2. Build the Backend

```bash
cd backend
chmod +x build.sh
./build.sh
```

### 3. Run the Server

```bash
./build/bin/server
```

---

## Verify Installation

Once the server is running, test it with:

```bash
curl http://localhost:8080/api/health
```

Expected response:
```json
{"status":"healthy"}
```

---

## Troubleshooting

### CMake not found
```bash
# Windows: Add CMake to PATH
# Edit Environment Variables > Add C:\Program Files\CMake\bin

# Linux/macOS: Reinstall
sudo apt-get install cmake  # Ubuntu/Debian
brew install cmake           # macOS
```

### Visual Studio not found
```bash
# Run this in Command Prompt to set up MSVC compiler
"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"

# Then run the build command again
```

### Build fails with compiler errors
- Ensure Visual Studio 2022 C++ tools are installed
- Check that CMake version is 3.10 or higher: `cmake --version`
- Try cleaning and rebuilding:
  ```bash
  cd backend
  rmdir /s /q build
  .\build.bat
  ```

### Port 8080 already in use
Change port in `backend/src/main.cpp` line ~200:
```cpp
app.port(8081).multithreaded().run();  // Change 8080 to 8081
```

---

## Connect Frontend to Backend

### Update Frontend API Calls

Edit `student-result-system/js/auth.js`:

```javascript
// Replace mock login with real backend call
async function loginStudent(roll, password) {
  const response = await fetch('http://localhost:8080/api/auth/login/student', {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ roll, password })
  });
  
  if (!response.ok) {
    throw new Error('Login failed');
  }
  
  return response.json();
}
```

### Seed Initial Data

Use the provided Python script (optional):
```bash
python seed_database.py
```

Or manually create a student:
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

---

## Next Steps

1. Read [README.md](README.md) for complete API documentation
2. Test endpoints using Postman or curl
3. Update frontend to use backend APIs
4. Set up database backups for production
5. Implement HTTPS for production deployment

---

## Support

For issues or questions:
1. Check [README.md](README.md) Troubleshooting section
2. Visit Crow framework docs: https://crowcpp.org/
3. SQLite documentation: https://www.sqlite.org/docs.html
