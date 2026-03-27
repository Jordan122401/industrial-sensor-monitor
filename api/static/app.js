function setText(id, value) {
    const el = document.getElementById(id);
    if (el) el.textContent = value;
}

function showError(message) {
    const box = document.getElementById("errorBox");
    if (!box) return;
    box.classList.remove("hidden");
    box.textContent = message;
}

function clearError() {
    const box = document.getElementById("errorBox");
    if (!box) return;
    box.classList.add("hidden");
    box.textContent = "";
}

async function getJson(url) {
    const response = await fetch(url, { cache: "no-store" });
    const data = await response.json();

    if (!response.ok) {
        throw new Error(data.message || "Request failed");
    }

    return data;
}

async function tryGetJson(url, fallbackValue) {
    try {
        return await getJson(url);
    } catch (error) {
        return fallbackValue;
    }
}

function formatNumber(value, decimals = 1) {
    if (value === null || value === undefined || value === "") return "--";
    const num = Number(value);
    if (Number.isNaN(num)) return "--";
    return num.toFixed(decimals);
}

function setDot(id, state) {
    const dot = document.getElementById(id);
    if (!dot) return;

    dot.className = "w-2 h-2 rounded-full shrink-0";

    if (state === "good") dot.classList.add("bg-sky-400");
    else if (state === "warn") dot.classList.add("bg-orange-400");
    else if (state === "bad") dot.classList.add("bg-rose-400");
    else dot.classList.add("bg-slate-500");
}

function computeSystemState(latest, apiStatus) {
    if (apiStatus && apiStatus !== "UNKNOWN") {
        return apiStatus;
    }

    if (!latest || Object.keys(latest).length === 0) {
        return "UNKNOWN";
    }

    const distance = Number(latest.distance_cm);
    const orientation = (latest.orientation || "").toUpperCase();

    if (!Number.isNaN(distance) && distance < 20) {
        return "ALERT";
    }

    if (orientation && orientation !== "FLAT" && orientation !== "UNKNOWN") {
        return "UNSTABLE_PLATFORM";
    }

    return "SAFE";
}

function updateStatusBadge(state) {
    const text = document.getElementById("systemStatusBadgeText");
    const dot = document.getElementById("systemStatusDot");

    if (!text || !dot) return;

    text.textContent = state;

    dot.className = "status-dot";

    if (state === "ALERT") {
        text.className = "text-[10px] font-mono text-rose-300 uppercase";
        dot.classList.add("bg-rose-400", "pulse-critical");
    } else if (state === "UNSTABLE_PLATFORM" || state === "OBSTACLE_NEAR" || state === "WARNING") {
        text.className = "text-[10px] font-mono text-orange-300 uppercase";
        dot.classList.add("bg-orange-400", "pulse-warning");
    } else {
        text.className = "text-[10px] font-mono text-sky-300 uppercase";
        dot.classList.add("bg-sky-400", "pulse-safe");
    }
}

function updateAlertPanel(state, latest) {
    const alertPanel = document.getElementById("alertPanel");
    const alertTitle = document.getElementById("alertTitle");
    const alertSubtitle = document.getElementById("alertSubtitle");
    const alertIcon = document.getElementById("alertIcon");
    const systemNotes = document.getElementById("systemNotes");

    if (!alertPanel || !alertTitle || !alertSubtitle || !alertIcon || !systemNotes) return;

    const distance = latest?.distance_cm;
    const orientation = latest?.orientation || "UNKNOWN";

    alertPanel.className = "flex items-center justify-between p-3 rounded-lg border-l-2";

    if (state === "ALERT") {
        alertPanel.classList.add("bg-rose-500/10", "border-rose-400");
        alertTitle.textContent = "Obstacle Detected";
        alertSubtitle.textContent = `CRITICAL • ${formatNumber(distance)} cm range`;
        alertIcon.textContent = "error";
        alertIcon.className = "material-symbols-outlined text-rose-300 pulse-critical";
        systemNotes.textContent = "Distance dropped below the alert threshold. The logger is still live, but the monitored object is too close to the ultrasonic sensor.";
    } else if (state === "UNSTABLE_PLATFORM") {
        alertPanel.classList.add("bg-orange-500/10", "border-orange-400");
        alertTitle.textContent = "Orientation Warning";
        alertSubtitle.textContent = `WARNING • Orientation is ${orientation}`;
        alertIcon.textContent = "warning";
        alertIcon.className = "material-symbols-outlined text-orange-300 pulse-warning";
        systemNotes.textContent = "The platform is not flat right now. BLE logging and Flask are still working, but the device orientation changed from its normal stable position.";
    } else {
        alertPanel.classList.add("bg-sky-500/10", "border-sky-400");
        alertTitle.textContent = "System Safe";
        alertSubtitle.textContent = "NOMINAL • Live telemetry healthy";
        alertIcon.textContent = "monitor_heart";
        alertIcon.className = "material-symbols-outlined text-sky-300 pulse-safe";
        systemNotes.textContent = "Live ESP32 sensor data is reaching the Raspberry Pi, being written into SQLite, and displayed by Flask without active alert conditions.";
    }
}

function updatePlatformStability(orientation) {
    const bar = document.getElementById("platformStabilityBar");
    const text = document.getElementById("platformStabilityText");

    if (!bar || !text) return;

    const value = (orientation || "").toUpperCase();

    let width = "20%";
    let label = "No reliable orientation yet";
    let barClass = "bg-slate-500";

    if (value === "FLAT") {
        width = "94%";
        label = "Platform balanced";
        barClass = "bg-sky-400";
    } else if (value && value !== "UNKNOWN") {
        width = "55%";
        label = `Platform shifted: ${value}`;
        barClass = "bg-orange-400";
    }

    bar.style.width = width;
    bar.className = `h-full transition-all duration-300 ${barClass}`;
    text.textContent = label.toUpperCase();
}

