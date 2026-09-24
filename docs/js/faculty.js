/* ============================================================
   FACULTY DOMAIN — page rendering & interaction logic
   [BACKEND] Every mutation here (add/delete/publish) is only
   held in memory for this browser tab. Replace with real
   POST/PUT/DELETE calls to /api/students and /api/results.
   ============================================================ */

let currentPage = 1;
const PAGE_SIZE = 4;
let pendingDeleteRoll = null;
let editingStudentRoll = null;

function populateFacultySidebar(faculty){
  const initials = faculty.name.split(" ").map(w=>w[0]).slice(0,2).join("");
  const av = document.getElementById("side-avatar");
  const nm = document.getElementById("side-name");
  const dt = document.getElementById("side-dept");
  if (av) av.textContent = initials;
  if (nm) nm.textContent = faculty.name;
  if (dt) dt.textContent = faculty.dept;
}

/* ---------------- Dashboard ---------------- */
function renderFacultyDashboard(faculty){
  populateFacultySidebar(faculty);
  const wn = document.getElementById("welcome-name");
  if (wn) wn.textContent = `Welcome, ${faculty.name}`;

  const published = STUDENTS.filter(s => s.resultStatus === "Published").length;
  const draft = STUDENTS.filter(s => s.resultStatus === "Draft").length;
  const dashboardSubjects = STUDENTS.flatMap(student => {
    const configured = getSubjectsFor(student.branch, Number(student.semester));
    return configured.length ? configured : SUBJECTS.map(subject => ({ code: subject, name: subject }));
  }).filter((subject, index, subjects) => subjects.findIndex(item => item.code === subject.code) === index);

  const statGrid = document.getElementById("stat-grid");
  if (statGrid){
    statGrid.innerHTML = `
      <div class="stat-card"><div class="label">Total Students</div><div class="value">${STUDENTS.length}</div></div>
      <div class="stat-card"><div class="label">Total Subjects</div><div class="value">${dashboardSubjects.length}</div></div>
      <div class="stat-card"><div class="label">Results Published</div><div class="value">${published}</div><div class="delta up">Up to date</div></div>
      <div class="stat-card"><div class="label">Pending Results</div><div class="value">${draft}</div><div class="delta ${draft>0?'warn':'up'}">${draft>0?'Needs review':'All clear'}</div></div>
    `;
  }

  // Class-wide average per subject
  const chart = document.getElementById("perf-chart");
  if (chart){
    const avgs = dashboardSubjects.map(subject => {
      const totals = STUDENTS
        .filter(student => {
          const configured = getSubjectsFor(student.branch, Number(student.semester));
          return (configured.length ? configured : SUBJECTS.map(name => ({ code: name }))).some(item => item.code === subject.code);
        })
        .map(student => {
          const mark = student.marks[subject.code] || student.marks[subject.name] || {};
          return (mark.grade_points ?? 0) * 10;
        });
      const avg = totals.length ? Math.round(totals.reduce((a,b)=>a+b,0) / totals.length) : 0;
      return { subject: subject.name, avg };
    });
    chart.innerHTML = avgs.map(a => `
      <div class="chart-col">
        <div class="chart-bar-wrap"><div class="chart-bar" style="height:${a.avg}%;"><span class="tip">${a.avg}</span></div></div>
        <span class="name">${a.subject}</span>
      </div>
    `).join("");
  }

  const list = document.getElementById("activity-list");
  if (list){
    list.innerHTML = RECENT_ACTIVITIES.map(a => `
      <li><span class="adot"></span><div><p class="atext">${a.text}</p><p class="atime">${a.time}</p></div></li>
    `).join("");
  }
}

