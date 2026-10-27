// Vercel Serverless Function — /api/reports/students
// Mirrors C++ ReportService::getStudentStatisticsReport()

const STUDENTS = [
  { id: "std-001", roll_number: "CS2026-101", full_name: "Arjun Sharma",  division: "A" },
  { id: "std-002", roll_number: "CS2026-102", full_name: "Priya Patel",   division: "A" },
  { id: "std-003", roll_number: "CS2026-103", full_name: "Ravi Kumar",    division: "A" },
  { id: "std-004", roll_number: "CS2026-104", full_name: "Sneha Reddy",   division: "A" },
  { id: "std-005", roll_number: "CS2026-105", full_name: "Aditya Singh",  division: "A" },
  { id: "std-006", roll_number: "CS2026-106", full_name: "Kavya Nair",    division: "B" },
  { id: "std-007", roll_number: "CS2026-107", full_name: "Rahul Mehta",   division: "B" },
  { id: "std-008", roll_number: "CS2026-108", full_name: "Ananya Joshi",  division: "B" },
  { id: "std-009", roll_number: "CS2026-109", full_name: "Vikram Desai",  division: "B" },
  { id: "std-010", roll_number: "CS2026-110", full_name: "Pooja Iyer",    division: "B" }
];

const ATT_STATS = [
  { student_id: "std-001", total: 22, attended: 20, percentage: 90.9 },
  { student_id: "std-002", total: 22, attended: 21, percentage: 95.5 },
  { student_id: "std-003", total: 22, attended: 19, percentage: 86.4 },
  { student_id: "std-004", total: 22, attended: 18, percentage: 81.8 },
  { student_id: "std-005", total: 22, attended: 22, percentage: 100.0 },
  { student_id: "std-006", total: 22, attended: 17, percentage: 77.3 },
  { student_id: "std-007", total: 22, attended: 14, percentage: 63.6 },
  { student_id: "std-008", total: 22, attended: 10, percentage: 45.5 },
  { student_id: "std-009", total: 22, attended: 16, percentage: 72.7 },
  { student_id: "std-010", total: 22, attended: 20, percentage: 90.9 }
];

export default function handler(req, res) {
  res.setHeader('Access-Control-Allow-Origin', '*');
  res.setHeader('Access-Control-Allow-Methods', 'GET, OPTIONS');

  if (req.method === 'OPTIONS') return res.status(204).end();

  const stats = STUDENTS.map(s => {
    const att = ATT_STATS.find(a => a.student_id === s.id) || { total: 0, attended: 0, percentage: 0.0 };
    return {
      student_id: s.id,
      roll_number: s.roll_number,
      full_name: s.full_name,
      division: s.division,
      total_classes: att.total,
      attended: att.attended,
      absent: att.total - att.attended,
      percentage: att.percentage,
      is_defaulter: att.percentage < 75.0
    };
  });

  return res.status(200).json({ success: true, status: 200, data: { students: stats, total: stats.length } });
}
