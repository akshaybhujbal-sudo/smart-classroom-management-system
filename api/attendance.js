// Vercel Serverless Function — /api/attendance
// Mirrors C++ AttendanceManager — GET (by date/student), POST (mark), PUT (update)

const ATTENDANCE = [
  { id: "att-001", date: "2026-10-22", student_id: "std-001", student_roll: "CS2026-101", status: "PRESENT", recorded_at: "2026-10-22T09:05:00Z", remarks: "" },
  { id: "att-002", date: "2026-10-22", student_id: "std-002", student_roll: "CS2026-102", status: "PRESENT", recorded_at: "2026-10-22T09:05:00Z", remarks: "" },
  { id: "att-003", date: "2026-10-22", student_id: "std-003", student_roll: "CS2026-103", status: "PRESENT", recorded_at: "2026-10-22T09:05:00Z", remarks: "" },
  { id: "att-004", date: "2026-10-22", student_id: "std-004", student_roll: "CS2026-104", status: "PRESENT", recorded_at: "2026-10-22T09:05:00Z", remarks: "" },
  { id: "att-005", date: "2026-10-22", student_id: "std-005", student_roll: "CS2026-105", status: "PRESENT", recorded_at: "2026-10-22T09:05:00Z", remarks: "" },
  { id: "att-006", date: "2026-10-22", student_id: "std-006", student_roll: "CS2026-106", status: "PRESENT", recorded_at: "2026-10-22T09:05:00Z", remarks: "" },
  { id: "att-007", date: "2026-10-22", student_id: "std-007", student_roll: "CS2026-107", status: "ABSENT",  recorded_at: "2026-10-22T09:05:00Z", remarks: "Sick leave" },
  { id: "att-008", date: "2026-10-23", student_id: "std-001", student_roll: "CS2026-101", status: "PRESENT", recorded_at: "2026-10-23T09:05:00Z", remarks: "" },
  { id: "att-009", date: "2026-10-23", student_id: "std-002", student_roll: "CS2026-102", status: "PRESENT", recorded_at: "2026-10-23T09:05:00Z", remarks: "" },
  { id: "att-010", date: "2026-10-23", student_id: "std-003", student_roll: "CS2026-103", status: "PRESENT", recorded_at: "2026-10-23T09:05:00Z", remarks: "" },
  { id: "att-011", date: "2026-10-23", student_id: "std-004", student_roll: "CS2026-104", status: "PRESENT", recorded_at: "2026-10-23T09:05:00Z", remarks: "" },
  { id: "att-012", date: "2026-10-23", student_id: "std-005", student_roll: "CS2026-105", status: "PRESENT", recorded_at: "2026-10-23T09:05:00Z", remarks: "" },
  { id: "att-013", date: "2026-10-23", student_id: "std-009", student_roll: "CS2026-109", status: "ABSENT",  recorded_at: "2026-10-23T09:05:00Z", remarks: "" },
  { id: "att-014", date: "2026-10-26", student_id: "std-001", student_roll: "CS2026-101", status: "PRESENT", recorded_at: "2026-10-26T09:05:00Z", remarks: "" },
  { id: "att-015", date: "2026-10-26", student_id: "std-002", student_roll: "CS2026-102", status: "PRESENT", recorded_at: "2026-10-26T09:05:00Z", remarks: "" },
  { id: "att-016", date: "2026-10-26", student_id: "std-003", student_roll: "CS2026-103", status: "PRESENT", recorded_at: "2026-10-26T09:05:00Z", remarks: "" },
  { id: "att-017", date: "2026-10-26", student_id: "std-004", student_roll: "CS2026-104", status: "PRESENT", recorded_at: "2026-10-26T09:05:00Z", remarks: "" },
  { id: "att-018", date: "2026-10-26", student_id: "std-005", student_roll: "CS2026-105", status: "PRESENT", recorded_at: "2026-10-26T09:05:00Z", remarks: "" },
  { id: "att-019", date: "2026-10-26", student_id: "std-006", student_roll: "CS2026-106", status: "PRESENT", recorded_at: "2026-10-26T09:05:00Z", remarks: "" },
  { id: "att-020", date: "2026-10-26", student_id: "std-007", student_roll: "CS2026-107", status: "PRESENT", recorded_at: "2026-10-26T09:05:00Z", remarks: "" },
  { id: "att-021", date: "2026-10-26", student_id: "std-008", student_roll: "CS2026-108", status: "PRESENT", recorded_at: "2026-10-26T09:05:00Z", remarks: "" },
  { id: "att-022", date: "2026-10-26", student_id: "std-009", student_roll: "CS2026-109", status: "ABSENT",  recorded_at: "2026-10-26T09:05:00Z", remarks: "" },
  { id: "att-023", date: "2026-10-26", student_id: "std-010", student_roll: "CS2026-110", status: "ABSENT",  recorded_at: "2026-10-26T09:05:00Z", remarks: "Personal" }
];