/* ---------------- Manage Students ---------------- */
function getFilteredStudents(){
  const q = (document.getElementById("search-input")?.value || "").toLowerCase().trim();
  const branch = document.getElementById("filter-branch")?.value || "";
  const year = document.getElementById("filter-year")?.value || "";
  const sem = document.getElementById("filter-semester")?.value || "";

  return STUDENTS.filter(s => {
    const matchesQ = !q || s.name.toLowerCase().includes(q) || s.roll.includes(q);
    const matchesBranch = !branch || s.branch === branch;
    const matchesYear = !year || s.year === year;
    const matchesSem = !sem || s.semester === sem;
    return matchesQ && matchesBranch && matchesYear && matchesSem;
  });
}

function marksSemesterLinks(student){
  const currentSemester = Math.max(1, Number(student.semester) || 1);
  const links = Array.from({ length: currentSemester }, (_, index) => {
    const semester = index + 1;
    return `<a class="btn btn-outline btn-sm" href="marks.html?roll=${encodeURIComponent(student.roll)}&semester=${semester}" title="Enter Semester ${semester} marks">Sem ${semester}</a>`;
  }).join("");
  return `<div class="marks-semester-links" style="display:flex; gap:4px; flex-wrap:wrap; align-items:center;"><span class="text-muted" style="font-size:12px;">Marks:</span>${links}</div>`;
}

function renderStudentsTable(){
  const tbody = document.getElementById("students-tbody");
  const emptyState = document.getElementById("students-empty");
  if (!tbody) return;

  const filtered = getFilteredStudents();
  const totalPages = Math.max(1, Math.ceil(filtered.length / PAGE_SIZE));
  if (currentPage > totalPages) currentPage = totalPages;
  const start = (currentPage - 1) * PAGE_SIZE;
  const pageItems = filtered.slice(start, start + PAGE_SIZE);

  if (filtered.length === 0){
    tbody.innerHTML = "";
    if (emptyState) emptyState.style.display = "block";
  } else {
    if (emptyState) emptyState.style.display = "none";
    tbody.innerHTML = pageItems.map(s => `
      <tr>
        <td class="cell-mono">${s.roll}</td>
        <td class="cell-name">${s.name}</td>
        <td>${s.branch}</td>
        <td>${s.year}</td>
        <td>${s.semester}</td>
        <td>
          <div class="row-actions">
            <button data-tip="View" onclick="viewStudent('${s.roll}')">👁</button>
            <button data-tip="Edit" onclick="openEditStudent('${s.roll}')">✎</button>
            ${marksSemesterLinks(s)}
            <button data-tip="Delete" class="danger" onclick="confirmDeleteStudent('${s.roll}')">🗑</button>
          </div>
        </td>
      </tr>
    `).join("");
  }

  const pageInfo = document.getElementById("page-info");
  if (pageInfo) pageInfo.textContent = `Showing ${filtered.length === 0 ? 0 : start+1}–${Math.min(start+PAGE_SIZE, filtered.length)} of ${filtered.length} students`;

  const pageBtns = document.getElementById("page-btns");
  if (pageBtns){
    let html = "";
    for (let i=1; i<=totalPages; i++){
      html += `<button class="${i===currentPage?'active':''}" onclick="goToPage(${i})">${i}</button>`;
    }
    pageBtns.innerHTML = html;
  }
}

function openEditStudent(roll){
  const student = findStudentByRoll(roll);
  const form = document.getElementById("edit-student-form");
  if (!student || !form) return;
  editingStudentRoll = roll;
  ["name","email","phone","dob","gender","branch","year","semester","academicYear"].forEach(name => { form.elements[name].value = student[name] ?? ""; });
  form.querySelectorAll(".field").forEach(clearFieldError);
  openModal("edit-student-modal");
}

