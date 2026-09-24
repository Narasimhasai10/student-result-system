/* ============================================================
   THEME TOGGLE — Dark/Light mode functionality
   ============================================================ */

const savedTheme = localStorage.getItem('theme');
if (savedTheme === 'dark' || savedTheme === 'light') {
  document.documentElement.setAttribute('data-theme', savedTheme);
}

function initThemeToggle(){
  const themeToggle = document.getElementById('themeToggle');
  const html = document.documentElement;
  
  // Get saved theme or default to light
  const activeTheme = savedTheme || 'light';
  html.setAttribute('data-theme', activeTheme);
  updateThemeIcon(activeTheme);
  
  // Toggle theme
  if(themeToggle){
    themeToggle.addEventListener('click', function(){
      const currentTheme = html.getAttribute('data-theme');
      const newTheme = currentTheme === 'dark' ? 'light' : 'dark';
      
      html.setAttribute('data-theme', newTheme);
      localStorage.setItem('theme', newTheme);
      updateThemeIcon(newTheme);
    });
  }
}

function updateThemeIcon(theme){
  const themeToggle = document.getElementById('themeToggle');
  if(themeToggle){
    themeToggle.textContent = theme === 'dark' ? '☀️' : '🌙';
    themeToggle.title = theme === 'dark' ? 'Switch to Light Mode' : 'Switch to Dark Mode';
  }
}

// Initialize on page load
if(document.readyState === 'loading'){
  document.addEventListener('DOMContentLoaded', initThemeToggle);
} else {
  initThemeToggle();
}
