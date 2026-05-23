// ==================== WEBSOCKET ====================
var gateway = `ws://${window.location.hostname}/ws`;
var websocket;

window.addEventListener('load', onLoad);

function onLoad(event) {
    initWebSocket();
}

function onOpen(event) {
    console.log('Connection opened');
}

function onClose(event) {
    console.log('Connection closed');
    setTimeout(initWebSocket, 2000);
}

function initWebSocket() {
    console.log('Trying to open a WebSocket connection…');
    websocket = new WebSocket(gateway);
    websocket.onopen = onOpen;
    websocket.onclose = onClose;
    websocket.onmessage = onMessage;
}

function Send_Data(data) {
    if (websocket && websocket.readyState === WebSocket.OPEN) {
        websocket.send(data);
        console.log("📤 Sent:", data);
    } else {
        console.warn("⚠️ WebSocket not ready!");
        alert("⚠️ WebSocket not connected!");
    }
}

function onMessage(event) {
    console.log("📩 Received:", event.data);
    try {
        var data = JSON.parse(event.data);
        // Can add special handling if needed (e.g., update status)

        // ==================================================
        // Check if sensor data
        if (data.type === "sensor_data") {
            // Update gauges with real values
            // (Check if gaugeTemp, gaugeHumi, and gaugeGas are initialized)
            if (window.gaugeTemp && window.gaugeHumi) {
                window.gaugeTemp.refresh(data.temperature);
                window.gaugeHumi.refresh(data.humidity);
            }
            
            if (window.gaugeGas && data.gas !== undefined) {
                window.gaugeGas.refresh(data.gas);
            }
            
            // Save data to history
            saveSensorData(data.temperature, data.humidity, data.gas || 0);
        }
        
        // Check if control response
        if (data.type === "control_response") {
            if (data.device === "fan_relay") {
                fanRelayState = data.state;
                updateFanRelayButton();
            } else if (data.device === "gas_valve_relay") {
                gasValveRelayState = data.state;
                updateGasValveRelayButton();
            }
        }
        // ==================================================
    } catch (e) {
        console.warn("Not valid JSON:", event.data);
    }
}


// ==================== UI NAVIGATION ====================
let relayList = [];
let deleteTarget = null;
let fanRelayState = false;
let gasValveRelayState = false;

function showSection(id, event) {
    document.querySelectorAll('.section').forEach(sec => sec.classList.add('is-hidden'));
    document.getElementById(id).classList.remove('is-hidden');
    document.querySelectorAll('.nav-item').forEach(i => i.classList.remove('active'));
    event.currentTarget.classList.add('active');
}


// ==================== HOME GAUGES ====================
window.onload = function () {
    // Generate demo data if not available
    generateDemoData();
    
    // Initialize 2 gauges and save to global variable (window.)
    // so other functions can access them
    window.gaugeTemp = new JustGage({
        id: "gauge_temp",
        value: 0, // Initial value
        min: -10,
        max: 50,
        donut: true,
        pointer: false,
        gaugeWidthScale: 0.25,
        gaugeColor: "transparent",
        levelColorsGradient: true,
        levelColors: ["#00BCD4", "#4CAF50", "#FFC107", "#F44336"]
    });

    window.gaugeHumi = new JustGage({
        id: "gauge_humi",
        value: 0, // Initial value
        min: 0,
        max: 100,
        donut: true,
        pointer: false,
        gaugeWidthScale: 0.25,
        gaugeColor: "transparent",
        levelColorsGradient: true,
    });

    window.gaugeGas = new JustGage({
        id: "gauge_gas",
        value: 0, // Initial value
        min: 0,
        max: 10000,
        donut: true,
        pointer: false,
        gaugeWidthScale: 0.25,
        gaugeColor: "transparent",
        levelColorsGradient: true,
        levelColors: ["#90EE90", "#FFD700", "#FF6347"]
    });

    // Initialize charts
    initCharts();
    
    // Update control button status
    updateFanRelayButton();
    updateGasValveRelayButton();

    // setInterval(() => {
    //     gaugeTemp.refresh(Math.floor(Math.random() * 15) + 20);
    //     gaugeHumi.refresh(Math.floor(Math.random() * 40) + 40);
    // }, 3000);
};

// ==================== RELAY CONTROL ====================
function toggleFanRelay() {
    fanRelayState = !fanRelayState;
    const command = JSON.stringify({
        type: "control",
        device: "fan_relay",
        state: fanRelayState
    });
    Send_Data(command);
    updateFanRelayButton();
}

function toggleGasValveRelay() {
    gasValveRelayState = !gasValveRelayState;
    const command = JSON.stringify({
        type: "control",
        device: "gas_valve_relay",
        state: gasValveRelayState
    });
    Send_Data(command);
    updateGasValveRelayButton();
}