async function handleEditStudent(event){
  event.preventDefault();
  const form = event.target;
  if (!validateStudentForm(form)){
    showToast("error", "Check the form", "Some fields need your attention.");
    return;
  }
  const updatedStudent = {
    name: form.elements.name.value.trim(), email: form.elements.email.value.trim(), phone: form.elements.phone.value.trim(),
    gender: form.elements.gender.value, branch: form.elements.branch.value, year: Number(form.elements.year.value),
    semester: Number(form.elements.semester.value), academicYear: form.elements.academicYear.value, dob: form.elements.dob.value
  };
  try {
    await apiRequest(`/students/${encodeURIComponent(editingStudentRoll)}`, { method: "PUT", body: JSON.stringify(updatedStudent) });
    const index = STUDENTS.findIndex(student => student.roll === editingStudentRoll);
    if (index >= 0) STUDENTS[index] = { ...STUDENTS[index], ...updatedStudent, year: String(updatedStudent.year), semester: String(updatedStudent.semester) };
    closeModal("edit-student-modal");
    renderStudentsTable();
    showToast("success", "Student updated", `${updatedStudent.name}'s details have been updated.`);
  } catch (error) { showToast("error", "Student not updated", error.message); }
}

function goToPage(p){ currentPage = p; renderStudentsTable(); }

function viewStudent(roll){
  const s = findStudentByRoll(roll);
  if (!s) return;
  const report = computeStudentReport(s);
  document.getElementById("view-modal-body").innerHTML = `
    <div style="display:grid; grid-template-columns:1fr 1fr; gap:10px 16px; font-size:13.5px; margin-bottom:14px;">
      <div><span class="text-muted">Roll Number</span><br><b>${s.roll}</b></div>
      <div><span class="text-muted">Name</span><br><b>${s.name}</b></div>
      <div><span class="text-muted">Branch</span><br><b>${s.branch}</b></div>
      <div><span class="text-muted">Year / Sem</span><br><b>${s.year} / ${s.semester}</b></div>
      <div><span class="text-muted">Email</span><br><b>${s.email}</b></div>
      <div><span class="text-muted">Phone</span><br><b>${s.phone}</b></div>
    </div>
    <div class="row-between" style="margin-bottom:0;">
      <span class="text-muted" style="font-size:13px;">Overall Result</span>
      <span class="badge ${report.status==='Pass'?'badge-success':'badge-danger'}">${report.percentage}% · ${report.status}</span>
    </div>
  `;
  openModal("view-modal");
}

function confirmDeleteStudent(roll){
  pendingDeleteRoll = roll;
  const s = findStudentByRoll(roll);
  document.getElementById("delete-modal-body").textContent = `This will permanently remove ${s.name} (Roll ${s.roll}) from the student list.`;
  openModal("delete-modal");
}
async function executeDeleteStudent(){
  try {
    await apiRequest(`/students/${encodeURIComponent(pendingDeleteRoll)}`, { method: "DELETE" });
    STUDENTS = STUDENTS.filter(s => s.roll !== pendingDeleteRoll);
    closeModal("delete-modal");
    renderStudentsTable();
    showToast("success", "Student removed", "The student record has been deleted.");
  } catch (error) { showToast("error", "Delete failed", error.message); }
}

async function initStudentsPage(){
  await loadStudents();
  ["search-input","filter-branch","filter-year","filter-semester"].forEach(id => {
    document.getElementById(id)?.addEventListener("input", () => { currentPage = 1; renderStudentsTable(); });
    document.getElementById(id)?.addEventListener("change", () => { currentPage = 1; renderStudentsTable(); });
  });
  renderStudentsTable();
}

