// Vercel Serverless Function — /api/timetable
// Mirrors C++ TimetableManager — GET (all/by day), POST (create), PUT (update), DELETE

const TIMETABLE = [
  { id: "tt-001", subject: "CS601: Machine Learning",     teacher: "Dr. A. Krishnan",    classroom: "CR-302",  weekday: "MONDAY",    start_time: "09:00", end_time: "10:30" },
  { id: "tt-002", subject: "CS602: Cloud Computing",      teacher: "Prof. S. Mehta",     classroom: "Lab-101", weekday: "MONDAY",    start_time: "14:00", end_time: "15:30" },
  { id: "tt-003", subject: "CS603: Information Security", teacher: "Dr. R. Sharma",      classroom: "CR-302",  weekday: "TUESDAY",   start_time: "11:00", end_time: "12:30" },
  { id: "tt-004", subject: "CS604: Software Engineering", teacher: "Prof. P. Iyer",      classroom: "CR-302",  weekday: "WEDNESDAY", start_time: "09:00", end_time: "10:30" },
  { id: "tt-005", subject: "CS605: Compiler Design",      teacher: "Dr. V. Nair",        classroom: "CR-302",  weekday: "THURSDAY",  start_time: "10:00", end_time: "11:30" },
  { id: "tt-006", subject: "CS606: Computer Networks",    teacher: "Prof. K. Desai",     classroom: "Lab-101", weekday: "FRIDAY",    start_time: "09:00", end_time: "10:30" },
  { id: "tt-007", subject: "CS607: Project Lab",          teacher: "Dr. A. Krishnan",    classroom: "Lab-202", weekday: "FRIDAY",    start_time: "14:00", end_time: "17:00" }
];

let timetable = [...TIMETABLE];
let nextId = 8;

function timesOverlap(s1, e1, s2, e2) {
  return s1 < e2 && s2 < e1;
}

export default function handler(req, res) {
  res.setHeader('Access-Control-Allow-Origin', '*');
  res.setHeader('Access-Control-Allow-Methods', 'GET, POST, PUT, DELETE, OPTIONS');
  res.setHeader('Access-Control-Allow-Headers', 'Content-Type');

  if (req.method === 'OPTIONS') return res.status(204).end();

  if (req.method === 'GET') {
    const { day, id } = req.query;
    if (id) {
      const entry = timetable.find(t => t.id === id);
      if (!entry) return res.status(404).json({ success: false, status: 404, error: "Timetable entry not found." });
      return res.status(200).json({ success: true, status: 200, data: entry });
    }
    if (day) {
      const filtered = timetable.filter(t => t.weekday === day.toUpperCase());
      return res.status(200).json({ success: true, status: 200, data: filtered });
    }
    return res.status(200).json({ success: true, status: 200, data: timetable });
  }

  if (req.method === 'POST') {
    const body = req.body;
    if (!body || !body.subject || !body.teacher || !body.classroom || !body.weekday || !body.start_time || !body.end_time) {
      return res.status(400).json({ success: false, status: 400, error: "Missing required fields." });
    }
    if (body.start_time >= body.end_time) {
      return res.status(400).json({ success: false, status: 400, error: "start_time must be before end_time." });
    }

    // Collision detection
    const collision = timetable.find(t =>
      t.weekday === body.weekday &&
      (t.classroom === body.classroom || t.teacher === body.teacher) &&
      timesOverlap(body.start_time, body.end_time, t.start_time, t.end_time)
    );
    if (collision) {
      return res.status(409).json({
        success: false, status: 409,
        error: `Collision detected: ${collision.classroom === body.classroom ? `Classroom '${body.classroom}'` : `Teacher '${body.teacher}'`} is already booked on ${body.weekday} from ${collision.start_time} to ${collision.end_time} for '${collision.subject}'.`
      });
    }

    const newEntry = {
      id: `tt-${String(nextId++).padStart(3, '0')}`,
      subject: body.subject,
      teacher: body.teacher,
      classroom: body.classroom,
      weekday: body.weekday,
      start_time: body.start_time,
      end_time: body.end_time
    };
    timetable.push(newEntry);
    return res.status(201).json({ success: true, status: 201, data: newEntry, message: "Timetable slot scheduled successfully." });
  }

  if (req.method === 'PUT') {
    const { id } = req.query;
    const idx = timetable.findIndex(t => t.id === id);
    if (idx === -1) return res.status(404).json({ success: false, status: 404, error: "Timetable entry not found." });
    timetable[idx] = { ...timetable[idx], ...req.body, id: timetable[idx].id };
    return res.status(200).json({ success: true, status: 200, data: timetable[idx], message: "Timetable slot updated successfully." });
  }

  if (req.method === 'DELETE') {
    const { id } = req.query;
    const idx = timetable.findIndex(t => t.id === id);
    if (idx === -1) return res.status(404).json({ success: false, status: 404, error: "Timetable entry not found." });
    timetable.splice(idx, 1);
    return res.status(200).json({ success: true, status: 200, data: { id, status: "deleted" }, message: "Timetable slot removed." });
  }

  return res.status(405).json({ success: false, status: 405, error: "Method not allowed." });
}
