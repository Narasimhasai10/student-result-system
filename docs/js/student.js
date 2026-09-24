/* ============================================================
   STUDENT DOMAIN — page rendering logic
   [BACKEND] Each render* function below stands in for a call
   to a real API endpoint (e.g. GET /api/students/:roll).
   ============================================================ */

function populateSidebarUser(student){
  const initials = student.name.split(" ").map(w=>w[0]).slice(0,2).join("");
  const av = document.getElementById("side-avatar");
  const nm = document.getElementById("side-name");
  const rl = document.getElementById("side-roll");
  if (av) av.textContent = initials;
  if (nm) nm.textContent = student.name;
  if (rl) rl.textContent = `Roll No. ${student.roll}`;
}

function renderStudentDashboard(student){
  populateSidebarUser(student);
  const report = computeStudentReport(student);

  const wn = document.getElementById("welcome-name");
  if (wn) wn.textContent = `Welcome, ${student.name.split(" ")[0]}`;
  const wy = document.getElementById("welcome-year");
  if (wy) wy.textContent = student.academicYear;

  const statGrid = document.getElementById("stat-grid");
  if (statGrid){
    statGrid.innerHTML = `
      <div class="stat-card"><div class="label">Current Semester</div><div class="value">${student.semester}</div></div>
      <div class="stat-card"><div class="label">Total Subjects</div><div class="value">${report.rows.length}</div></div>
      <div class="stat-card"><div class="label">${report.sgpa !== undefined ? "SGPA" : "Percentage"}</div><div class="value">${report.sgpa ?? (report.percentage === null ? "Incomplete" : `${report.percentage}%`)}</div><div class="delta ${report.sgpa !== null && report.sgpa !== undefined ? 'up':'warn'}">${report.sgpa !== null && report.sgpa !== undefined?'Recorded':'Pending grades'}</div></div>
      <div class="stat-card"><div class="label">Current SGPA</div><div class="value">${report.sgpa ?? "Incomplete"}</div></div>
      <div class="stat-card"><div class="label">Result Status</div><div class="value" style="font-size:18px; color:${report.status==='Pass'?'var(--success)':'var(--danger)'};">${report.status}</div></div>
    `;
  }

  const chart = document.getElementById("perf-chart");
  if (chart){
    chart.innerHTML = report.rows.map(r => `
      <div class="chart-col">
        <div class="chart-bar-wrap">
          <div class="chart-bar" style="height:${r.total}%;"><span class="tip">${r.total}</span></div>
        </div>
        <span class="name">${r.subject}</span>
      </div>
    `).join("");
  }

  const badge = document.getElementById("result-badge");
  if (badge){
    badge.textContent = student.resultStatus;
    badge.className = "badge " + (student.resultStatus === "Published" ? "badge-success" : "badge-warning");
  }

  const recent = document.getElementById("recent-result");
  if (recent){
    recent.innerHTML = `
      <div class="row-between" style="margin-bottom:14px;">
        <span class="text-muted" style="font-size:13px;">Semester ${student.semester} · ${student.academicYear}</span>
        <span class="badge ${report.status==='Pass'?'badge-success':'badge-danger'}">${report.status}</span>
      </div>
      <div style="display:flex; justify-content:space-between; font-size:14px; padding:10px 0; border-top:1px solid var(--line);">
        <span class="text-muted">Percentage</span><span class="cell-mono">${report.percentage === null ? "Incomplete" : `${report.percentage}%`}</span>
      </div>
      <div style="display:flex; justify-content:space-between; font-size:14px; padding:10px 0; border-top:1px solid var(--line);">
        <span class="text-muted">SGPA</span><span class="cell-mono">${report.sgpa ?? "Incomplete"}</span>
      </div>
    `;
  }
}

