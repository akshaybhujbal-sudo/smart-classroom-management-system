/**
 * Smart Classroom Management System — Single Page Application (SPA) Controller
 * Interactivity, DOM Binding, Real-Time State Management and REST API Integration
 */

(function () {
    'use strict';

    // Application State Store
    const state = {
        currentView: 'dashboard',
        students: [],
        attendanceDate: getCurrentDateStr(),
        attendanceRoster: [],
        timetableEntries: [],
        selectedTimetableDay: 'ALL',
        classroomState: null,
        reportsData: [],
        isBackendOnline: true
    };

    // Helper: Formats current date as YYYY-MM-DD
    function getCurrentDateStr() {
        const d = new Date();
        const year = d.getFullYear();
        const month = String(d.getMonth() + 1).padStart(2, '0');
        const day = String(d.getDate()).padStart(2, '0');
        return `${year}-${month}-${day}`;
    }

    // Helper: Returns current day of week (e.g. MONDAY)
    function getCurrentWeekdayStr() {
        const days = ['SUNDAY', 'MONDAY', 'TUESDAY', 'WEDNESDAY', 'THURSDAY', 'FRIDAY', 'SATURDAY'];
        return days[new Date().getDay()];
    }

    // -------------------------------------------------------------------------
    // Toast Notification System
    // -------------------------------------------------------------------------
    function showToast(message, type = 'info', duration = 3500) {
        const container = document.getElementById('toast-container');
        if (!container) return;

        const toast = document.createElement('div');
        toast.className = `toast ${type}`;

        let iconSvg = '';
        if (type === 'success') {
            iconSvg = '<svg viewBox="0 0 24 24" width="18" height="18" fill="none" stroke="#10b981" stroke-width="2.5"><polyline points="20 6 9 17 4 12"/></svg>';
        } else if (type === 'error') {
            iconSvg = '<svg viewBox="0 0 24 24" width="18" height="18" fill="none" stroke="#ef4444" stroke-width="2.5"><circle cx="12" cy="12" r="10"/><line x1="15" y1="9" x2="9" y2="15"/><line x1="9" y1="9" x2="15" y2="15"/></svg>';
        } else if (type === 'warning') {
            iconSvg = '<svg viewBox="0 0 24 24" width="18" height="18" fill="none" stroke="#f59e0b" stroke-width="2.5"><path d="M10.29 3.86L1.82 18a2 2 0 0 0 1.71 3h16.94a2 2 0 0 0 1.71-3L13.71 3.86a2 2 0 0 0-3.42 0z"/><line x1="12" y1="9" x2="12" y2="13"/><line x1="12" y1="17" x2="12.01" y2="17"/></svg>';
        } else {
            iconSvg = '<svg viewBox="0 0 24 24" width="18" height="18" fill="none" stroke="#2563eb" stroke-width="2.5"><circle cx="12" cy="12" r="10"/><line x1="12" y1="16" x2="12" y2="12"/><line x1="12" y1="8" x2="12.01" y2="8"/></svg>';
        }

        toast.innerHTML = `
            ${iconSvg}
            <div style="flex: 1; font-weight: 500;">${escapeHtml(message)}</div>
            <button style="background:none; border:none; color:var(--text-muted); cursor:pointer; font-size:16px;" onclick="this.parentElement.remove()">&times;</button>
        `;

        container.appendChild(toast);
        setTimeout(() => {
            toast.style.opacity = '0';
            toast.style.transform = 'translateX(20px)';
            toast.style.transition = 'all 0.3s ease';
            setTimeout(() => toast.remove(), 300);
        }, duration);
    }

    function escapeHtml(str) {
        if (!str) return '';
        return String(str)
            .replace(/&/g, '&amp;')
            .replace(/</g, '&lt;')
            .replace(/>/g, '&gt;')
            .replace(/"/g, '&quot;')
            .replace(/'/g, '&#039;');
    }

    // -------------------------------------------------------------------------
    // Modal Helpers
    // -------------------------------------------------------------------------
    function openModal(modalId) {
        const modal = document.getElementById(modalId);
        if (modal) modal.classList.add('active');
    }

    function closeModal(modalId) {
        const modal = document.getElementById(modalId);
        if (modal) modal.classList.remove('active');
    }

    // Setup global modal close buttons
    document.querySelectorAll('[data-close-modal]').forEach(btn => {
        btn.addEventListener('click', (e) => {
            const overlay = e.target.closest('.modal-overlay');
            if (overlay) overlay.classList.remove('active');
        });
    });

    // Close on backdrop click
    document.querySelectorAll('.modal-overlay').forEach(overlay => {
        overlay.addEventListener('click', (e) => {
            if (e.target === overlay) overlay.classList.remove('active');
        });
    });

    // Confirmation Dialog Handler
    let confirmCallback = null;
    function showConfirmDialog(title, message, onConfirm) {
        document.getElementById('modal-confirm-title').textContent = title;
        document.getElementById('modal-confirm-message').textContent = message;
        confirmCallback = onConfirm;
        openModal('modal-confirm');
    }

    document.getElementById('btn-confirm-action').addEventListener('click', () => {
        closeModal('modal-confirm');
        if (confirmCallback) {
            confirmCallback();
            confirmCallback = null;
        }
    });

    // -------------------------------------------------------------------------
    // Navigation & Routing
    // -------------------------------------------------------------------------
    function navigateTo(viewName) {
        state.currentView = viewName;

        // Update Nav items
        document.querySelectorAll('.nav-item').forEach(item => {
            if (item.dataset.view === viewName) {
                item.classList.add('active');
            } else {
                item.classList.remove('active');
            }
        });

        // Update Section views
        document.querySelectorAll('.view-section').forEach(sec => {
            if (sec.id === `view-${viewName}`) {
                sec.classList.add('active');
            } else {
                sec.classList.remove('active');
            }
        });

        // Update Breadcrumb title
        const titles = {
            dashboard: 'Executive Dashboard',
            students: 'Student Records Management',
            attendance: 'Daily Attendance Ledger',
            timetable: 'Academic Lecture Schedule',
            classroom: 'Smart Classroom IoT Telemetry',
            reports: 'Reports & Academic Analytics',
            about: 'About Project & Architecture'
        };
        document.getElementById('page-title').textContent = titles[viewName] || 'Dashboard';

        // Close mobile sidebar if open
        document.getElementById('sidebar').classList.remove('open');

        // Load data for the active view
        loadViewData(viewName);
    }

    // Attach click events on navigation items
    document.querySelectorAll('.nav-item').forEach(item => {
        item.addEventListener('click', () => {
            const view = item.dataset.view;
            if (view) navigateTo(view);
        });
    });

    // Quick navigation actions
    document.querySelectorAll('[data-action="go-timetable"]').forEach(el => {
        el.addEventListener('click', () => navigateTo('timetable'));
    });
    document.querySelectorAll('[data-action="go-classroom"]').forEach(el => {
        el.addEventListener('click', () => navigateTo('classroom'));
    });

    // Mobile Sidebar Toggle
    document.getElementById('mobile-toggle').addEventListener('click', () => {
        document.getElementById('sidebar').classList.toggle('open');
    });

    // Live Header Clock
    function updateClock() {
        const now = new Date();
        const timeStr = now.toLocaleTimeString('en-US', { hour12: false });
        const clockEl = document.getElementById('current-clock');
        if (clockEl) clockEl.textContent = timeStr;
    }
    setInterval(updateClock, 1000);
    updateClock();

    // Global Sync Button
    document.getElementById('btn-refresh-global').addEventListener('click', async function () {
        this.classList.add('spinning');
        await loadViewData(state.currentView, true);
        setTimeout(() => this.classList.remove('spinning'), 500);
        showToast('Data synchronized with C++ backend', 'info', 2000);
    });

    // -------------------------------------------------------------------------
    // Backend Health & Data Dispatcher
    // -------------------------------------------------------------------------
    function updateBackendStatus(isOnline) {
        state.isBackendOnline = isOnline;
        const dot = document.getElementById('backend-status-dot');
        const text = document.getElementById('backend-status-text');
        if (isOnline) {
            dot.classList.remove('offline');
            text.textContent = 'C++ Online';
        } else {
            dot.classList.add('offline');
            text.textContent = 'Offline';
        }
    }

    async function loadViewData(viewName, force = false) {
        try {
            switch (viewName) {
                case 'dashboard':
                    await renderDashboardView();
                    break;
                case 'students':
                    await renderStudentsView();
                    break;
                case 'attendance':
                    await renderAttendanceView();
                    break;
                case 'timetable':
                    await renderTimetableView();
                    break;
                case 'classroom':
                    await renderClassroomView();
                    break;
                case 'reports':
                    await renderReportsView();
                    break;
                case 'about':
                    // Static academic brief
                    break;
            }
            updateBackendStatus(true);
        } catch (err) {
            console.error('Failed to load view data:', err);
            updateBackendStatus(false);
            showToast('Unable to connect to C++ backend server.', 'error');
        }
    }

    // -------------------------------------------------------------------------
    // 1. DASHBOARD VIEW
    // -------------------------------------------------------------------------
    async function renderDashboardView() {
        const res = await window.API.getDashboard();
        if (!res.success) {
            updateBackendStatus(false);
            showToast(res.error, 'error');
            return;
        }

        const data = res.data;
        document.getElementById('kpi-total-students').textContent = data.students.active || 0;
        document.getElementById('kpi-inactive-students').textContent = `${data.students.inactive || 0} archived / alumni`;
        document.getElementById('kpi-attendance-pct').textContent = `${data.today_attendance.percentage || 0}%`;
        document.getElementById('kpi-attendance-ratio').textContent = `${data.today_attendance.present || 0} Present / ${data.today_attendance.total_logged || 0} Total`;
        document.getElementById('kpi-defaulters-count').textContent = data.students.defaulters_count || 0;
        document.getElementById('kpi-room-temp').textContent = `${data.classroom.temperature_celsius || 24}°C`;
        document.getElementById('kpi-room-occupancy').textContent = `${data.classroom.occupancy_count || 0} inside hall`;

        // Classroom quick card
        document.getElementById('dash-env-temp').textContent = `${data.classroom.temperature_celsius || 24}°C`;
        document.getElementById('dash-env-occupancy').textContent = `${data.classroom.occupancy_count || 0} persons`;
        document.getElementById('dash-lights-status').textContent = `${data.classroom.lights_on || 0} ON`;
        document.getElementById('dash-proj-status').textContent = data.classroom.projector_on ? 'POWERED ON' : 'POWERED OFF';
        document.getElementById('dash-proj-status').className = data.classroom.projector_on ? 'badge active' : 'badge inactive';

        // Today's schedule table
        const todayDay = getCurrentWeekdayStr();
        document.getElementById('dash-today-name').textContent = todayDay;

        const ttRes = await window.API.getTimetable(todayDay);
        const tbody = document.getElementById('dash-timetable-body');
        tbody.innerHTML = '';

        if (ttRes.success && ttRes.data && ttRes.data.length > 0) {
            ttRes.data.forEach(entry => {
                const tr = document.createElement('tr');
                tr.innerHTML = `
                    <td><strong style="color:var(--primary); font-family:var(--font-mono);">${escapeHtml(entry.start_time)} - ${escapeHtml(entry.end_time)}</strong></td>
                    <td><strong>${escapeHtml(entry.subject)}</strong></td>
                    <td>${escapeHtml(entry.teacher)}</td>
                    <td><span class="badge weekday">${escapeHtml(entry.classroom)}</span></td>
                `;
                tbody.appendChild(tr);
            });
        } else {
            tbody.innerHTML = `<tr><td colspan="4" style="text-align:center; padding:24px; color:var(--text-muted);">No lecture classes scheduled for today (${todayDay}).</td></tr>`;
        }
    }

    // -------------------------------------------------------------------------
    // 2. STUDENTS VIEW
    // -------------------------------------------------------------------------
    async function renderStudentsView() {
        const searchInput = document.getElementById('student-search-input');
        const filterSelect = document.getElementById('student-filter-status');

        const query = searchInput ? searchInput.value.trim() : '';
        const filterVal = filterSelect ? filterSelect.value : 'all';

        const params = {};
        if (query) params.search = query;
        if (filterVal === 'active') params.include_inactive = 'false';
        if (filterVal === 'all') params.include_inactive = 'true';

        const res = await window.API.getStudents(params);
        const tbody = document.getElementById('student-table-body');
        tbody.innerHTML = '';

        if (!res.success) {
            tbody.innerHTML = `<tr><td colspan="8" style="text-align:center; padding:24px; color:var(--danger);">${escapeHtml(res.error)}</td></tr>`;
            return;
        }

        let students = res.data || [];
        if (filterVal === 'inactive') {
            students = students.filter(s => !s.is_active);
        }

        if (students.length === 0) {
            tbody.innerHTML = `
                <tr>
                    <td colspan="8">
                        <div class="empty-state">
                            <div class="empty-state-icon">
                                <svg viewBox="0 0 24 24" fill="none" stroke="currentColor"><circle cx="12" cy="12" r="10"/><line x1="12" y1="8" x2="12" y2="12"/><line x1="12" y1="16" x2="12.01" y2="16"/></svg>
                            </div>
                            <h3>No Students Found</h3>
                            <p>No student records match the selected search or filter criteria.</p>
                            <button class="btn btn-primary btn-sm" id="empty-add-student-btn">Register First Student</button>
                        </div>
                    </td>
                </tr>
            `;
            const emptyBtn = document.getElementById('empty-add-student-btn');
            if (emptyBtn) emptyBtn.addEventListener('click', openAddStudentModal);
            return;
        }

        students.forEach(s => {
            const tr = document.createElement('tr');
            tr.innerHTML = `
                <td><strong style="color:var(--navy-900); font-family:var(--font-mono);">${escapeHtml(s.roll_number)}</strong></td>
                <td><strong>${escapeHtml(s.full_name)}</strong></td>
                <td><span class="badge" style="background-color:var(--bg-alt);">${escapeHtml(s.division || 'A')}</span></td>
                <td>${escapeHtml(s.department)}</td>
                <td>${escapeHtml(s.email || '—')}</td>
                <td>Sem ${s.semester}</td>
                <td><span class="badge ${s.is_active ? 'active' : 'inactive'}">${s.is_active ? 'Active' : 'Archived'}</span></td>
                <td style="text-align: right;">
                    <button class="btn-icon" data-edit-student="${escapeHtml(s.id)}" title="Edit Student">
                        <svg viewBox="0 0 24 24" width="15" height="15" fill="none" stroke="currentColor" stroke-width="2"><path d="M11 4H4a2 2 0 0 0-2 2v14a2 2 0 0 0 2 2h14a2 2 0 0 0 2-2v-7"/><path d="M18.5 2.5a2.121 2.121 0 0 1 3 3L12 15l-4 1 1-4 9.5-9.5z"/></svg>
                    </button>
                    ${s.is_active ? `
                    <button class="btn-icon danger" data-delete-student="${escapeHtml(s.id)}" title="Deactivate (Soft Delete)">
                        <svg viewBox="0 0 24 24" width="15" height="15" fill="none" stroke="currentColor" stroke-width="2"><polyline points="3 6 5 6 21 6"/><path d="M19 6v14a2 2 0 0 1-2 2H7a2 2 0 0 1-2-2V6m3 0V4a2 2 0 0 1 2-2h4a2 2 0 0 1 2 2v2"/><line x1="10" y1="11" x2="10" y2="17"/><line x1="14" y1="11" x2="14" y2="17"/></svg>
                    </button>` : ''}
                </td>
            `;
            tbody.appendChild(tr);
        });

        // Attach action handlers
        tbody.querySelectorAll('[data-edit-student]').forEach(btn => {
            btn.addEventListener('click', () => {
                const id = btn.dataset.editStudent;
                const student = students.find(item => item.id === id);
                if (student) openEditStudentModal(student);
            });
        });

        tbody.querySelectorAll('[data-delete-student]').forEach(btn => {
            btn.addEventListener('click', () => {
                const id = btn.dataset.deleteStudent;
                const student = students.find(item => item.id === id);
                if (student) {
                    showConfirmDialog(
                        'Deactivate Student',
                        `Are you sure you want to deactivate ${student.full_name} (${student.roll_number})? Their profile will be archived, and historical attendance logs will be preserved.`,
                        async () => {
                            const delRes = await window.API.deleteStudent(id);
                            if (delRes.success) {
                                showToast(`Student ${student.full_name} deactivated successfully`, 'success');
                                renderStudentsView();
                            } else {
                                showToast(delRes.error, 'error');
                            }
                        }
                    );
                }
            });
        });
    }

    // Student Filter and Search Event Listeners
    let studentSearchTimeout = null;
    document.getElementById('student-search-input').addEventListener('input', () => {
        clearTimeout(studentSearchTimeout);
        studentSearchTimeout = setTimeout(renderStudentsView, 250);
    });
    document.getElementById('student-filter-status').addEventListener('change', renderStudentsView);

    // Add Student Modal Logic
    function openAddStudentModal() {
        document.getElementById('modal-student-title').textContent = 'Register New Student';
        document.getElementById('student-id-field').value = '';
        document.getElementById('std-roll').value = '';
        document.getElementById('std-roll').disabled = false;
        document.getElementById('std-name').value = '';
        document.getElementById('std-division').value = 'A';
        document.getElementById('std-dept').value = 'Computer Science & Engineering';
        document.getElementById('std-email').value = '';
        document.getElementById('std-semester').value = '6';
        openModal('modal-student');
    }

    function openEditStudentModal(student) {
        document.getElementById('modal-student-title').textContent = 'Edit Student Details';
        document.getElementById('student-id-field').value = student.id;
        document.getElementById('std-roll').value = student.roll_number;
        document.getElementById('std-roll').disabled = false;
        document.getElementById('std-name').value = student.full_name;
        document.getElementById('std-division').value = student.division || 'A';
        document.getElementById('std-dept').value = student.department || '';
        document.getElementById('std-email').value = student.email || '';
        document.getElementById('std-semester').value = student.semester || 1;
        openModal('modal-student');
    }

    document.getElementById('btn-open-add-student').addEventListener('click', openAddStudentModal);

    document.getElementById('form-student').addEventListener('submit', async (e) => {
        e.preventDefault();
        const id = document.getElementById('student-id-field').value;
        const payload = {
            roll_number: document.getElementById('std-roll').value.trim(),
            full_name: document.getElementById('std-name').value.trim(),
            division: document.getElementById('std-division').value,
            department: document.getElementById('std-dept').value.trim(),
            email: document.getElementById('std-email').value.trim(),
            semester: parseInt(document.getElementById('std-semester').value, 10),
            is_active: true
        };

        const submitBtn = document.getElementById('btn-submit-student');
        submitBtn.disabled = true;
        submitBtn.textContent = 'Saving...';

        let res;
        if (id) {
            res = await window.API.updateStudent(id, payload);
        } else {
            res = await window.API.createStudent(payload);
        }

        submitBtn.disabled = false;
        submitBtn.textContent = 'Save Student';

        if (res.success) {
            closeModal('modal-student');
            showToast(id ? 'Student record updated' : 'Student registered successfully', 'success');
            renderStudentsView();
            renderDashboardView();
        } else {
            showToast(res.error, 'error');
        }
    });

    // -------------------------------------------------------------------------
    // 3. ATTENDANCE MANAGEMENT VIEW
    // -------------------------------------------------------------------------
    async function renderAttendanceView() {
        const datePicker = document.getElementById('att-date-picker');
        if (!datePicker.value) {
            datePicker.value = state.attendanceDate;
        } else {
            state.attendanceDate = datePicker.value;
        }

        document.getElementById('att-session-date-label').textContent = state.attendanceDate;

        // Fetch active students and existing attendance for date
        const [stdRes, attRes] = await Promise.all([
            window.API.getStudents({ include_inactive: 'false' }),
            window.API.getAttendanceByDate(state.attendanceDate)
        ]);

        const tbody = document.getElementById('att-roster-body');
        tbody.innerHTML = '';

        if (!stdRes.success) {
            tbody.innerHTML = `<tr><td colspan="5" style="text-align:center; padding:24px; color:var(--danger);">${escapeHtml(stdRes.error)}</td></tr>`;
            return;
        }

        const activeStudents = stdRes.data || [];
        const existingRecords = attRes.success ? (attRes.data || []) : [];

        // Build roster map
        state.attendanceRoster = activeStudents.map(student => {
            const match = existingRecords.find(r => r.student_id === student.id || r.student_roll === student.roll_number);
            return {
                student_id: student.id,
                student_roll: student.roll_number,
                full_name: student.full_name,
                division: student.division || 'A',
                status: match ? match.status : 'PRESENT',
                remarks: match ? match.remarks : ''
            };
        });

        if (state.attendanceRoster.length === 0) {
            tbody.innerHTML = `<tr><td colspan="5" style="text-align:center; padding:24px; color:var(--text-muted);">No active students found in institution directory.</td></tr>`;
            updateAttendanceSummaryMetrics();
            return;
        }

        state.attendanceRoster.forEach((item, index) => {
            const tr = document.createElement('tr');
            tr.innerHTML = `
                <td><strong style="color:var(--navy-900); font-family:var(--font-mono);">${escapeHtml(item.student_roll)}</strong></td>
                <td><strong>${escapeHtml(item.full_name)}</strong></td>
                <td><span class="badge" style="background-color:var(--bg-alt);">${escapeHtml(item.division)}</span></td>
                <td style="text-align: center;">
                    <div style="display:inline-flex; gap:8px;">
                        <label style="cursor:pointer; display:flex; align-items:center; gap:4px; font-weight:600; color:#047857;">
                            <input type="radio" name="att-status-${index}" value="PRESENT" ${item.status === 'PRESENT' ? 'checked' : ''} data-roster-index="${index}">
                            Present
                        </label>
                        <label style="cursor:pointer; display:flex; align-items:center; gap:4px; font-weight:600; color:#b91c1c;">
                            <input type="radio" name="att-status-${index}" value="ABSENT" ${item.status === 'ABSENT' ? 'checked' : ''} data-roster-index="${index}">
                            Absent
                        </label>
                    </div>
                </td>
                <td>
                    <input type="text" class="form-control" style="padding:4px 8px; font-size:12.5px;" placeholder="Optional note..." value="${escapeHtml(item.remarks)}" data-roster-remarks="${index}">
                </td>
            `;
            tbody.appendChild(tr);
        });

        // Event listeners for radio button toggles
        tbody.querySelectorAll('input[type="radio"]').forEach(radio => {
            radio.addEventListener('change', (e) => {
                const idx = parseInt(e.target.dataset.rosterIndex, 10);
                state.attendanceRoster[idx].status = e.target.value;
                updateAttendanceSummaryMetrics();
            });
        });

        tbody.querySelectorAll('input[data-roster-remarks]').forEach(input => {
            input.addEventListener('input', (e) => {
                const idx = parseInt(e.target.dataset.rosterRemarks, 10);
                state.attendanceRoster[idx].remarks = e.target.value.trim();
            });
        });

        updateAttendanceSummaryMetrics();
    }

    function updateAttendanceSummaryMetrics() {
        let present = 0;
        let absent = 0;
        state.attendanceRoster.forEach(item => {
            if (item.status === 'PRESENT') present++;
            else absent++;
        });

        const total = state.attendanceRoster.length;
        const pct = total > 0 ? ((present / total) * 100).toFixed(1) : '0.0';

        document.getElementById('att-count-present').textContent = present;
        document.getElementById('att-count-absent').textContent = absent;
        document.getElementById('att-rate-percentage').textContent = `${pct}%`;
    }

    // Date Picker Change
    document.getElementById('att-date-picker').addEventListener('change', () => {
        renderAttendanceView();
    });

    // Mark All Present / Absent shortcuts
    document.getElementById('btn-mark-all-present').addEventListener('click', () => {
        state.attendanceRoster.forEach(item => item.status = 'PRESENT');
        document.querySelectorAll('#att-roster-body input[value="PRESENT"]').forEach(r => r.checked = true);
        updateAttendanceSummaryMetrics();
    });

    document.getElementById('btn-mark-all-absent').addEventListener('click', () => {
        state.attendanceRoster.forEach(item => item.status = 'ABSENT');
        document.querySelectorAll('#att-roster-body input[value="ABSENT"]').forEach(r => r.checked = true);
        updateAttendanceSummaryMetrics();
    });

    // Save Attendance Button
    document.getElementById('btn-save-attendance').addEventListener('click', async function () {
        if (state.attendanceRoster.length === 0) {
            showToast('No student records to save.', 'warning');
            return;
        }

        this.disabled = true;
        this.innerHTML = '<span>Saving...</span>';

        let successCount = 0;
        let errorCount = 0;

        for (const item of state.attendanceRoster) {
            const payload = {
                date: state.attendanceDate,
                student_id: item.student_id,
                student_roll: item.student_roll,
                status: item.status,
                remarks: item.remarks
            };
            const res = await window.API.recordAttendance(payload);
            if (res.success) successCount++;
            else errorCount++;
        }

        this.disabled = false;
        this.innerHTML = `
            <svg viewBox="0 0 24 24" width="16" height="16" fill="none" stroke="currentColor" stroke-width="2.5"><polyline points="20 6 9 17 4 12"/></svg>
            <span>Save Attendance</span>
        `;

        if (errorCount === 0) {
            showToast(`Attendance saved successfully for ${state.attendanceDate} (${successCount} records)`, 'success');
        } else {
            showToast(`Saved ${successCount} records, ${errorCount} errors encountered`, 'warning');
        }

        renderDashboardView();
    });

    // -------------------------------------------------------------------------
    // 4. TIMETABLE MANAGEMENT VIEW
    // -------------------------------------------------------------------------
    async function renderTimetableView() {
        const res = await window.API.getTimetable(state.selectedTimetableDay);
        const tbody = document.getElementById('timetable-table-body');
        tbody.innerHTML = '';

        if (!res.success) {
            tbody.innerHTML = `<tr><td colspan="6" style="text-align:center; padding:24px; color:var(--danger);">${escapeHtml(res.error)}</td></tr>`;
            return;
        }

        const entries = res.data || [];
        if (entries.length === 0) {
            tbody.innerHTML = `
                <tr>
                    <td colspan="6">
                        <div class="empty-state">
                            <div class="empty-state-icon">
                                <svg viewBox="0 0 24 24" fill="none" stroke="currentColor"><circle cx="12" cy="12" r="10"/><polyline points="12 6 12 12 16 14"/></svg>
                            </div>
                            <h3>No Scheduled Classes</h3>
                            <p>No lecture slots recorded for ${state.selectedTimetableDay === 'ALL' ? 'the week' : state.selectedTimetableDay}.</p>
                        </div>
                    </td>
                </tr>
            `;
            return;
        }

        entries.forEach(slot => {
            const tr = document.createElement('tr');
            tr.innerHTML = `
                <td><span class="badge weekday">${escapeHtml(slot.weekday)}</span></td>
                <td><strong style="color:var(--primary); font-family:var(--font-mono);">${escapeHtml(slot.start_time)} - ${escapeHtml(slot.end_time)}</strong></td>
                <td><strong>${escapeHtml(slot.subject)}</strong></td>
                <td>${escapeHtml(slot.teacher)}</td>
                <td>${escapeHtml(slot.classroom)}</td>
                <td style="text-align: right;">
                    <button class="btn-icon" data-edit-slot="${escapeHtml(slot.id)}" title="Edit Slot">
                        <svg viewBox="0 0 24 24" width="15" height="15" fill="none" stroke="currentColor" stroke-width="2"><path d="M11 4H4a2 2 0 0 0-2 2v14a2 2 0 0 0 2 2h14a2 2 0 0 0 2-2v-7"/><path d="M18.5 2.5a2.121 2.121 0 0 1 3 3L12 15l-4 1 1-4 9.5-9.5z"/></svg>
                    </button>
                    <button class="btn-icon danger" data-delete-slot="${escapeHtml(slot.id)}" title="Delete Slot">
                        <svg viewBox="0 0 24 24" width="15" height="15" fill="none" stroke="currentColor" stroke-width="2"><polyline points="3 6 5 6 21 6"/><path d="M19 6v14a2 2 0 0 1-2 2H7a2 2 0 0 1-2-2V6m3 0V4a2 2 0 0 1 2-2h4a2 2 0 0 1 2 2v2"/><line x1="10" y1="11" x2="10" y2="17"/><line x1="14" y1="11" x2="14" y2="17"/></svg>
                    </button>
                </td>
            `;
            tbody.appendChild(tr);
        });

        // Attach action handlers
        tbody.querySelectorAll('[data-edit-slot]').forEach(btn => {
            btn.addEventListener('click', () => {
                const id = btn.dataset.editSlot;
                const slot = entries.find(s => s.id === id);
                if (slot) openEditSlotModal(slot);
            });
        });

        tbody.querySelectorAll('[data-delete-slot]').forEach(btn => {
            btn.addEventListener('click', () => {
                const id = btn.dataset.deleteSlot;
                const slot = entries.find(s => s.id === id);
                if (slot) {
                    showConfirmDialog(
                        'Delete Timetable Slot',
                        `Remove ${slot.subject} scheduled for ${slot.weekday} (${slot.start_time} - ${slot.end_time})?`,
                        async () => {
                            const delRes = await window.API.deleteTimetableEntry(id);
                            if (delRes.success) {
                                showToast('Timetable slot removed', 'success');
                                renderTimetableView();
                                renderDashboardView();
                            } else {
                                showToast(delRes.error, 'error');
                            }
                        }
                    );
                }
            });
        });
    }

    // Weekday Tab filtering
    document.querySelectorAll('#timetable-day-tabs button').forEach(tab => {
        tab.addEventListener('click', () => {
            document.querySelectorAll('#timetable-day-tabs button').forEach(t => t.classList.remove('active'));
            tab.classList.add('active');
            state.selectedTimetableDay = tab.dataset.day;
            renderTimetableView();
        });
    });

    // Add / Edit Slot Modal Logic
    function openAddSlotModal() {
        document.getElementById('modal-timetable-title').textContent = 'Schedule Lecture Slot';
        document.getElementById('tt-id-field').value = '';
        document.getElementById('tt-subject').value = '';
        document.getElementById('tt-teacher').value = '';
        document.getElementById('tt-room').value = 'CR-302 (Smart Hall)';
        document.getElementById('tt-weekday').value = state.selectedTimetableDay === 'ALL' ? 'MONDAY' : state.selectedTimetableDay;
        document.getElementById('tt-start').value = '09:00';
        document.getElementById('tt-end').value = '10:00';
        openModal('modal-timetable');
    }

    function openEditSlotModal(slot) {
        document.getElementById('modal-timetable-title').textContent = 'Edit Lecture Slot';
        document.getElementById('tt-id-field').value = slot.id;
        document.getElementById('tt-subject').value = slot.subject;
        document.getElementById('tt-teacher').value = slot.teacher;
        document.getElementById('tt-room').value = slot.classroom;
        document.getElementById('tt-weekday').value = slot.weekday;
        document.getElementById('tt-start').value = slot.start_time;
        document.getElementById('tt-end').value = slot.end_time;
        openModal('modal-timetable');
    }

    document.getElementById('btn-open-add-slot').addEventListener('click', openAddSlotModal);

    document.getElementById('form-timetable').addEventListener('submit', async (e) => {
        e.preventDefault();
        const id = document.getElementById('tt-id-field').value;
        const payload = {
            subject: document.getElementById('tt-subject').value.trim(),
            teacher: document.getElementById('tt-teacher').value.trim(),
            classroom: document.getElementById('tt-room').value.trim(),
            weekday: document.getElementById('tt-weekday').value,
            start_time: document.getElementById('tt-start').value,
            end_time: document.getElementById('tt-end').value
        };

        const submitBtn = document.getElementById('btn-submit-timetable');
        submitBtn.disabled = true;
        submitBtn.textContent = 'Saving...';

        let res;
        if (id) {
            res = await window.API.updateTimetableEntry(id, payload);
        } else {
            res = await window.API.createTimetableEntry(payload);
        }

        submitBtn.disabled = false;
        submitBtn.textContent = 'Save Schedule';

        if (res.success) {
            closeModal('modal-timetable');
            showToast(id ? 'Lecture slot updated' : 'Lecture slot scheduled successfully', 'success');
            renderTimetableView();
            renderDashboardView();
        } else {
            showToast(res.error, 'error', 4500);
        }
    });

    // -------------------------------------------------------------------------
    // 5. SMART CLASSROOM IoT VIEW
    // -------------------------------------------------------------------------
    async function renderClassroomView() {
        const res = await window.API.getClassroomStatus();
        if (!res.success) {
            showToast(res.error, 'error');
            return;
        }

        const data = res.data;
        state.classroomState = data;

        document.getElementById('iot-temp-display').textContent = `${data.temperature_celsius || 24.0}°C`;
        document.getElementById('iot-occupancy-display').textContent = `${data.occupancy_count || 0} students`;

        const devices = data.devices || [];
        const activeCount = devices.filter(d => d.is_powered).length;
        document.getElementById('iot-active-appliances').textContent = `${activeCount} / ${devices.length} ON`;

        const grid = document.getElementById('classroom-devices-grid');
        grid.innerHTML = '';

        devices.forEach(dev => {
            const card = document.createElement('div');
            card.className = `device-card ${dev.is_powered ? 'powered-on' : ''}`;

            let iconSvg = '';
            if (dev.device_type === 'LIGHT') {
                iconSvg = '<svg viewBox="0 0 24 24" width="22" height="22" fill="none" stroke="currentColor" stroke-width="2"><path d="M9 18h6"/><path d="M10 22h4"/><path d="M12 2a7 7 0 0 0-7 7c0 3 2 5 3 6h8c1-1 3-3 3-6a7 7 0 0 0-7-7z"/></svg>';
            } else if (dev.device_type === 'FAN') {
                iconSvg = '<svg viewBox="0 0 24 24" width="22" height="22" fill="none" stroke="currentColor" stroke-width="2"><circle cx="12" cy="12" r="3"/><path d="M12 9a3 3 0 0 0-3-3H4v6h5a3 3 0 0 0 3-3z"/><path d="M15 12a3 3 0 0 0 3-3V3h-6v5a3 3 0 0 0 3 3z"/><path d="M12 15a3 3 0 0 0 3 3h5v-6h-5a3 3 0 0 0-3 3z"/><path d="M9 12a3 3 0 0 0-3 3v6h6v-5a3 3 0 0 0-3-3z"/></svg>';
            } else {
                iconSvg = '<svg viewBox="0 0 24 24" width="22" height="22" fill="none" stroke="currentColor" stroke-width="2"><rect x="2" y="4" width="20" height="13" rx="2"/><line x1="8" y1="21" x2="16" y2="21"/><line x1="12" y1="17" x2="12" y2="21"/></svg>';
            }

            card.innerHTML = `
                <div>
                    <div class="device-header">
                        <div style="display:flex; align-items:center; gap:8px;">
                            <span style="color:${dev.is_powered ? 'var(--primary-light)' : 'var(--text-muted)'};">${iconSvg}</span>
                            <span class="device-title">${escapeHtml(dev.name)}</span>
                        </div>
                        <span class="badge ${dev.is_powered ? 'active' : 'inactive'}">${dev.is_powered ? 'ACTIVE' : 'OFF'}</span>
                    </div>
                    <div class="device-meta">
                        Location: ${escapeHtml(dev.location || 'CR-302')} | Operating Level: ${dev.level || 0}
                    </div>
                </div>
                <div class="device-switch-wrap">
                    <span style="font-size:13px; font-weight:600; color:var(--text-secondary);">Power Switch</span>
                    <label class="switch">
                        <input type="checkbox" ${dev.is_powered ? 'checked' : ''} data-device-id="${escapeHtml(dev.id)}">
                        <span class="slider"></span>
                    </label>
                </div>
            `;
            grid.appendChild(card);
        });

        // Attach toggle handlers with optimistic update and authoritative rollback
        grid.querySelectorAll('input[type="checkbox"]').forEach(checkbox => {
            checkbox.addEventListener('change', async (e) => {
                const devId = e.target.dataset.deviceId;
                const newPowerState = e.target.checked;
                const level = newPowerState ? 1 : 0;

                const res = await window.API.updateDeviceState(devId, newPowerState, level);
                if (res.success) {
                    showToast(`Device state updated: ${newPowerState ? 'ON' : 'OFF'}`, 'success', 2000);
                    renderClassroomView();
                    renderDashboardView();
                } else {
                    e.target.checked = !newPowerState; // Rollback
                    showToast(res.error, 'error');
                }
            });
        });
    }

    // -------------------------------------------------------------------------
    // 6. REPORTS & ANALYTICS VIEW
    // -------------------------------------------------------------------------
    async function renderReportsView() {
        const res = await window.API.getStudentReports();
        const tbody = document.getElementById('reports-table-body');
        tbody.innerHTML = '';

        // Configure CSV Download URLs
        document.getElementById('btn-export-attendance-csv').href = window.API.getExportCsvUrl('attendance');
        document.getElementById('btn-export-students-csv').href = window.API.getExportCsvUrl('students');

        if (!res.success) {
            tbody.innerHTML = `<tr><td colspan="7" style="text-align:center; padding:24px; color:var(--danger);">${escapeHtml(res.error)}</td></tr>`;
            return;
        }

        const stats = res.data || [];
        if (stats.length === 0) {
            tbody.innerHTML = `<tr><td colspan="7" style="text-align:center; padding:24px; color:var(--text-muted);">No attendance records found for analytics.</td></tr>`;
            return;
        }

        stats.forEach(s => {
            const tr = document.createElement('tr');
            tr.innerHTML = `
                <td><strong style="color:var(--navy-900); font-family:var(--font-mono);">${escapeHtml(s.roll_number)}</strong></td>
                <td><strong>${escapeHtml(s.full_name)}</strong></td>
                <td>${s.total_sessions}</td>
                <td><strong style="color:var(--success);">${s.attended_sessions}</strong></td>
                <td><strong style="color:var(--danger);">${s.absent_sessions}</strong></td>
                <td>
                    <div style="display:flex; align-items:center; gap:8px;">
                        <strong style="color:${s.is_defaulter ? 'var(--danger)' : 'var(--primary)'}; font-size:14px;">${s.attendance_percentage}%</strong>
                        <div style="flex:1; background-color:var(--border-color); height:6px; border-radius:3px; overflow:hidden; max-width:80px;">
                            <div style="width:${Math.min(100, s.attendance_percentage)}%; background-color:${s.is_defaulter ? 'var(--danger)' : 'var(--success)'}; height:100%;"></div>
                        </div>
                    </div>
                </td>
                <td>
                    ${s.is_defaulter ?
                        `<span class="badge defaulter">Shortage (&lt;75%)</span>` :
                        `<span class="badge active">Compliant</span>`
                    }
                </td>
            `;
            tbody.appendChild(tr);
        });
    }

    // -------------------------------------------------------------------------
    // Initial Application Bootstrap
    // -------------------------------------------------------------------------
    document.addEventListener('DOMContentLoaded', () => {
        navigateTo('dashboard');
    });

})();
