/**
 * Smart Classroom Management System — REST API Client
 * Interfaces seamlessly with the Python Flask REST API Server
 */

class ApiService {
    constructor() {
        // Use relative URL so it dynamically matches the local server host and port
        this.baseUrl = '';
    }

    async request(endpoint, options = {}) {
        const url = `${this.baseUrl}${endpoint}`;
        const defaultHeaders = {
            'Content-Type': 'application/json',
            'Accept': 'application/json'
        };

        const config = {
            ...options,
            headers: {
                ...defaultHeaders,
                ...options.headers
            }
        };

        try {
            const response = await fetch(url, config);

            // Handle non-JSON responses (e.g. CSV stream)
            const contentType = response.headers.get('content-type') || '';
            if (contentType.includes('text/csv')) {
                const blob = await response.blob();
                return { success: true, blob };
            }

            let data;
            try {
                data = await response.json();
            } catch (err) {
                return {
                    success: false,
                    status: response.status,
                    error: `Invalid response format from Python server (HTTP ${response.status})`
                };
            }

            if (!response.ok) {
                return {
                    success: false,
                    status: response.status,
                    error: data.error || data.message || `Request failed with HTTP status ${response.status}`
                };
            }

            return data;
        } catch (networkError) {
            console.error('[API Connection Error]:', networkError);
            return {
                success: false,
                status: 0,
                error: 'Cannot connect to Python backend server. Please verify the server is running on localhost.'
            };
        }
    }

    // -------------------------------------------------------------------------
    // Dashboard Endpoints
    // -------------------------------------------------------------------------
    async getDashboard() {
        return this.request('/api/dashboard');
    }

    // -------------------------------------------------------------------------
    // Student Endpoints
    // -------------------------------------------------------------------------
    async getStudents(params = {}) {
        const query = new URLSearchParams();
        if (params.search) query.append('search', params.search);
        if (params.include_inactive !== undefined) query.append('include_inactive', params.include_inactive);
        const qStr = query.toString();
        return this.request(`/api/students${qStr ? '?' + qStr : ''}`);
    }

    async getStudentById(id) {
        return this.request(`/api/students/${encodeURIComponent(id)}`);
    }

    async createStudent(studentData) {
        return this.request('/api/students', {
            method: 'POST',
            body: JSON.stringify(studentData)
        });
    }

    async updateStudent(id, studentData) {
        return this.request(`/api/students/${encodeURIComponent(id)}`, {
            method: 'PUT',
            body: JSON.stringify(studentData)
        });
    }

    async deleteStudent(id) {
        return this.request(`/api/students/${encodeURIComponent(id)}`, {
            method: 'DELETE'
        });
    }

    // -------------------------------------------------------------------------
    // Attendance Endpoints
    // -------------------------------------------------------------------------
    async getAttendanceByDate(date) {
        return this.request(`/api/attendance?date=${encodeURIComponent(date)}`);
    }

    async getStudentAttendance(studentId) {
        return this.request(`/api/attendance/student/${encodeURIComponent(studentId)}`);
    }

    async recordAttendance(recordData) {
        return this.request('/api/attendance', {
            method: 'POST',
            body: JSON.stringify(recordData)
        });
    }

    async updateAttendance(id, status, remarks = '') {
        return this.request(`/api/attendance/${encodeURIComponent(id)}`, {
            method: 'PUT',
            body: JSON.stringify({ status, remarks })
        });
    }

    // -------------------------------------------------------------------------
    // Timetable Endpoints
    // -------------------------------------------------------------------------
    async getTimetable(day = null) {
        let endpoint = '/api/timetable';
        if (day && day !== 'ALL') {
            endpoint += `?day=${encodeURIComponent(day)}`;
        }
        return this.request(endpoint);
    }

    async createTimetableEntry(entryData) {
        return this.request('/api/timetable', {
            method: 'POST',
            body: JSON.stringify(entryData)
        });
    }

    async updateTimetableEntry(id, entryData) {
        return this.request(`/api/timetable/${encodeURIComponent(id)}`, {
            method: 'PUT',
            body: JSON.stringify(entryData)
        });
    }

    async deleteTimetableEntry(id) {
        return this.request(`/api/timetable/${encodeURIComponent(id)}`, {
            method: 'DELETE'
        });
    }

    // -------------------------------------------------------------------------
    // Smart Classroom IoT Endpoints
    // -------------------------------------------------------------------------
    async getClassroomStatus() {
        return this.request('/api/classroom');
    }

    async updateDeviceState(deviceId, isPowered, level = 0) {
        return this.request('/api/classroom/devices', {
            method: 'PUT',
            body: JSON.stringify({
                device_id: deviceId,
                is_powered: isPowered,
                level: level
            })
        });
    }

    // -------------------------------------------------------------------------
    // Reports Endpoints
    // -------------------------------------------------------------------------
    async getAttendanceReport(date = null) {
        let endpoint = '/api/reports/attendance';
        if (date) endpoint += `?date=${encodeURIComponent(date)}`;
        return this.request(endpoint);
    }

    async getStudentReports() {
        return this.request('/api/reports/students');
    }

    async getDefaultersReport() {
        return this.request('/api/reports/defaulters');
    }

    getExportCsvUrl(type = 'attendance', date = null) {
        let url = `/api/reports/export-csv?type=${encodeURIComponent(type)}`;
        if (date) url += `&date=${encodeURIComponent(date)}`;
        return url;
    }
}

// Global API singleton instance
window.API = new ApiService();