function computeDelta(currentValue, previousValue, unitSuffix = "") {
    const current = Number(currentValue);
    const previous = Number(previousValue);

    if (Number.isNaN(current) || Number.isNaN(previous)) return "No trend yet";

    const delta = current - previous;

    if (Math.abs(delta) < 0.1) return "Stable";
    if (delta > 0) return `+${delta.toFixed(1)} ${unitSuffix}`.trim();
    return `${delta.toFixed(1)} ${unitSuffix}`.trim();
}

function buildTable(rows) {
    const body = document.getElementById("tableBody");
    if (!body) return;

    body.innerHTML = "";

    if (!rows || rows.length === 0) {
        body.innerHTML = `<tr><td class="px-6 py-4 text-slate-300" colspan="6">No data yet.</td></tr>`;
        return;
    }

    for (const row of rows.slice(0, 10)) {
        const distance = Number(row.distance_cm);
        const orientation = (row.orientation || "UNKNOWN").toUpperCase();

        let statusText = "SAFE";
        let statusClass = "text-sky-300 bg-sky-400/10";

        if (!Number.isNaN(distance) && distance < 20) {
            statusText = "ALERT";
            statusClass = "text-rose-300 bg-rose-400/10";
        } else if (orientation && orientation !== "FLAT" && orientation !== "UNKNOWN") {
            statusText = "UNSTABLE";
            statusClass = "text-orange-300 bg-orange-400/10";
        }

        const tr = document.createElement("tr");
        tr.className = "hover:bg-white/5 transition-colors";
        tr.innerHTML = `
            <td class="px-6 py-4 text-slate-300">${row.timestamp || "--"}</td>
            <td class="px-6 py-4">${formatNumber(row.temperature)}</td>
            <td class="px-6 py-4">${formatNumber(row.humidity)}</td>
            <td class="px-6 py-4">${formatNumber(row.distance_cm)}</td>
            <td class="px-6 py-4">${row.orientation || "UNKNOWN"}</td>
            <td class="px-6 py-4">
                <span class="${statusClass} px-2 py-0.5 rounded uppercase text-[9px]">${statusText}</span>
            </td>
        `;
        body.appendChild(tr);
    }
}

async function refreshDashboard() {
    try {
        clearError();

        const health = await getJson("/api/health");
        const latest = await tryGetJson("/api/sensor-data/latest", {});
        const apiStatusResponse = await tryGetJson("/api/status", { status: "UNKNOWN", timestamp: null });
        const rows = await tryGetJson("/api/sensor-data", []);

        const currentState = computeSystemState(latest, apiStatusResponse.status);
        const previousRow = rows.length > 1 ? rows[1] : null;

        updateStatusBadge(currentState);
        updateAlertPanel(currentState, latest);
        updatePlatformStability(latest.orientation);

        setText("temperatureValue", formatNumber(latest.temperature));
        setText("humidityValue", formatNumber(latest.humidity));
        setText("distanceValue", formatNumber(latest.distance_cm));
        setText("orientationValue", latest.orientation || "--");

        setText("overviewTemp", `${formatNumber(latest.temperature)} °C`);
        setText("overviewHumidity", `${formatNumber(latest.humidity)} %`);
        setText("overviewDistance", `${formatNumber(latest.distance_cm)} cm`);

        setText("tempTrend", previousRow ? `${computeDelta(latest.temperature, previousRow.temperature, "°C")} since last sample` : "No trend yet");
        setText("humidityTrend", previousRow ? `${computeDelta(latest.humidity, previousRow.humidity, "%")} since last sample` : "No trend yet");
        setText("distanceTrend", previousRow ? `${computeDelta(latest.distance_cm, previousRow.distance_cm, "cm")} since last sample` : "No trend yet");
        setText("orientationNote", latest.orientation ? `Current state: ${latest.orientation}` : "No orientation yet");

        setText("databaseStatus", health.db_exists ? "Connected" : "Missing");
        setText("tablesStatus", health.required_tables_present ? "Ready" : "Missing");
        setText("apiStatus", "Online");
        setText("streamStatus", Object.keys(latest).length ? "Receiving data" : "No live rows yet");

        setDot("dbDot", health.db_exists ? "good" : "bad");
        setDot("tablesDot", health.required_tables_present ? "good" : "bad");
        setDot("apiDot", "good");
        setDot("streamDot", Object.keys(latest).length ? "good" : "warn");
        setDot("updateDot", latest.timestamp || apiStatusResponse.timestamp ? "good" : "warn");

        const lastUpdateValue = latest.timestamp || apiStatusResponse.timestamp || "No timestamp yet";
        setText("lastUpdate", lastUpdateValue);
        setText("lastSync", `LAST SYNC: ${lastUpdateValue}`);

        const latestJson = document.getElementById("latestJson");
        if (latestJson) {
            latestJson.textContent = JSON.stringify(latest, null, 2);
        }

        buildTable(rows);
    } catch (error) {
        showError(error.message);
        setText("systemStatusBadgeText", "ERROR");
        setText("databaseStatus", "CHECK FAILED");
        setText("tablesStatus", "CHECK FAILED");
        setText("apiStatus", "OFFLINE");
        setText("streamStatus", "UNKNOWN");

        setDot("dbDot", "bad");
        setDot("tablesDot", "bad");
        setDot("apiDot", "bad");
        setDot("streamDot", "bad");
        setDot("updateDot", "bad");
    }
}

refreshDashboard();
setInterval(refreshDashboard, 2000);