/* ---------------- Add Student ---------------- */
function validateAddStudentForm(form){
  let valid = true;
  const required = ["roll","name","email","phone","dob","gender","branch","year","semester","academicYear"];
  required.forEach(name => {
    const input = form.elements[name];
    const field = input.closest(".field");
    clearFieldError(field);
    if (!input.value || !input.value.trim()){
      setFieldError(field, "This field is required.");
      valid = false;
    }
  });

  const emailInput = form.elements["email"];
  const emailField = emailInput.closest(".field");
  if (emailInput.value && !/^[^\s@]+@[^\s@]+\.[^\s@]+$/.test(emailInput.value)){
    setFieldError(emailField, "Enter a valid email address.");
    valid = false;
  }

  const phoneInput = form.elements["phone"];
  const phoneField = phoneInput.closest(".field");
  if (phoneInput.value && !/^\d{10}$/.test(phoneInput.value)){
    setFieldError(phoneField, "Enter a valid 10-digit phone number.");
    valid = false;
  }

  const rollInput = form.elements["roll"];
  const rollField = rollInput.closest(".field");
  if (rollInput.value && findStudentByRoll(rollInput.value)){
    setFieldError(rollField, "A student with this roll number already exists.");
    valid = false;
  }

  return valid;
}

function validateStudentForm(form){
  let valid = true;
  ["name","email","phone","dob","gender","branch","year","semester","academicYear"].forEach(name => {
    const input = form.elements[name];
    const field = input.closest(".field");
    clearFieldError(field);
    if (!input.value || !input.value.trim()) { setFieldError(field, "This field is required."); valid = false; }
  });
  const emailInput = form.elements.email;
  if (emailInput.value && !/^[^\s@]+@[^\s@]+\.[^\s@]+$/.test(emailInput.value)) { setFieldError(emailInput.closest(".field"), "Enter a valid email address."); valid = false; }
  const phoneInput = form.elements.phone;
  if (phoneInput.value && !/^\d{10}$/.test(phoneInput.value)) { setFieldError(phoneInput.closest(".field"), "Enter a valid 10-digit phone number."); valid = false; }
  return valid;
}

async function handleAddStudent(e){
  e.preventDefault();
  const form = e.target;
  if (!validateAddStudentForm(form)){
    showToast("error", "Check the form", "Some fields need your attention.");
    return;
  }
  const newStudent = {
    roll: form.elements["roll"].value.trim(),
    name: form.elements["name"].value.trim(),
    email: form.elements["email"].value.trim(),
    phone: form.elements["phone"].value.trim(),
    gender: form.elements["gender"].value,
    branch: form.elements["branch"].value,
    year: Number(form.elements["year"].value),
    semester: Number(form.elements["semester"].value),
    academicYear: form.elements["academicYear"].value,
    dob: form.elements["dob"].value
  };
  try {
    await apiRequest("/students", { method: "POST", body: JSON.stringify(newStudent) });
    showToast("success", "Student added", `${newStudent.name} has been added successfully.`);
    form.reset();
    setTimeout(() => { window.location.href = "students.html"; }, 700);
  } catch (error) { showToast("error", "Student not added", error.message); }
}

/* ---------------- Enter Marks ---------------- */
let marksTargetStudent = null;

async function initEnterMarksPage(){
  await loadStudents();
  const select = document.getElementById("student-select");
  if (!select) return;
  select.innerHTML = `<option value="">— Select a student —</option>` +
    STUDENTS.map(s => `<option value="${s.roll}">${s.roll} — ${s.name} (${s.branch})</option>`).join("");

  const params = new URLSearchParams(window.location.search);
  const presetRoll = params.get("roll");
  if (presetRoll){ select.value = presetRoll; loadMarksForm(presetRoll); }

  select.addEventListener("change", () => loadMarksForm(select.value));
  document.getElementById("marks-exam-select")?.addEventListener("change", () => {
    if (select.value) renderMarksEntry();
  });
}

async function loadMarksForm(roll){
  const wrap = document.getElementById("marks-form-wrap");
  const empty = document.getElementById("marks-empty");
  if (!roll){
    wrap.style.display = "none";
    empty.style.display = "block";
    return;
  }
  const params = new URLSearchParams(window.location.search);
  const requestedSemester = Number(params.get("semester"));
  const hasRequestedSemester = Number.isInteger(requestedSemester) && requestedSemester > 0;
  const semester = hasRequestedSemester ? requestedSemester : undefined;
  marksTargetStudent = await loadStudent(roll, semester);
  wrap.style.display = "block";
  empty.style.display = "none";

  document.getElementById("marks-ay-select").value = marksTargetStudent.academicYear;
  const semesterContext = document.getElementById("marks-semester-context");
  if (semesterContext) semesterContext.textContent = `Showing Semester ${marksTargetStudent.semester}${hasRequestedSemester ? "" : " (current)"}`;

  renderMarksEntry();
}

