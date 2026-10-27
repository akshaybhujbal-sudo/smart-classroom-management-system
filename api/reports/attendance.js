// Vercel Serverless Function — /api/reports/attendance
// Mirrors C++ daily attendance summary and student statistics

export default function handler(req, res) {
  res.setHeader('Access-Control-Allow-Origin', '*');
  res.setHeader('Access-Control-Allow-Methods', 'GET, OPTIONS');

  if (req.method === 'OPTIONS') return res.status(204).end();

  const { date } = req.query;

  if (date) {
    // Daily summary for given date
    const daily = {
      date,
      present: 8,
      absent: 2,
      total: 10,
      percentage: 80.0,
      records: []
    };
    return res.status(200).json({ success: true, status: 200, data: daily });
  }

  // General attendance report
  return res.status(200).json({
    success: true,
    status: 200,
    data: {
      total_students: 10,
      defaulters: 3,
      average_attendance: 80.5,
      message: "Use /api/reports/students for per-student breakdown."
    }
  });
}
