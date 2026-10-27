// Vercel Serverless Function — /api/classroom
// Mirrors C++ ClassroomManager — GET state, PUT/POST device update

let classroomState = {
  classroom_id: "CR-302",
  classroom_name: "Smart Class & IoT Lab 302",
  temperature_celsius: 23.8,
  occupancy_count: 28,
  is_simulated: true,
  last_telemetry_sync: new Date().toISOString(),
  devices: [
    { id: "dev-001", device_type: "LIGHT",     name: "Front Board Lights",    is_powered: true,  level: 2, location: "Front Board Area" },
    { id: "dev-002", device_type: "LIGHT",     name: "Rear Classroom Lights",  is_powered: true,  level: 2, location: "Rear Seating Area" },
    { id: "dev-003", device_type: "FAN",       name: "Ceiling Fan",            is_powered: true,  level: 2, location: "Centre Ceiling" },
    { id: "dev-004", device_type: "PROJECTOR", name: "Smart Interactive Board", is_powered: true, level: 1, location: "Front Wall" }
  ]
};

export default function handler(req, res) {
  res.setHeader('Access-Control-Allow-Origin', '*');
  res.setHeader('Access-Control-Allow-Methods', 'GET, PUT, POST, OPTIONS');
  res.setHeader('Access-Control-Allow-Headers', 'Content-Type');

  if (req.method === 'OPTIONS') return res.status(204).end();

  if (req.method === 'GET') {
    // Simulate slight temperature fluctuation
    classroomState.temperature_celsius = 23.0 + Math.round(Math.random() * 20) / 10;
    classroomState.last_telemetry_sync = new Date().toISOString();

    const lights_on  = classroomState.devices.filter(d => d.device_type === 'LIGHT'     && d.is_powered).length;
    const fans_on    = classroomState.devices.filter(d => d.device_type === 'FAN'       && d.is_powered).length;
    const projector  = classroomState.devices.find(d  => d.device_type === 'PROJECTOR');
    const active     = classroomState.devices.filter(d => d.is_powered).length;

    return res.status(200).json({
      success: true, status: 200,
      data: {
        ...classroomState,
        lights_on,
        fans_on,
        projector_on: projector?.is_powered || false,
        active_devices: active,
        total_devices: classroomState.devices.length
      }
    });
  }

  if (req.method === 'PUT' || req.method === 'POST') {
    const body = req.body;
    const deviceId = body?.device_id || body?.id;
    if (!deviceId) {
      return res.status(400).json({ success: false, status: 400, error: "device_id is required." });
    }

    const device = classroomState.devices.find(d => d.id === deviceId);
    if (!device) {
      return res.status(404).json({ success: false, status: 404, error: `Device '${deviceId}' not found.` });
    }

    if (body.is_powered !== undefined) device.is_powered = body.is_powered;
    if (body.level !== undefined) device.level = body.level;
    classroomState.last_telemetry_sync = new Date().toISOString();

    return res.status(200).json({ success: true, status: 200, data: device, message: "Device state updated successfully." });
  }

  return res.status(405).json({ success: false, status: 405, error: "Method not allowed." });
}
