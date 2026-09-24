CREATE DATABASE IF NOT EXISTS student_result_system
  CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
USE student_result_system;

CREATE TABLE IF NOT EXISTS students (
  id INT AUTO_INCREMENT PRIMARY KEY,
  roll VARCHAR(64) NOT NULL UNIQUE,
  password VARCHAR(255) NOT NULL,
  name VARCHAR(150) NOT NULL,
  email VARCHAR(255) NOT NULL,
  phone VARCHAR(32) NOT NULL,
  gender VARCHAR(32) NOT NULL,
  branch VARCHAR(100) NOT NULL,
  year INT NOT NULL,
  semester INT NOT NULL,
  academic_year VARCHAR(32) NOT NULL,
  dob DATE NULL,
  result_status VARCHAR(32) NOT NULL DEFAULT 'Draft'
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS faculty (
  id VARCHAR(64) PRIMARY KEY,
  password VARCHAR(255) NOT NULL,
  name VARCHAR(150) NOT NULL,
  email VARCHAR(255) NOT NULL,
  department VARCHAR(150) NOT NULL
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS marks (
  id INT AUTO_INCREMENT PRIMARY KEY,
  student_roll VARCHAR(64) NOT NULL,
  semester INT NOT NULL,
  subject VARCHAR(150) NOT NULL,
  ie1 INT NULL,
  ie2 INT NULL,
  sem INT NULL,
  credits DECIMAL(3,1) NULL,
  grade_points DECIMAL(3,1) NULL,
  UNIQUE KEY uq_mark (student_roll, semester, subject),
  CONSTRAINT fk_marks_student FOREIGN KEY (student_roll) REFERENCES students(roll) ON DELETE CASCADE
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS results (
  id INT AUTO_INCREMENT PRIMARY KEY,
  student_roll VARCHAR(64) NOT NULL,
  semester INT NOT NULL,
  total_marks DECIMAL(10,2) NOT NULL,
  percentage DECIMAL(5,2) NOT NULL,
  grade VARCHAR(8) NOT NULL,
  status VARCHAR(32) NOT NULL DEFAULT 'Draft',
  published_date TIMESTAMP NULL DEFAULT NULL,
  UNIQUE KEY uq_result (student_roll, semester),
  CONSTRAINT fk_results_student FOREIGN KEY (student_roll) REFERENCES students(roll) ON DELETE CASCADE
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS notifications (
  id INT AUTO_INCREMENT PRIMARY KEY,
  recipient_type VARCHAR(16) NOT NULL,
  recipient_id VARCHAR(64) NOT NULL,
  title VARCHAR(150) NOT NULL,
  message TEXT NOT NULL,
  is_read TINYINT(1) NOT NULL DEFAULT 0,
  created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
  INDEX idx_notifications_recipient (recipient_type, recipient_id, id)
) ENGINE=InnoDB;

INSERT INTO faculty (id, password, name, email, department) VALUES
  ('FAC001', 'webcap', 'Dr. Meera Nair', 'meera.nair@college.edu', 'Computer Science')
ON DUPLICATE KEY UPDATE name=VALUES(name), email=VALUES(email), department=VALUES(department);

INSERT INTO students (roll, password, name, email, phone, gender, branch, year, semester, academic_year, dob, result_status) VALUES
  ('25B11AI247', 'webcap', 'D L NARASIMHA SAI', '25b11ai247@adityauniversity.in', '1234567890', 'Male', 'AIML', 2, 3, '2026-27', '2007-12-10', 'Published'),
  ('25B11AI138', 'webcap', 'B SRIRAM', '25b11ai138@adityauniversity.in', '9876543211', 'MALE', 'AIML', 2, 3, '2026-27', '2007-06-20', 'Published'),
  ('25B11AI528', 'webcap', 'K V CHANIKYA', '25b11ai528@adityauniversity.in', '9876543212', 'Male', 'AIML', 2, 3, '2026-27', '2007-03-10', 'Published')
ON DUPLICATE KEY UPDATE name=VALUES(name), email=VALUES(email), branch=VALUES(branch), year=VALUES(year), semester=VALUES(semester);

INSERT INTO marks (student_roll, semester, subject, ie1, ie2, sem) VALUES
  ('25B11AI247', 3, 'Mathematics', 25, NULL, 60, NULL),
  ('25B11AI247', 3, 'Physics', 23, NULL, 55, NULL),
  ('25B11AI247', 3, 'C++', 24, NULL, 56, NULL),
  ('25B11AI247', 3, 'DBMS', 23, NULL, 54, NULL),
  ('25B11AI247', 3, 'English', 21, NULL, 50, NULL),
  ('25B11AI138', 3, 'C++', 26, NULL, 62, NULL),
  ('25B11AI138', 3, 'DBMS', 25, NULL, 60, NULL),
  ('25B11AI138', 3, 'Mathematics', 27, NULL, 64, NULL),
  ('25B11AI528', 3, 'C++', 22, NULL, 48, NULL),
  ('25B11AI528', 3, 'DBMS', 21, NULL, 46, NULL),
  ('25B11AI528', 3, 'Mathematics', 23, NULL, 50, NULL),
  ('25B11AI247', 1, '2501CS01', 15, 23, NULL, NULL),
  ('25B11AI247', 1, '2501EC95', 16, 24, NULL, NULL),
  ('25B11AI247', 1, '2501EE01', 13, 23, NULL, NULL),
  ('25B11AI247', 1, '2501MA01', 16, 24, NULL, NULL),
  ('25B11AI247', 2, '2501CS03', 16, 22, NULL, NULL),
  ('25B11AI247', 2, '2501CS71', 13, 16, NULL, NULL),
  ('25B11AI247', 2, '2501IT42', 16, 22, NULL, NULL),
  ('25B11AI247', 2, '2501MA02', 16, 22, NULL, NULL),
  ('25B11AI247', 2, '2501PH02', 13, 21, NULL, NULL)
ON DUPLICATE KEY UPDATE ie1=VALUES(ie1), ie2=VALUES(ie2);

INSERT INTO results (student_roll, semester, total_marks, percentage, grade, status, published_date) VALUES
  ('101', 2, 228, 76.00, 'B', 'Published', NOW()),
  ('102', 3, 264, 88.00, 'A', 'Published', NOW()),
  ('103', 2, 210, 70.00, 'B', 'Published', NOW())
ON DUPLICATE KEY UPDATE total_marks=VALUES(total_marks), percentage=VALUES(percentage), grade=VALUES(grade), status=VALUES(status), published_date=VALUES(published_date);
