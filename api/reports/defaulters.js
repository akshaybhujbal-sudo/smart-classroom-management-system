// Vercel Serverless Function — /api/reports/defaulters
// Mirrors C++ ReportService::getDefaultersReport(75.0)

const DEFAULTERS = [
  { student_id: "std-007", roll_number: "CS2026-107", full_name: "Rahul Mehta",  division: "B", total_classes: 22, attended: 14, absent: 8, percentage: 63.6 },
  { student_id: "std-008", roll_number: "CS2026-108", full_name: "Ananya Joshi", division: "B", total_classes: 22, attended: 10, absent: 12, percentage: 45.5 },
  { student_id: "std-009", roll_number: "CS2026-109", full_name: "Vikram Desai", division: "B", total_classes: 22, attended: 16, absent: 6,  percentage: 72.7 }
];

export default function handler(req, res) {
  res.setHeader('Access-Control-Allow-Origin', '*');
  res.setHeader('Access-Control-Allow-Methods', 'GET, OPTIONS');

  if (req.method === 'OPTIONS') return res.status(204).end();

  return res.status(200).json({
    success: true,
    status: 200,
    data: {
      threshold: 75.0,
      defaulters: DEFAULTERS,
      count: DEFAULTERS.length
    }
  });
}