function renderMarksEntry(){
  if (!marksTargetStudent) return;
  const semester = Number(marksTargetStudent.semester);
  const exam = document.getElementById("marks-exam-select")?.value || "ie1";
  const configured = getSubjectsFor(marksTargetStudent.branch, semester);
  const subjects = configured.length ? configured : SUBJECTS.map(subject => ({ code: subject, name: subject, credits: 0, hasInternals: true }));
  const visible = exam === "ie1" || exam === "ie2" ? subjects.filter(subject => subject.hasInternals) : subjects;
  const tbody = document.getElementById("enter-marks-tbody");
  tbody.innerHTML = visible.map(subject => {
    const mark = marksTargetStudent.marks[subject.code] || {};
    const configuredCredits = subject.credits > 0 ? subject.credits : null;
    const credits = mark.credits ?? configuredCredits ?? "";
    const value = mark[exam] ?? "";
    const inputAttributes = exam === "grade_points" ? 'min="0" max="10" step="0.1"' : 'min="0" step="0.1"';
    const editor = `<input type="number" class="input" style="width:110px;" ${inputAttributes} data-role="value" value="${value}">`;
    const creditsEditor = exam === "credits" ? `<input type="number" class="input" style="width:90px;" min="0" step="0.1" data-role="value" value="${credits}">` : (credits === "" ? "—" : credits);
    return `<tr data-subject="${subject.code}"><td class="cell-name">${subject.code} · ${subject.name}</td><td>${creditsEditor}</td><td>${exam === "ie1" ? editor : (mark.ie1 ?? "—")}</td><td>${exam === "ie2" ? editor : (mark.ie2 ?? "—")}</td><td>${exam === "grade_points" ? editor : (mark.grade_points ?? "—")}</td></tr>`;
  }).join("");
}
function clamp(v, min, max){ return Math.max(min, Math.min(max, v)); }

async function saveMarks(){
  if (!marksTargetStudent) return;
  const exam = document.getElementById("marks-exam-select")?.value || "ie1";
  const marks = {};
  document.querySelectorAll("#enter-marks-tbody tr").forEach(row => {
    const subj = row.dataset.subject;
    const input = row.querySelector('[data-role="value"]');
    if (input && input.value !== "") marks[subj] = { [exam]: Number(input.value) };
  });
  if (!Object.keys(marks).length){ showToast("error", "Nothing to save", "Enter at least one value first."); return; }
  try {
    await apiRequest(`/marks/${encodeURIComponent(marksTargetStudent.roll)}`, { method: "PUT", body: JSON.stringify({ semester: Number(marksTargetStudent.semester), ...marks }) });
    showToast("success", "Marks saved", `Marks for ${marksTargetStudent.name} have been updated.`);
  } catch (error) { showToast("error", "Marks not saved", error.message); }
}

