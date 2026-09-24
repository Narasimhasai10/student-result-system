async function handleStudentLogin(event) {
  event.preventDefault();
  const roll = document.getElementById("roll").value.trim();
  const password = document.getElementById("password").value;
  if (!roll || !password) return showToast("error", "Login failed", "Enter your roll number and password.");
  try {
    const result = await apiRequest("/auth/login/student", { method: "POST", body: JSON.stringify({ roll, password }) });
    Session.set({ role: "student", roll: result.student.roll, token: result.token });
    window.location.href = "dashboard.html";
  } catch (error) { showToast("error", "Login failed", error.message); }
}

async function handleFacultyLogin(event) {
  event.preventDefault();
  const id = document.getElementById("facultyId").value.trim();
  const password = document.getElementById("password").value;
  if (!id || !password) return showToast("error", "Login failed", "Enter your faculty ID and password.");
  try {
    const result = await apiRequest("/auth/login/faculty", { method: "POST", body: JSON.stringify({ id, password }) });
    Session.set({ role: "faculty", id: result.faculty.id, token: result.token });
    window.location.href = "dashboard.html";
  } catch (error) { showToast("error", "Login failed", error.message); }
}

async function requireStudent() {
  const session = Session.get();
  if (!session || session.role !== "student") { window.location.href = "login.html"; return null; }
  try { return await loadStudent(session.roll); }
  catch (error) { Session.clear(); window.location.href = "login.html"; return null; }
}

async function requireFaculty() {
  const session = Session.get();
  if (!session || session.role !== "faculty") { window.location.href = "login.html"; return null; }
  try { const faculty = await loadFaculty(session.id); FACULTY = [faculty]; return faculty; }
  catch (error) { Session.clear(); window.location.href = "login.html"; return null; }
}

function logout() { Session.clear(); window.location.href = "login.html"; }
async function changeStudentPassword() {
  const session = Session.get();
  const currentPassword = window.prompt("Current Password:");
  const newPassword = window.prompt("New Password:");
  const confirmPassword = window.prompt("Confirm New Password:");
  if (!session || !currentPassword || !newPassword || newPassword !== confirmPassword) return showToast("error", "Password not changed", "Check the current password and confirmation.");
  try {
    await apiRequest("/auth/password/student", { method: "PUT", body: JSON.stringify({ roll: session.roll, currentPassword, newPassword }) });
    showToast("success", "Password changed", "Your new password is active.");
  } catch (error) { showToast("error", "Password not changed", error.message); }
}
async function changeFacultyPassword() {
  const session = Session.get();
  const currentPassword = window.prompt("Current Password:");
  const newPassword = window.prompt("New Password:");
  const confirmPassword = window.prompt("Confirm New Password:");
  if (!session || !currentPassword || !newPassword || newPassword !== confirmPassword) return showToast("error", "Password not changed", "Check the current password and confirmation.");
  try {
    await apiRequest("/auth/password/faculty", { method: "PUT", body: JSON.stringify({ id: session.id, currentPassword, newPassword }) });
    showToast("success", "Password changed", "Your new password is active.");
  } catch (error) { showToast("error", "Password not changed", error.message); }
}
function initLogoutButtons() {
  document.querySelectorAll("[data-logout]").forEach(button => button.addEventListener("click", event => {
    event.preventDefault();
    openModal("logout-modal");
  }));
}
document.addEventListener("DOMContentLoaded", initLogoutButtons);