let attendance = [...ATTENDANCE];
let nextId = 24;

function getCurrentDate() {
  return new Date().toISOString().split('T')[0];
}

export default function handler(req, res) {
  res.setHeader('Access-Control-Allow-Origin', '*');
  res.setHeader('Access-Control-Allow-Methods', 'GET, POST, PUT, OPTIONS');
  res.setHeader('Access-Control-Allow-Headers', 'Content-Type');

  if (req.method === 'OPTIONS') return res.status(204).end();

  if (req.method === 'GET') {
    const { date, student, id } = req.query;

    // By record id
    if (id) {
      const rec = attendance.find(a => a.id === id);
      if (!rec) return res.status(404).json({ success: false, status: 404, error: "Attendance record not found." });
      return res.status(200).json({ success: true, status: 200, data: rec });
    }

    // By student id
    if (student) {
      const recs = attendance.filter(a => a.student_id === student);
      // Compute stats
      const total = recs.length;
      const attended = recs.filter(a => a.status === 'PRESENT').length;
      const percentage = total === 0 ? 0.0 : Math.round((attended / total) * 1000) / 10;
      return res.status(200).json({
        success: true, status: 200,
        data: { student_id: student, records: recs, total, attended, percentage }
      });
    }

    // By date (default: today)
    const filterDate = date || getCurrentDate();
    const recs = attendance.filter(a => a.date === filterDate);
    return res.status(200).json({ success: true, status: 200, data: recs });
  }

  if (req.method === 'POST') {
    const body = req.body;
    if (!body || !body.student_id || !body.date || !body.status) {
      return res.status(400).json({ success: false, status: 400, error: "Missing required fields: student_id, date, status." });
    }
    if (!['PRESENT', 'ABSENT'].includes(body.status)) {
      return res.status(400).json({ success: false, status: 400, error: "Status must be 'PRESENT' or 'ABSENT'." });
    }

    // Idempotent upsert
    const existingIdx = attendance.findIndex(a => a.student_id === body.student_id && a.date === body.date);
    if (existingIdx !== -1) {
      attendance[existingIdx].status = body.status;
      attendance[existingIdx].remarks = body.remarks || attendance[existingIdx].remarks;
      return res.status(200).json({ success: true, status: 200, data: attendance[existingIdx], message: "Attendance updated." });
    }

    const newRec = {
      id: `att-${String(nextId++).padStart(3, '0')}`,
      date: body.date,
      student_id: body.student_id,
      student_roll: body.student_roll || "",
      status: body.status,
      recorded_at: new Date().toISOString(),
      remarks: body.remarks || ""
    };
    attendance.push(newRec);
    return res.status(200).json({ success: true, status: 200, data: newRec, message: "Attendance marked successfully." });
  }

  if (req.method === 'PUT') {
    const { id } = req.query;
    const idx = attendance.findIndex(a => a.id === id);
    if (idx === -1) return res.status(404).json({ success: false, status: 404, error: "Attendance record not found." });
    if (req.body.status) attendance[idx].status = req.body.status;
    if (req.body.remarks !== undefined) attendance[idx].remarks = req.body.remarks;
    return res.status(200).json({ success: true, status: 200, data: attendance[idx], message: "Attendance record updated." });
  }

  return res.status(405).json({ success: false, status: 405, error: "Method not allowed." });
}