function updateFanRelayButton() {
    const btn = document.getElementById('fanRelayBtn');
    const status = document.getElementById('fanRelayStatus');
    if (fanRelayState) {
        btn.classList.add('active');
        status.textContent = 'ON';
    } else {
        btn.classList.remove('active');
        status.textContent = 'OFF';
    }
}

function updateGasValveRelayButton() {
    const btn = document.getElementById('gasValveRelayBtn');
    const status = document.getElementById('gasValveRelayStatus');
    if (gasValveRelayState) {
        btn.classList.add('active');
        status.textContent = 'ON';
    } else {
        btn.classList.remove('active');
        status.textContent = 'OFF';
    }
}


// ==================== DEVICE FUNCTIONS ====================
function openAddRelayDialog() {
    document.getElementById('addRelayDialog').classList.remove('is-hidden');
}
function closeAddRelayDialog() {
    document.getElementById('addRelayDialog').classList.add('is-hidden');
}
function saveRelay() {
    const name = document.getElementById('relayName').value.trim();
    const gpio = document.getElementById('relayGPIO').value.trim();
    if (!name || !gpio) return alert("⚠️ Please fill all fields!");
    relayList.push({ id: Date.now(), name, gpio, state: false });
    renderRelays();
    closeAddRelayDialog();
}
function renderRelays() {
    const container = document.getElementById('relayContainer');
    container.innerHTML = "";
    relayList.forEach(r => {
        const card = document.createElement('div');
        card.className = 'device-card';
        card.innerHTML = `
      <i class="fa-solid fa-bolt device-icon"></i>
      <h3>${r.name}</h3>
      <p>GPIO: ${r.gpio}</p>
            <button class="toggle-btn ${r.state ? 'on' : ''}" onclick="toggleRelayDevice(${r.id})">
        ${r.state ? 'ON' : 'OFF'}
      </button>
      <i class="fa-solid fa-trash delete-icon" onclick="showDeleteDialog(${r.id})"></i>
    `;
        container.appendChild(card);
    });
}
function toggleRelayDevice(id) {
    const relay = relayList.find(r => r.id === id);
    if (relay) {
        relay.state = !relay.state;
        const relayJSON = JSON.stringify({
            page: "device",
            value: {
                name: relay.name,
                status: relay.state ? "ON" : "OFF",
                gpio: relay.gpio
            }
        });
        Send_Data(relayJSON);
        renderRelays();
    }
}
function showDeleteDialog(id) {
    deleteTarget = id;
    document.getElementById('confirmDeleteDialog').classList.remove('is-hidden');
}
function closeConfirmDelete() {
    document.getElementById('confirmDeleteDialog').classList.add('is-hidden');
}
function confirmDelete() {
    relayList = relayList.filter(r => r.id !== deleteTarget);
    renderRelays();
    closeConfirmDelete();
}


// ==================== SETTINGS FORM (BỔ SUNG) ====================
document.getElementById("settingsForm").addEventListener("submit", function (e) {
    e.preventDefault();

    const ssid = document.getElementById("ssid").value.trim();
    const password = document.getElementById("password").value.trim();
    const token = document.getElementById("token").value.trim();
    const server = document.getElementById("server").value.trim();
    const port = document.getElementById("port").value.trim();

    const settingsJSON = JSON.stringify({
        page: "setting",
        value: {
            ssid: ssid,
            password: password,
            token: token,
            server: server,
            port: port
        }
    });

    Send_Data(settingsJSON);
    alert("✅ Configuration sent to device!");
});


// ==================== CHART & HISTORY DATA ====================
let tempChart, humiChart;
let sensorHistory = JSON.parse(localStorage.getItem('sensorHistory')) || [];

// Save sensor data
function saveSensorData(temp, humi, gas) {
    const now = new Date();
    sensorHistory.push({
        timestamp: now.getTime(),
        temperature: temp,
        humidity: humi,
        gas: gas
    });
    
    // Limit stored data (max 10080 points = 1 week if updated every minute)
    if (sensorHistory.length > 10080) {
        sensorHistory.shift();
    }
    
    localStorage.setItem('sensorHistory', JSON.stringify(sensorHistory));
    updateCharts();
}

// Get data by time range
function getDataByTimeRange(range) {
    const now = new Date().getTime();
    let startTime;
    
    switch(range) {
        case '1day':
            startTime = now - (24 * 60 * 60 * 1000);
            break;
        case '3days':
            startTime = now - (3 * 24 * 60 * 60 * 1000);
            break;
        case '1week':
            startTime = now - (7 * 24 * 60 * 60 * 1000);
            break;
        default:
            startTime = now - (24 * 60 * 60 * 1000);
    }
    
    return sensorHistory.filter(data => data.timestamp >= startTime);
}