function renderStudentProfile(student){
  populateSidebarUser(student);
  const grid = document.getElementById("profile-grid");
  if (!grid) return;
  const fields = [
    ["Student ID", student.id],
    ["Roll Number", student.roll],
    ["Full Name", student.name],
    ["Email Address", student.email],
    ["Phone Number", student.phone],
    ["Branch", student.branch],
    ["Year", student.year],
    ["Semester", student.semester],
    ["Date of Birth", student.dob],
  ];
  grid.innerHTML = fields.map(([label,val]) => `
    <div>
      <span class="label" style="display:block; font-size:11.5px; color:var(--muted); text-transform:uppercase; letter-spacing:.05em; font-weight:600; margin-bottom:6px;">${label}</span>
      <span style="font-size:15px; color:var(--navy-900); font-weight:600;">${val}</span>
    </div>
  `).join("");

  const initials = student.name.split(" ").map(w=>w[0]).slice(0,2).join("");
  const av = document.getElementById("profile-avatar");
  const nm = document.getElementById("profile-name");
  const meta = document.getElementById("profile-meta");
  if (av) av.textContent = initials;
  if (nm) nm.textContent = student.name;
  if (meta) meta.textContent = `${student.branch} · Year ${student.year} · Semester ${student.semester}`;
}

function renderStudentMarks(student){
  populateSidebarUser(student);
  const report = computeStudentReport(student);

  const semSel = document.getElementById("semester-select");
  if (semSel) semSel.value = student.semester;
  const yearLabel = document.getElementById("marks-year");
  if (yearLabel) yearLabel.textContent = student.academicYear;

  const tbody = document.getElementById("marks-tbody");
  if (tbody){
    tbody.innerHTML = report.rows.map(r => `
      <tr>
        <td class="cell-name">${r.subject}</td>
        <td class="cell-mono">${r.credits ?? "—"}</td>
        <td class="cell-mono">${r.ie1 ?? "—"}</td>
        <td class="cell-mono">${r.ie2 ?? "—"}</td>
        <td class="cell-mono">${r.grade_points ?? "—"}</td>
      </tr>
    `).join("");
  }

  const setText = (id, val) => { const el = document.getElementById(id); if (el) el.textContent = val; };
  setText("summary-percentage", report.percentage === null ? "Incomplete" : `${report.percentage}%`);
  setText("summary-sgpa", report.sgpa ?? "Incomplete");
  const statusEl = document.getElementById("summary-status");
  if (statusEl){
    statusEl.textContent = report.status;
    statusEl.className = "badge " + (report.status === "Pass" ? "badge-success" : "badge-danger");
  }
}

function renderStudentResult(student){
  populateSidebarUser(student);
  const report = computeStudentReport(student);

  const setText = (id, val) => { const el = document.getElementById(id); if (el) el.textContent = val; };
  setText("r-name", student.name);
  setText("r-roll", student.roll);
  setText("r-branch", student.branch);
  setText("r-semester", `Semester ${student.semester}`);
  setText("r-year", student.academicYear);
  setText("r-id", student.id);

  const tbody = document.getElementById("result-tbody");
  if (tbody){
    tbody.innerHTML = report.rows.map(r => `
      <tr>
        <td class="cell-name">${r.subject}</td>
        <td class="cell-mono">${r.credits ?? "—"}</td>
        <td class="cell-mono">${r.ie1 ?? "—"}</td>
        <td class="cell-mono">${r.ie2 ?? "—"}</td>
        <td class="cell-mono"><b>${r.grade_points ?? "—"}</b></td>
        <td>${r.grade_points !== null && r.grade_points !== undefined ? (r.pass ? '<span class="badge badge-success">Pass</span>' : '<span class="badge badge-danger">Fail</span>') : '<span class="badge badge-warning">Pending</span>'}</td>
      </tr>
    `).join("");
  }

  setText("summary-percent", report.percentage === null ? "Incomplete" : `${report.percentage}%`);
  setText("summary-sgpa", report.sgpa ?? "Incomplete");
  const resultStatus = document.getElementById("summary-result-status");
  if (resultStatus){
    resultStatus.textContent = report.status;
    resultStatus.className = "badge " + (report.status === "Pass" ? "badge-success" : "badge-danger");
  }

  const band = document.getElementById("status-band");
  if (band){
    band.textContent = report.status === "Pass" ? "Result: Pass" : "Result: Fail";
    band.className = "result-status-band " + (report.status === "Pass" ? "pass" : "fail");
  }
}

function printResult(){ window.print(); }
function downloadResult(){
  showToast("success", "Download started", "Your marksheet is being prepared as a PDF (demo only).");
}
