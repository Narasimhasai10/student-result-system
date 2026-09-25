# 🎓 Student Result Management System

A full-stack web application for managing student academic results, built with a **C++ (Crow) REST API backend**, a **MySQL database**, and a **vanilla HTML/CSS/JavaScript frontend**. It has separate portals for students and faculty, with light and dark themes.

**🔗 Live Demo:** https://narasimhasai10.github.io/student-result-system/

---

## 📖 About This Project

This project was originally developed and tested as a complete **client–server system**: a C++ backend connected to a real MySQL database, running on `localhost:8080`. Because of that, the app only worked for people on the **same network as the host machine** — the backend server has to be running on a computer, and the frontend has to reach it at that address. Anyone outside that network (for example, someone opening it from another Wi-Fi, or through GitHub Pages) couldn't connect to the backend, so login and data loading would fail.

To let anyone try the project — without needing to install MySQL, build the C++ server, or be on the same network — this repo also includes a **Demo Mode**: a self-contained mock version of the same API that runs entirely inside the browser. It looks and behaves exactly like the real app, but uses sample data instead of a live database connection.

In short:
- **Real mode** = the actual system as built: C++ + MySQL, requires local setup and network access to the host
- **Demo mode** = a browser-only stand-in that lets anyone experience the full app instantly, with no backend required

---

## ✨ Features

### Student Portal
- Secure login with roll number and password
- View profile details
- View semester-wise marks (internal + external)
- View published results with grades and percentage
- Change password

### Faculty Portal
- Secure login with faculty ID and password
- Add, edit, and remove student records
- Enter and update marks by subject and semester
- Publish results once marks are finalized
- Search students by roll number, name, or branch

### General
- Responsive design that works on desktop and mobile
- Light and dark theme toggle with accessible color contrast
- Clean dashboard views for both roles

---

## 🧰 Tech Stack

| Layer     | Technology                          |
|-----------|--------------------------------------|
| Frontend  | HTML, CSS, JavaScript (no frameworks) |
| Backend   | C++17, [Crow](https://github.com/CrowCpp/Crow) web framework |
| Database  | MySQL 8.0                            |
| Build     | CMake                                |

---

## 📁 Project Structure

```
student-result-system/
├── docs/                     # Frontend (served by GitHub Pages)
│   ├── index.html
│   ├── css/style.css
│   ├── js/                   # auth.js, data.js, student.js, faculty.js, theme.js, ui.js
│   ├── student/               # Student portal pages
│   └── faculty/                # Faculty portal pages
├── backend/                  # C++ REST API server (the real system)
│   ├── src/                  # main.cpp, database.cpp, auth.cpp, student.cpp, faculty.cpp, marks.cpp, results.cpp
│   ├── include/               # Header files
│   ├── CMakeLists.txt
│   ├── SETUP.md               # Backend setup instructions
│   └── seed_database.py       # Script to seed sample data
└── database/
    └── student_result_system.sql   # Database schema
```

---

## 🧪 Option 1: Try the Demo (No Setup Needed)

This is what the live link and a downloaded ZIP both run by default.

**Online:** open https://narasimhasai10.github.io/student-result-system/

**Offline / downloaded copy:**
1. Click the green **Code** button above → **Download ZIP** → unzip it.
2. Open the `docs` folder and double-click **`index.html`**.
3. Log in with one of the demo accounts below.

No installs, no MySQL, no build steps required — the frontend automatically detects there's no backend to talk to and switches to demo mode on its own (it can also be forced with `?demo=1` in the URL).

### Demo Logins

| Role    | ID              | Password      |
|---------|-----------------|---------------|
| Student | `101`           | `student123`  |
| Student | `102`           | `student123`  |
| Student | `103`           | `student123`  |
| Faculty | `FAC001`        | `faculty123`  |

**How it works:** demo mode is a mock API that runs inside the browser and stores its data in `localStorage`. It mirrors the real backend's routes and responses exactly, but any changes you make (adding a student, entering marks, publishing a result) are only saved on your own device — nothing touches the real database, and nothing is shared between visitors.

---

## 🖥️ Option 2: Run the Real System (C++ Backend + MySQL)

This runs the actual system as it was built — the real database, the real C++ server.

1. Install **MySQL Server 8.0** and a C++ toolchain (MinGW-w64 or MSVC) with **CMake**.
2. Import the schema:
   ```
   mysql -u root -p < database/student_result_system.sql
   ```
3. Follow the detailed steps in [`backend/SETUP.md`](backend/SETUP.md) to configure the database connection and build the server.
4. Build and run the backend (see `backend/build.bat` / `backend/build.sh`). It starts on `http://localhost:8080`.
5. Open `docs/index.html` in a browser **on the same machine, or on another device on the same network** — the app will detect the running backend and use real data instead of demo mode.

> ⚠️ **Network limitation:** the real backend only accepts connections from the host machine (`localhost`) or other devices on the same local network as the host. It is not deployed anywhere public, so it cannot be reached from the internet or from GitHub Pages. This is why Option 1 (Demo Mode) exists — to let anyone try the app without needing access to the host's network.

Environment variables read by the backend (see `backend/src/database.cpp`):
- `SRMS_MYSQL_HOST` (default `127.0.0.1`)
- `SRMS_MYSQL_PORT` (default `3306`)
- `SRMS_MYSQL_USER`
- `SRMS_MYSQL_PASSWORD`

---

## 📡 API Overview

The backend exposes a REST API on port `8080`, including:

- `POST /auth/login/student`, `POST /auth/login/faculty`
- `GET/POST/PUT/DELETE /students`, `/students/{roll}`
- `GET/PUT /marks/{roll}`
- `GET/POST /results/{roll}`
- `GET /faculty/{id}`

See [`backend/OVERVIEW.md`](backend/OVERVIEW.md) and [`backend/Postman_Collection.json`](backend/Postman_Collection.json) for full details and example requests.

---

## 📱 Notes

- No real student data is included anywhere in this repository — all sample data (in the SQL seed and in demo mode) is fictional.
- The `backend/` folder is provided so the real system can be reviewed and run locally; it is not deployed as part of the live GitHub Pages demo.

---

## 📄 License

This project is provided for educational purposes.
