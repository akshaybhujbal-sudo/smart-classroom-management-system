// Vercel Serverless Function — /api/reports/export-csv
// Mirrors C++ CsvExporter

export default function handler(req, res) {
  res.setHeader('Access-Control-Allow-Origin', '*');

  if (req.method === 'OPTIONS') return res.status(204).end();

  const { type } = req.query;

  if (type === 'students') {
    const csv = [
      'id,roll_number,full_name,division,department,semester,is_active',
      'std-001,CS2026-101,Arjun Sharma,A,Computer Science & Engineering,6,true',
      'std-002,CS2026-102,Priya Patel,A,Computer Science & Engineering,6,true',
      'std-003,CS2026-103,Ravi Kumar,A,Computer Science & Engineering,6,true',
      'std-004,CS2026-104,Sneha Reddy,A,Computer Science & Engineering,6,true',
      'std-005,CS2026-105,Aditya Singh,A,Computer Science & Engineering,6,true',
      'std-006,CS2026-106,Kavya Nair,B,Computer Science & Engineering,6,true',
      'std-007,CS2026-107,Rahul Mehta,B,Computer Science & Engineering,6,true',
      'std-008,CS2026-108,Ananya Joshi,B,Computer Science & Engineering,6,true',
      'std-009,CS2026-109,Vikram Desai,B,Computer Science & Engineering,6,true',
      'std-010,CS2026-110,Pooja Iyer,B,Computer Science & Engineering,6,true'
    ].join('\n');

    res.setHeader('Content-Type', 'text/csv');
    res.setHeader('Content-Disposition', 'attachment; filename="students_report.csv"');
    return res.status(200).send(csv);
  }

  // Default: attendance CSV
  const csv = [
    'id,date,student_id,student_roll,status,recorded_at,remarks',
    'att-001,2026-10-22,std-001,CS2026-101,PRESENT,2026-10-22T09:05:00Z,',
    'att-002,2026-10-22,std-002,CS2026-102,PRESENT,2026-10-22T09:05:00Z,',
    'att-003,2026-10-22,std-003,CS2026-103,PRESENT,2026-10-22T09:05:00Z,',
    'att-004,2026-10-22,std-004,CS2026-104,PRESENT,2026-10-22T09:05:00Z,',
    'att-005,2026-10-22,std-005,CS2026-105,PRESENT,2026-10-22T09:05:00Z,',
    'att-006,2026-10-22,std-006,CS2026-106,PRESENT,2026-10-22T09:05:00Z,',
    'att-007,2026-10-22,std-007,CS2026-107,ABSENT,2026-10-22T09:05:00Z,Sick leave',
    'att-008,2026-10-23,std-001,CS2026-101,PRESENT,2026-10-23T09:05:00Z,',
    'att-009,2026-10-23,std-002,CS2026-102,PRESENT,2026-10-23T09:05:00Z,',
    'att-010,2026-10-23,std-003,CS2026-103,PRESENT,2026-10-23T09:05:00Z,',
    'att-014,2026-10-26,std-001,CS2026-101,PRESENT,2026-10-26T09:05:00Z,',
    'att-015,2026-10-26,std-002,CS2026-102,PRESENT,2026-10-26T09:05:00Z,',
    'att-016,2026-10-26,std-003,CS2026-103,PRESENT,2026-10-26T09:05:00Z,',
    'att-017,2026-10-26,std-004,CS2026-104,PRESENT,2026-10-26T09:05:00Z,',
    'att-018,2026-10-26,std-005,CS2026-105,PRESENT,2026-10-26T09:05:00Z,',
    'att-019,2026-10-26,std-006,CS2026-106,PRESENT,2026-10-26T09:05:00Z,',
    'att-020,2026-10-26,std-007,CS2026-107,PRESENT,2026-10-26T09:05:00Z,',
    'att-021,2026-10-26,std-008,CS2026-108,PRESENT,2026-10-26T09:05:00Z,',
    'att-022,2026-10-26,std-009,CS2026-109,ABSENT,2026-10-26T09:05:00Z,',
    'att-023,2026-10-26,std-010,CS2026-110,ABSENT,2026-10-26T09:05:00Z,Personal'
  ].join('\n');

  res.setHeader('Content-Type', 'text/csv');
  res.setHeader('Content-Disposition', 'attachment; filename="attendance_report.csv"');
  return res.status(200).send(csv);
}
