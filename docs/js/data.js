const API_BASE = "http://localhost:8080/api";
const SUBJECTS = ["C++", "DBMS", "Mathematics", "Physics", "English"];
const MAX_INTERNAL = 30, MAX_EXTERNAL = 70, MAX_TOTAL = 100;
const PASS_MARK = 40;
let STUDENTS = [];
let FACULTY = [];
const RECENT_ACTIVITIES = [];

/* ============================================================
   DEMO MODE — runs the whole app in the browser (localStorage)
   when no backend is reachable (e.g. on GitHub Pages).
   Force it with ?demo=1 in the URL. Login: 101 / student123, FAC001 / faculty123
   ============================================================ */
const DEMO_KEY = "srms_demo_db_v1";

function isDemoMode() {
  try {
    if (/github\.io$/i.test(location.hostname) || new URLSearchParams(location.search).has("demo")) sessionStorage.setItem("srms_demo", "1");
    return sessionStorage.getItem("srms_demo") === "1";
  } catch (e) { return false; }
}

function resetDemoData() { localStorage.removeItem(DEMO_KEY); }

function demoSeed() {
  const m = (internal, external) => ({ internal, external });
  const now = new Date().toISOString();
  return {
    faculty: [{ id: "FAC001", password: "faculty123", name: "Dr. Meera Nair", email: "meera.nair@college.edu", department: "Computer Science" }],
    students: [
      { roll: "101", password: "student123", name: "Raj Kumar", email: "raj.kumar@college.edu", phone: "9876543210", gender: "Male", branch: "CSE", year: 1, semester: 2, academicYear: "2024-25", dob: "2005-01-15", resultStatus: "Published" },
      { roll: "102", password: "student123", name: "Neha Sharma", email: "neha.sharma@college.edu", phone: "9876543211", gender: "Female", branch: "ECE", year: 2, semester: 3, academicYear: "2024-25", dob: "2004-06-20", resultStatus: "Published" },
      { roll: "103", password: "student123", name: "Amit Patel", email: "amit.patel@college.edu", phone: "9876543212", gender: "Male", branch: "IT", year: 1, semester: 2, academicYear: "2024-25", dob: "2005-03-10", resultStatus: "Published" }
    ],
    marks: {
      "101": { 1: { Mathematics: m(25, 60), Physics: m(23, 55) }, 2: { "C++": m(24, 56), DBMS: m(23, 54), English: m(21, 50) } },
      "102": { 3: { "C++": m(26, 62), DBMS: m(25, 60), Mathematics: m(27, 64) } },
      "103": { 2: { "C++": m(22, 48), DBMS: m(21, 46), Mathematics: m(23, 50) } }
    },
    results: [
      { studentRoll: "101", semester: 2, totalMarks: 228, percentage: 76, grade: "B", status: "Published", publishedDate: now },
      { studentRoll: "102", semester: 3, totalMarks: 264, percentage: 88, grade: "A", status: "Published", publishedDate: now },
      { studentRoll: "103", semester: 2, totalMarks: 210, percentage: 70, grade: "B", status: "Published", publishedDate: now }
    ]
  };
}

function demoLoad() {
  try { const d = JSON.parse(localStorage.getItem(DEMO_KEY)); if (d && d.students) return d; } catch (e) {}
  const d = demoSeed();
  localStorage.setItem(DEMO_KEY, JSON.stringify(d));
  return d;
}

// Mirrors the C++ REST API (same paths, same JSON shapes, same error messages).
function demoRequest(path, options = {}) {
  const method = (options.method || "GET").toUpperCase();
  const body = options.body ? JSON.parse(options.body) : {};
  const [route, qs] = path.split("?");
  const semParam = Number(new URLSearchParams(qs || "").get("semester")) || 0;
  const [res, a, b] = route.split("/").filter(Boolean).map(decodeURIComponent);
  const db = demoLoad();
  const save = () => localStorage.setItem(DEMO_KEY, JSON.stringify(db));
  const fail = msg => { throw new Error(msg); };
  const pub = ({ password, ...rest }) => rest;
  const findStudent = roll => db.students.find(s => s.roll === String(roll));
  const findFaculty = id => db.faculty.find(f => f.id.toLowerCase() === String(id).toLowerCase());

  if (res === "health") return { status: "ok", mode: "demo" };

  if (res === "auth") {
    if (a === "logout") return { success: true };
    const user = b === "student" ? findStudent(body.roll) : findFaculty(body.id);
    if (a === "login") {
      if (!user || user.password !== body.password) fail("Invalid credentials");
      return b === "student" ? { token: "demo-token", student: pub(user) } : { token: "demo-token", faculty: pub(user) };
    }
    if (a === "password") {
      if (!user || user.password !== body.currentPassword) fail("Current password is incorrect");
      if (!body.newPassword) fail("New password cannot be empty");
      user.password = body.newPassword; save();
      return { success: true };
    }
  }

  if (res === "students") {
    if (!a && method === "GET") return db.students.map(pub);
    if (!a && method === "POST") {
      for (const f of ["roll", "name", "email", "phone", "gender", "branch", "academicYear", "dob"]) if (!body[f]) fail("Missing field: " + f);
      if (!(Number(body.year) > 0 && Number(body.semester) > 0)) fail("Year and semester must be positive");
      if (findStudent(body.roll)) fail("Failed to add student");
      db.students.push({ password: "student123", resultStatus: "Draft", ...body, year: Number(body.year), semester: Number(body.semester) });
      save(); return { success: true };
    }
    const s = findStudent(a);
    if (!s) fail("Student not found");
    if (method === "GET") return pub(s);
    if (method === "PUT") {
      Object.assign(s, body, { roll: s.roll, year: Number(body.year ?? s.year), semester: Number(body.semester ?? s.semester) });
      save(); return { success: true };
    }
    if (method === "DELETE") {
      db.students = db.students.filter(x => x !== s);
      delete db.marks[a]; db.results = db.results.filter(r => r.studentRoll !== a);
      save(); return { success: true };
    }
  }

  if (res === "marks") {
    const all = db.marks[a] || {};
    if (b === "semesters") return Object.keys(all).map(Number).sort((x, y) => x - y);
    if (method === "GET") {
      const out = semParam ? all[semParam] : all;
      if (!out || !Object.keys(out).length) fail("Marks not found");
      return out;
    }
    const s = findStudent(a);
    if (!s) fail("Student not found");
    const { semester, ...subjects } = body;
    const sem = Number(semester) || s.semester;
    db.marks[a] = db.marks[a] || {};
    db.marks[a][sem] = { ...(db.marks[a][sem] || {}), ...subjects };
    save(); return { success: true };
  }

  if (res === "results") {
    if (!a) return db.results;
    const s = findStudent(a);
    if (method === "GET") {
      const sem = semParam || (s && s.semester);
      const r = db.results.find(x => x.studentRoll === a && x.semester === Number(sem));
      if (!r) fail("Result not found");
      return r;
    }
    if (!s) fail("Failed to publish result");
    const sem = Number(body.semester) || s.semester;
    db.results = db.results.filter(x => !(x.studentRoll === a && x.semester === sem));
    db.results.push({ studentRoll: a, semester: sem, totalMarks: body.totalMarks, percentage: body.percentage, grade: body.grade, status: "Published", publishedDate: new Date().toISOString() });
    s.resultStatus = "Published"; save();
    return { success: true };
  }

  if (res === "faculty") {
    if (!a) return db.faculty.map(pub);
    const f = findFaculty(a);
    if (!f) fail("Faculty not found");
    return pub(f);
  }

  return fail("Not found");
}