// Initialize charts
function initCharts() {
    // Temperature chart
    const tempCtx = document.getElementById('tempChart').getContext('2d');
    tempChart = new Chart(tempCtx, {
        type: 'line',
        data: {
            labels: [],
            datasets: [{
                label: 'Temperature (°C)',
                data: [],
                borderColor: '#FF6384',
                backgroundColor: 'rgba(255, 99, 132, 0.1)',
                borderWidth: 2,
                tension: 0.4,
                fill: true,
                pointRadius: 3,
                pointBackgroundColor: '#FF6384'
            }]
        },
        options: {
            responsive: true,
            maintainAspectRatio: true,
            plugins: {
                legend: {
                    display: true,
                    labels: { font: { size: 12 } }
                }
            },
            scales: {
                y: {
                    beginAtZero: false,
                    min: -10,
                    max: 50
                }
            }
        }
    });

    // Humidity chart
    const humiCtx = document.getElementById('humiChart').getContext('2d');
    humiChart = new Chart(humiCtx, {
        type: 'line',
        data: {
            labels: [],
            datasets: [{
                label: 'Humidity (%)',
                data: [],
                borderColor: '#36A2EB',
                backgroundColor: 'rgba(54, 162, 235, 0.1)',
                borderWidth: 2,
                tension: 0.4,
                fill: true,
                pointRadius: 3,
                pointBackgroundColor: '#36A2EB'
            }]
        },
        options: {
            responsive: true,
            maintainAspectRatio: true,
            plugins: {
                legend: {
                    display: true,
                    labels: { font: { size: 12 } }
                }
            },
            scales: {
                y: {
                    beginAtZero: true,
                    min: 0,
                    max: 100
                }
            }
        }
    });

    // Gas chart
    const gasCtx = document.getElementById('gasChart').getContext('2d');
    gasChart = new Chart(gasCtx, {
        type: 'line',
        data: {
            labels: [],
            datasets: [{
                label: 'Gas (ppm)',
                data: [],
                borderColor: '#FFA500',
                backgroundColor: 'rgba(255, 165, 0, 0.1)',
                borderWidth: 2,
                tension: 0.4,
                fill: true,
                pointRadius: 3,
                pointBackgroundColor: '#FFA500'
            }]
        },
        options: {
            responsive: true,
            maintainAspectRatio: true,
            plugins: {
                legend: {
                    display: true,
                    labels: { font: { size: 12 } }
                }
            },
            scales: {
                y: {
                    beginAtZero: true,
                    min: 0,
                    max: 10000
                }
            }
        }
    });

    // Update charts with initial data
    updateCharts('1day');
}

// Update charts function
function updateCharts(range) {
    if (!range) range = '1day';
    const data = getDataByTimeRange(range);
    
    // Update filter buttons (no-op if buttons removed)
    const btns = document.querySelectorAll('.filter-btn');
    if (btns.length) {
        btns.forEach(btn => btn.classList.remove('active'));
        const sel = document.querySelector(`[data-range="${range}"]`);
        if (sel) sel.classList.add('active');
    }
    
    // Create time labels
    const labels = data.map(d => {
        const date = new Date(d.timestamp);
        return date.toLocaleTimeString('en-US', { hour: '2-digit', minute: '2-digit' });
    });
    
    // Create value list
    const temps = data.map(d => d.temperature);
    const humis = data.map(d => d.humidity);
    const gases = data.map(d => d.gas || 0);
    
    // Update temperature chart
    tempChart.data.labels = labels;
    tempChart.data.datasets[0].data = temps;
    tempChart.update();
    
    // Update humidity chart
    humiChart.data.labels = labels;
    humiChart.data.datasets[0].data = humis;
    humiChart.update();
    
    // Update gas chart
    gasChart.data.labels = labels;
    gasChart.data.datasets[0].data = gases;
    gasChart.update();
}

// Generate demo data function (for testing, can be deleted later)
function generateDemoData() {
    if (sensorHistory.length > 0) return; // If data already exists, skip generation
    
    const now = new Date().getTime();
    for (let i = 0; i < 168; i++) { // 168 hours = 1 week
        sensorHistory.push({
            timestamp: now - (i * 60 * 60 * 1000),
            temperature: 20 + Math.sin(i / 24) * 5 + Math.random() * 3,
            humidity: 50 + Math.cos(i / 24) * 15 + Math.random() * 5,
            gas: 100 + Math.sin(i / 12) * 50 + Math.random() * 20
        });
    }
    sensorHistory.reverse();
    localStorage.setItem('sensorHistory', JSON.stringify(sensorHistory));
}
