// Vercel Serverless Function — GET /api/dashboard
// Mirrors the C++ ReportService::getDashboardSummary() response

export default function handler(req, res) {
  res.setHeader('Access-Control-Allow-Origin', '*');
  res.setHeader('Access-Control-Allow-Methods', 'GET, OPTIONS');

  if (req.method === 'OPTIONS') return res.status(204).end();

  const now = new Date();
  const today = now.toISOString().split('T')[0];

  res.status(200).json({
    success: true,
    status: 200,
    data: {
      current_date: today,
      system_status: "ONLINE",
      timestamp: now.toISOString(),
      students: {
        active: 10,
        inactive: 2,
        total: 12,
        defaulters_count: 2
      },
      today_attendance: {
        present: 8,
        absent: 2,
        percentage: 80.0,
        total_logged: 10
      },
      latest_attendance: {
        date: "2026-10-26",
        present: 8,
        absent: 2,
        percentage: 80.0,
        total_logged: 10
      },
      attendance_trend: [
        { date: "2026-10-22", present: 6, absent: 1, total: 7, percentage: 85.7 },
        { date: "2026-10-23", present: 5, absent: 1, total: 6, percentage: 83.3 },
        { date: "2026-10-24", present: 5, absent: 1, total: 6, percentage: 83.3 },
        { date: "2026-10-25", present: 9, absent: 1, total: 10, percentage: 90.0 },
        { date: "2026-10-26", present: 8, absent: 2, total: 10, percentage: 80.0 }
      ],
      classroom: {
        classroom_id: "CR-302",
        classroom_name: "Smart Class & IoT Lab 302",
        temperature_celsius: 23.8,
        occupancy_count: 28,
        is_simulated: true,
        last_sync: now.toISOString(),
        projector_on: true,
        lights_on: 2,
        fans_on: 1,
        active_devices: 4,
        total_devices: 4
      }
    }
  });
}