async function apiRequest(path, options = {}) {
  if (isDemoMode()) return demoRequest(path, options);
  let response;
  try {
    response = await fetch(`${API_BASE}${path}`, {
      ...options,
      headers: { "Content-Type": "application/json", ...(options.headers || {}) }
    });
  } catch (networkError) {
    // Backend unreachable -> switch to demo mode automatically
    try { sessionStorage.setItem("srms_demo", "1"); } catch (e) {}
    return demoRequest(path, options);
  }
  const body = await response.json().catch(() => ({}));
  if (!response.ok) throw new Error(body.error || "Request failed");
  return body;
}

function studentFromApi(student, marks = {}) {
  return { ...student, year: String(student.year), semester: String(student.semester), marks, password: undefined };
}

async function loadStudents() {
  const students = await apiRequest("/students");
  STUDENTS = await Promise.all(students.map(async student => {
    const marks = await apiRequest(`/marks/${encodeURIComponent(student.roll)}?semester=${student.semester}`).catch(() => ({}));
    return studentFromApi(student, marks);
  }));
  return STUDENTS;
}

async function loadStudent(roll, semester) {
  const student = await apiRequest(`/students/${encodeURIComponent(roll)}`);
  const selectedSemester = semester || student.semester;
  const marks = await apiRequest(`/marks/${encodeURIComponent(roll)}?semester=${selectedSemester}`).catch(() => ({}));
  return { ...studentFromApi(student, marks), semester: String(selectedSemester) };
}

async function loadFaculty(id) {
  const faculty = await apiRequest(`/faculty/${encodeURIComponent(id)}`);
  return { ...faculty, dept: faculty.department };
}

function gradeForTotal(total) {
  if (total >= 90) return "A+";
  if (total >= 80) return "A";
  if (total >= 70) return "B";
  if (total >= 60) return "C";
  if (total >= 50) return "D";
  if (total >= PASS_MARK) return "E";
  return "F";
}
function gradeClass(grade) {
  if (grade === "A+" || grade === "A") return "grade-A";
  if (grade === "B") return "grade-B";
  if (grade === "F") return "grade-F";
  return "grade-C";
}
function gradePoint(grade) { return ({"A+":10, A:9, B:8, C:7, D:6, E:5, F:0})[grade] ?? 0; }
function computeStudentReport(student) {
  const rows = Object.entries(student.marks || {}).map(([subject, mark]) => {
    const internal = Number(mark.internal) || 0, external = Number(mark.external) || 0;
    const total = internal + external;
    return { subject, internal, external, total, grade: gradeForTotal(total), pass: total >= PASS_MARK };
  });
  const totalMarks = rows.reduce((sum, row) => sum + row.total, 0);
  const maxMarks = rows.length * MAX_TOTAL;
  const percentage = maxMarks ? +(totalMarks / maxMarks * 100).toFixed(2) : 0;
  const overallGrade = gradeForTotal(percentage);
  return { rows, totalMarks, maxMarks, percentage, overallGrade, status: rows.length && rows.every(row => row.pass) ? "Pass" : "Fail", cgpa: rows.length ? +(rows.reduce((sum, row) => sum + gradePoint(row.grade), 0) / rows.length).toFixed(2) : 0 };
}

const Session = {
  KEY: "srms_session",
  set(data) { sessionStorage.setItem(this.KEY, JSON.stringify(data)); },
  get() { try { return JSON.parse(sessionStorage.getItem(this.KEY)); } catch (error) { return null; } },
  clear() { sessionStorage.removeItem(this.KEY); }
};

function findStudentByRoll(roll) { return STUDENTS.find(student => student.roll === String(roll).trim()); }
function findFacultyById(id) { return FACULTY.find(faculty => faculty.id.toLowerCase() === String(id).trim().toLowerCase()); }
