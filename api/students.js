// Vercel Serverless Function — /api/students
// Mirrors C++ StudentManager — GET (list/search), POST (create), PUT (update), DELETE (soft-delete)

const STUDENTS = [
  { id: "std-001", roll_number: "CS2026-101", full_name: "Arjun Sharma",    division: "A", department: "Computer Science & Engineering", email: "arjun.sharma@college.edu",    semester: 6, is_active: true,  created_at: "2026-07-01T08:00:00Z" },
  { id: "std-002", roll_number: "CS2026-102", full_name: "Priya Patel",     division: "A", department: "Computer Science & Engineering", email: "priya.patel@college.edu",     semester: 6, is_active: true,  created_at: "2026-07-01T08:00:00Z" },
  { id: "std-003", roll_number: "CS2026-103", full_name: "Ravi Kumar",      division: "A", department: "Computer Science & Engineering", email: "ravi.kumar@college.edu",      semester: 6, is_active: true,  created_at: "2026-07-01T08:00:00Z" },
  { id: "std-004", roll_number: "CS2026-104", full_name: "Sneha Reddy",     division: "A", department: "Computer Science & Engineering", email: "sneha.reddy@college.edu",     semester: 6, is_active: true,  created_at: "2026-07-01T08:00:00Z" },
  { id: "std-005", roll_number: "CS2026-105", full_name: "Aditya Singh",    division: "A", department: "Computer Science & Engineering", email: "aditya.singh@college.edu",    semester: 6, is_active: true,  created_at: "2026-07-01T08:00:00Z" },
  { id: "std-006", roll_number: "CS2026-106", full_name: "Kavya Nair",      division: "B", department: "Computer Science & Engineering", email: "kavya.nair@college.edu",      semester: 6, is_active: true,  created_at: "2026-07-01T08:00:00Z" },
  { id: "std-007", roll_number: "CS2026-107", full_name: "Rahul Mehta",     division: "B", department: "Computer Science & Engineering", email: "rahul.mehta@college.edu",     semester: 6, is_active: true,  created_at: "2026-07-01T08:00:00Z" },
  { id: "std-008", roll_number: "CS2026-108", full_name: "Ananya Joshi",    division: "B", department: "Computer Science & Engineering", email: "ananya.joshi@college.edu",    semester: 6, is_active: true,  created_at: "2026-07-01T08:00:00Z" },
  { id: "std-009", roll_number: "CS2026-109", full_name: "Vikram Desai",    division: "B", department: "Computer Science & Engineering", email: "vikram.desai@college.edu",    semester: 6, is_active: true,  created_at: "2026-07-01T08:00:00Z" },
  { id: "std-010", roll_number: "CS2026-110", full_name: "Pooja Iyer",      division: "B", department: "Computer Science & Engineering", email: "pooja.iyer@college.edu",      semester: 6, is_active: true,  created_at: "2026-07-01T08:00:00Z" },
  { id: "std-011", roll_number: "CS2026-111", full_name: "Manish Gupta",    division: "A", department: "Computer Science & Engineering", email: "manish.gupta@college.edu",    semester: 6, is_active: false, created_at: "2026-07-01T08:00:00Z" },
  { id: "std-012", roll_number: "CS2026-112", full_name: "Deepika Verma",   division: "A", department: "Computer Science & Engineering", email: "deepika.verma@college.edu",   semester: 6, is_active: false, created_at: "2026-07-01T08:00:00Z" }
];

let students = [...STUDENTS];
let nextId = 13;

export default function handler(req, res) {
  res.setHeader('Access-Control-Allow-Origin', '*');
  res.setHeader('Access-Control-Allow-Methods', 'GET, POST, PUT, DELETE, OPTIONS');
  res.setHeader('Access-Control-Allow-Headers', 'Content-Type');

  if (req.method === 'OPTIONS') return res.status(204).end();

  // GET /api/students or /api/students?search=X or /api/students/:id
  if (req.method === 'GET') {
    const { search, include_inactive, id } = req.query;

    // Individual student by id (routed as /api/students?id=xxx from vercel.json)
    if (id) {
      const student = students.find(s => s.id === id);
      if (!student) return res.status(404).json({ success: false, status: 404, error: `Student '${id}' not found.` });
      return res.status(200).json({ success: true, status: 200, data: student });
    }

    // Search
    if (search) {
      const q = search.toLowerCase();
      const results = students.filter(s =>
        s.is_active && (
          s.full_name.toLowerCase().includes(q) ||
          s.roll_number.toLowerCase().includes(q) ||
          s.email.toLowerCase().includes(q)
        )
      );
      return res.status(200).json({ success: true, status: 200, data: results });
    }

    // All students
    const showInactive = include_inactive !== 'false' && include_inactive !== '0';
    const list = showInactive ? students : students.filter(s => s.is_active);
    return res.status(200).json({ success: true, status: 200, data: list });
  }

  // POST — register new student
  if (req.method === 'POST') {
    const body = req.body;
    if (!body || !body.roll_number || !body.full_name) {
      return res.status(400).json({ success: false, status: 400, error: "Missing required fields: roll_number and full_name are required." });
    }

    const existing = students.find(s => s.roll_number.toLowerCase() === body.roll_number.toLowerCase());
    if (existing) {
      return res.status(409).json({ success: false, status: 409, error: `A student with roll number '${body.roll_number}' already exists.` });
    }

    const newStudent = {
      id: `std-0${String(nextId++).padStart(2, '0')}`,
      roll_number: body.roll_number,
      full_name: body.full_name,
      division: body.division || "A",
      department: body.department || "Computer Science & Engineering",
      email: body.email || "",
      semester: body.semester || 1,
      is_active: true,
      created_at: new Date().toISOString()
    };
    students.push(newStudent);
    return res.status(201).json({ success: true, status: 201, data: newStudent, message: "Student registered successfully." });
  }

  // PUT — update student
  if (req.method === 'PUT') {
    const { id } = req.query;
    const idx = students.findIndex(s => s.id === id);
    if (idx === -1) return res.status(404).json({ success: false, status: 404, error: `Student '${id}' not found.` });
    students[idx] = { ...students[idx], ...req.body, id: students[idx].id };
    return res.status(200).json({ success: true, status: 200, data: students[idx], message: "Student updated successfully." });
  }

  // DELETE — soft delete
  if (req.method === 'DELETE') {
    const { id } = req.query;
    const idx = students.findIndex(s => s.id === id);
    if (idx === -1) return res.status(404).json({ success: false, status: 404, error: `Student '${id}' not found.` });
    students[idx].is_active = false;
    return res.status(200).json({ success: true, status: 200, data: { id, status: "deactivated" }, message: "Student record deactivated (historical data preserved)." });
  }

  return res.status(405).json({ success: false, status: 405, error: "Method not allowed." });
}