/* ---------------- Manage Results ---------------- */
function renderResultsTable(){
  const tbody = document.getElementById("results-tbody");
  if (!tbody) return;
  const q = (document.getElementById("results-search")?.value || "").toLowerCase().trim();
  const sem = document.getElementById("results-sem-filter")?.value || "";

  const filtered = STUDENTS.filter(s => {
    const matchesQ = !q || s.name.toLowerCase().includes(q) || s.roll.includes(q);
    const matchesSem = !sem || s.semester === sem;
    return matchesQ && matchesSem;
  });

  const empty = document.getElementById("results-empty");
  if (filtered.length === 0){
    tbody.innerHTML = "";
    if (empty) empty.style.display = "block";
    return;
  }
  if (empty) empty.style.display = "none";

  tbody.innerHTML = filtered.map(s => {
    const report = computeStudentReport(s);
    return `
      <tr>
        <td class="cell-mono">${s.roll}</td>
        <td class="cell-name">${s.name}</td>
        <td>Semester ${s.semester}</td>
        <td class="cell-mono">${report.percentage === null ? "Incomplete" : `${report.percentage}%`}</td>
        <td><span class="badge ${report.status==='Pass'?'badge-success':report.status==='Incomplete'?'badge-warning':'badge-danger'}">${report.status}</span></td>
        <td><span class="badge ${s.resultStatus==='Published'?'badge-success':'badge-warning'}">${s.resultStatus}</span></td>
        <td>
          <div class="row-actions">
            ${marksSemesterLinks(s)}
            ${s.resultStatus === "Published"
              ? `<button data-tip="Unpublish" onclick="toggleResultStatus('${s.roll}')">⬇</button>`
              : `<button data-tip="Publish" onclick="toggleResultStatus('${s.roll}')">⬆</button>`}
          </div>
        </td>
      </tr>
    `;
  }).join("");
}

async function toggleResultStatus(roll){
  const s = findStudentByRoll(roll);
  if (!s) return;
  const report = computeStudentReport(s);
  if (report.status === "Incomplete"){
    showToast("error", "Result incomplete", "Assign grade points for every subject before publishing this result.");
    return;
  }
  try {
    await apiRequest(`/results/${encodeURIComponent(roll)}`, { method: "POST", body: JSON.stringify({ semester: Number(s.semester), totalMarks: report.totalMarks, percentage: report.percentage ?? 0, grade: "Published", facultyId: Session.get()?.id }) });
    s.resultStatus = "Published";
    renderResultsTable();
    showToast("success", "Result published", `${s.name}'s result is now published.`);
  } catch (error) { showToast("error", "Result update failed", error.message); }
}

async function initResultsPage(){
  await loadStudents();
  ["results-search","results-sem-filter"].forEach(id => {
    document.getElementById(id)?.addEventListener("input", renderResultsTable);
    document.getElementById(id)?.addEventListener("change", renderResultsTable);
  });
  renderResultsTable();
}

/* ---------------- Search Student ---------------- */
async function runStudentSearch(){
  if (!STUDENTS.length) await loadStudents();
  const roll = document.getElementById("s-roll").value.toLowerCase().trim();
  const name = document.getElementById("s-name").value.toLowerCase().trim();
  const branch = document.getElementById("s-branch").value;

  const results = STUDENTS.filter(s =>
    (!roll || s.roll.includes(roll)) &&
    (!name || s.name.toLowerCase().includes(name)) &&
    (!branch || s.branch === branch)
  );

  const wrap = document.getElementById("search-results");
  const empty = document.getElementById("search-empty");
  const hint = document.getElementById("search-hint");
  if (hint) hint.style.display = "none";

  if (results.length === 0){
    wrap.innerHTML = "";
    empty.style.display = "block";
    return;
  }
  empty.style.display = "none";
  wrap.innerHTML = results.map(s => `
    <div class="card" style="margin-bottom:12px;">
      <div class="card-pad" style="display:flex; justify-content:space-between; align-items:center; gap:16px; flex-wrap:wrap;">
        <div>
          <p class="cell-name" style="font-size:15px;">${s.name}</p>
          <p class="text-muted" style="font-size:13px; margin-top:4px;">Roll ${s.roll} · ${s.branch} · Year ${s.year}, Sem ${s.semester}</p>
        </div>
        <div style="display:flex; gap:8px;">
          <button class="btn btn-outline btn-sm" onclick="viewStudent('${s.roll}')">View</button>
          ${marksSemesterLinks(s)}
        </div>
      </div>
    </div>
  `).join("");
}
