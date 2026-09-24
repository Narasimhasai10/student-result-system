/* ============================================================
   SHARED UI BEHAVIOUR — sidebar, toasts, modals, password toggle
   Used across every dashboard page in both domains.
   ============================================================ */

/* ---------- Mobile sidebar ---------- */
function initSidebar(){
  const sidebar = document.querySelector(".sidebar");
  const overlay = document.querySelector(".sidebar-overlay");
  const hamburger = document.querySelector(".hamburger");
  const closeBtn = document.querySelector(".sidebar-close");

  function open(){ sidebar?.classList.add("open"); overlay?.classList.add("open"); }
  function close(){ sidebar?.classList.remove("open"); overlay?.classList.remove("open"); }

  hamburger?.addEventListener("click", open);
  closeBtn?.addEventListener("click", close);
  overlay?.addEventListener("click", close);
}

/* ---------- Toasts ---------- */
function ensureToastStack(){
  let stack = document.querySelector(".toast-stack");
  if (!stack){
    stack = document.createElement("div");
    stack.className = "toast-stack";
    document.body.appendChild(stack);
  }
  return stack;
}

function showToast(type, title, message){
  const stack = ensureToastStack();
  const toast = document.createElement("div");
  toast.className = `toast ${type}`;
  const icon = type === "success" ? "✓" : type === "error" ? "✕" : "ℹ";
  toast.innerHTML = `
    <span class="ticon">${icon}</span>
    <div>
      <strong>${title}</strong>
      <p>${message || ""}</p>
    </div>
    <button class="close-t" aria-label="Dismiss">✕</button>
  `;
  stack.appendChild(toast);
  toast.querySelector(".close-t").addEventListener("click", () => toast.remove());
  setTimeout(() => toast.remove(), type === "notify" ? 3000 : 4500);
}

/* ---------- Modals ---------- */
function openModal(id){ document.getElementById(id)?.classList.add("open"); }
function closeModal(id){ document.getElementById(id)?.classList.remove("open"); }

/* ---------- Password show/hide ---------- */
function initPasswordToggles(){
  document.querySelectorAll(".toggle-pw").forEach(btn => {
    btn.addEventListener("click", () => {
      const input = btn.closest(".password-wrap").querySelector("input");
      const isPw = input.type === "password";
      input.type = isPw ? "text" : "password";
      btn.textContent = isPw ? "Hide" : "Show";
    });
  });
}

/* ---------- Small field-error helper ---------- */
function setFieldError(fieldEl, message){
  fieldEl.classList.add("has-error");
  const err = fieldEl.querySelector(".error-text");
  if (err) err.textContent = message;
  fieldEl.querySelector(".input")?.classList.add("is-invalid");
}
function clearFieldError(fieldEl){
  fieldEl.classList.remove("has-error");
  fieldEl.querySelector(".input")?.classList.remove("is-invalid");
}

/* ---------- Notifications ---------- */
function relativeNotificationTime(value){
  const timestamp = Date.parse(String(value).replace(" ", "T"));
  if (!Number.isFinite(timestamp)) return "Just now";
  const seconds = Math.max(0, Math.floor((Date.now() - timestamp) / 1000));
  if (seconds < 60) return "Just now";
  const minutes = Math.floor(seconds / 60);
  if (minutes < 60) return `${minutes}m ago`;
  const hours = Math.floor(minutes / 60);
  if (hours < 24) return `${hours}h ago`;
  return `${Math.floor(hours / 24)}d ago`;
}

function initNotifications(){
  const session = Session.get();
  const bell = document.querySelector('.icon-btn[data-tip="Notifications"]');
  if (!session || !bell || typeof loadNotifications !== "function") return;

  const type = session.role === "student" ? "student" : "faculty";
  const id = type === "student" ? session.roll : session.id;
  const seenKey = `srms_notif_seen_${type}_${id}`;
  let panel = document.querySelector(".notification-panel");
  if (!panel){
    panel = document.createElement("div");
    panel.className = "notification-panel";
    panel.innerHTML = '<div class="notification-head"><strong>Notifications</strong><button type="button" class="notification-read-all">Mark all read</button></div><div class="notification-list"></div>';
    bell.parentElement.style.position = "relative";
    bell.parentElement.appendChild(panel);
  }

  const list = panel.querySelector(".notification-list");
  const dot = bell.querySelector(".dot");
  const render = notifications => {
    const unread = notifications.filter(notification => !notification.isRead);
    if (dot) dot.style.display = unread.length ? "block" : "none";
    list.innerHTML = notifications.length ? notifications.map(notification => `
      <button class="notification-item ${notification.isRead ? "" : "unread"}" data-notification-id="${notification.id}">
        <strong>${notification.title}</strong><span>${notification.message}</span><small>${relativeNotificationTime(notification.createdAt)}</small>
      </button>
    `).join("") : '<div class="notification-empty">No notifications yet.</div>';
    list.querySelectorAll("[data-notification-id]").forEach(item => item.addEventListener("click", async () => {
      await markNotificationRead(item.dataset.notificationId).catch(() => {});
      item.classList.remove("unread");
      refresh();
    }));
    return notifications;
  };
  const notifyNew = notifications => {
    const highestSeen = Number(localStorage.getItem(seenKey) || 0);
    notifications.filter(notification => Number(notification.id) > highestSeen).sort((a, b) => a.id - b.id).forEach(notification => showToast("notify", notification.title, notification.message));
    const highest = notifications.reduce((max, notification) => Math.max(max, Number(notification.id)), highestSeen);
    if (highest > highestSeen) localStorage.setItem(seenKey, String(highest));
  };
  async function refresh(){
    try {
      const notifications = await loadNotifications(type, id);
      notifyNew(notifications);
      render(notifications);
    } catch (error) { /* Notification polling should not interrupt the page. */ }
  }
  bell.addEventListener("click", async event => {
    event.stopPropagation();
    panel.classList.toggle("open");
    if (panel.classList.contains("open")){
      await refresh();
      await markAllNotificationsRead(type, id).catch(() => {});
      if (dot) dot.style.display = "none";
    }
  });
  panel.querySelector(".notification-read-all").addEventListener("click", async () => {
    await markAllNotificationsRead(type, id).catch(() => {});
    await refresh();
  });
  document.addEventListener("click", event => {
    if (!panel.contains(event.target) && event.target !== bell) panel.classList.remove("open");
  });
  refresh();
  setInterval(refresh, 15000);
}

/* ---------- Init on every page ---------- */
document.addEventListener("DOMContentLoaded", () => {
  initSidebar();
  initPasswordToggles();
  initNotifications();
});
