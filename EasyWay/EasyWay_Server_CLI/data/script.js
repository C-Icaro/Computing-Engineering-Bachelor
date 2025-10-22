// EasyWay - JavaScript para Interface Web Local

class EasyWayApp {
    constructor() {
        this.currentTab = 'devices';
        this.devices = [];
        this.messages = [];
        this.logs = [];
        this.refreshInterval = null;
        
        this.init();
    }
    
    init() {
        this.setupEventListeners();
        this.startAutoRefresh();
        this.loadInitialData();
    }
    
    setupEventListeners() {
        // Auto-refresh quando a página ganha foco
        window.addEventListener('focus', () => {
            this.refreshAllData();
        });
        
        // Atualizar status do servidor periodicamente
        setInterval(() => {
            this.checkServerStatus();
        }, 5000);
    }
    
    startAutoRefresh() {
        this.refreshInterval = setInterval(() => {
            this.refreshAllData();
        }, 10000); // Atualizar a cada 10 segundos
    }
    
    stopAutoRefresh() {
        if (this.refreshInterval) {
            clearInterval(this.refreshInterval);
            this.refreshInterval = null;
        }
    }
    
    async loadInitialData() {
        await Promise.all([
            this.loadDevices(),
            this.loadLogs()
        ]);
    }
    
    async refreshAllData() {
        await Promise.all([
            this.loadDevices(),
            this.loadLogs()
        ]);
    }
    
    // Tab Management
    showTab(tabName) {
        // Esconder todas as abas
        document.querySelectorAll('.tab-content').forEach(tab => {
            tab.classList.remove('active');
        });
        
        // Remover classe active de todos os botões
        document.querySelectorAll('.tab-button').forEach(button => {
            button.classList.remove('active');
        });
        
        // Mostrar aba selecionada
        document.getElementById(tabName + '-tab').classList.add('active');
        
        // Adicionar classe active ao botão correspondente
        event.target.classList.add('active');
        
        this.currentTab = tabName;
        
        // Carregar dados específicos da aba se necessário
        if (tabName === 'messages') {
            this.loadMessages();
        }
    }
    
    // Server Status
    async checkServerStatus() {
        try {
            const response = await fetch('/api/status');
            if (response.ok) {
                this.updateServerStatus(true);
            } else {
                this.updateServerStatus(false);
            }
        } catch (error) {
            this.updateServerStatus(false);
        }
    }
    
    updateServerStatus(isOnline) {
        const statusElement = document.getElementById('serverStatus');
        const statusDot = document.querySelector('.status-dot');
        
        if (isOnline) {
            statusElement.textContent = 'Servidor Online';
            statusDot.className = 'status-dot online';
        } else {
            statusElement.textContent = 'Servidor Offline';
            statusDot.className = 'status-dot offline';
        }
    }
    
    // Devices Management
    async loadDevices() {
        try {
            const response = await fetch('/api/devices');
            if (response.ok) {
                const data = await response.json();
                this.devices = data;
                this.updateDevicesDisplay();
                this.updateDeviceSelector();
                this.updateStats();
            }
        } catch (error) {
            console.error('Erro ao carregar dispositivos:', error);
            this.showToast('Erro ao carregar dispositivos', 'error');
        }
    }
    
    updateDevicesDisplay() {
        const devicesGrid = document.getElementById('devicesGrid');
        devicesGrid.innerHTML = '';
        
        if (this.devices.length === 0) {
            devicesGrid.innerHTML = `
                <div class="no-devices">
                    <i class="fas fa-microchip" style="font-size: 3rem; color: #a0aec0; margin-bottom: 20px;"></i>
                    <h3>Nenhum dispositivo conectado</h3>
                    <p>Os dispositivos aparecerão aqui quando se conectarem ao sistema.</p>
                </div>
            `;
            return;
        }
        
        this.devices.forEach(device => {
            const deviceCard = this.createDeviceCard(device);
            devicesGrid.appendChild(deviceCard);
        });
    }
    
    createDeviceCard(device) {
        const card = document.createElement('div');
        card.className = `device-card ${device.isOnline ? 'online' : 'offline'}`;
        
        const lastSeen = this.formatTimestamp(device.lastSeen);
        const statusClass = device.isOnline ? 'online' : 'offline';
        const statusText = device.isOnline ? 'Online' : 'Offline';
        
        card.innerHTML = `
            <div class="device-header">
                <div class="device-id">${device.id}</div>
                <div class="device-status ${statusClass}">${statusText}</div>
            </div>
            <div class="device-info">
                <div class="device-info-item">
                    <div class="device-info-label">Status da Porta</div>
                    <div class="device-info-value">${device.status || 'Desconhecido'}</div>
                </div>
                <div class="device-info-item">
                    <div class="device-info-label">Último Comando</div>
                    <div class="device-info-value">${device.lastCommand || 'Nenhum'}</div>
                </div>
                <div class="device-info-item">
                    <div class="device-info-label">Última Mensagem</div>
                    <div class="device-info-value">${device.lastMessage || 'Nenhuma'}</div>
                </div>
                <div class="device-info-item">
                    <div class="device-info-label">Última Atividade</div>
                    <div class="device-info-value">${lastSeen}</div>
                </div>
            </div>
            <div class="device-actions">
                <button class="btn btn-success" onclick="app.sendCommandToDevice('${device.id}', 'abrir')">
                    <i class="fas fa-unlock"></i> Abrir
                </button>
                <button class="btn btn-danger" onclick="app.sendCommandToDevice('${device.id}', 'fechar')">
                    <i class="fas fa-lock"></i> Fechar
                </button>
                <button class="btn btn-info" onclick="app.sendCommandToDevice('${device.id}', 'status')">
                    <i class="fas fa-info-circle"></i> Status
                </button>
            </div>
        `;
        
        return card;
    }
    
    updateDeviceSelector() {
        const deviceSelect = document.getElementById('deviceSelect');
        deviceSelect.innerHTML = '<option value="">Escolha um dispositivo...</option>';
        
        this.devices.forEach(device => {
            const option = document.createElement('option');
            option.value = device.id;
            option.textContent = `${device.id} (${device.isOnline ? 'Online' : 'Offline'})`;
            deviceSelect.appendChild(option);
        });
    }
    
    updateStats() {
        const onlineCount = this.devices.filter(device => device.isOnline).length;
        const totalCount = this.devices.length;
        
        document.getElementById('onlineCount').textContent = onlineCount;
        document.getElementById('totalCount').textContent = totalCount;
    }
    
    // Command Management
    async sendCommand(command) {
        const deviceSelect = document.getElementById('deviceSelect');
        const deviceId = deviceSelect.value;
        
        if (!deviceId) {
            this.showToast('Selecione um dispositivo primeiro', 'warning');
            return;
        }
        
        await this.sendCommandToDevice(deviceId, command);
    }
    
    async sendCommandToDevice(deviceId, command) {
        try {
            const response = await fetch(`/api/devices/${deviceId}/control`, {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                },
                body: JSON.stringify({ command: command })
            });
            
            if (response.ok) {
                this.showToast(`Comando '${command}' enviado para ${deviceId}`, 'success');
                // Atualizar dispositivos após enviar comando
                setTimeout(() => this.loadDevices(), 1000);
            } else {
                this.showToast(`Erro ao enviar comando para ${deviceId}`, 'error');
            }
        } catch (error) {
            console.error('Erro ao enviar comando:', error);
            this.showToast('Erro de conexão ao enviar comando', 'error');
        }
    }
    
    // Message Management
    async sendMessage() {
        const deviceSelect = document.getElementById('deviceSelect');
        const messageText = document.getElementById('messageText');
        
        const deviceId = deviceSelect.value;
        const message = messageText.value.trim();
        
        if (!deviceId) {
            this.showToast('Selecione um dispositivo primeiro', 'warning');
            return;
        }
        
        if (!message) {
            this.showToast('Digite uma mensagem', 'warning');
            return;
        }
        
        try {
            const response = await fetch('/api/message', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                },
                body: JSON.stringify({
                    deviceId: deviceId,
                    message: message
                })
            });
            
            if (response.ok) {
                this.showToast(`Mensagem enviada para ${deviceId}`, 'success');
                messageText.value = '';
                // Adicionar mensagem à lista local
                this.addMessageToLocalList(deviceId, message, true);
            } else {
                this.showToast(`Erro ao enviar mensagem para ${deviceId}`, 'error');
            }
        } catch (error) {
            console.error('Erro ao enviar mensagem:', error);
            this.showToast('Erro de conexão ao enviar mensagem', 'error');
        }
    }
    
    addMessageToLocalList(deviceId, message, isSent = false) {
        const messageItem = {
            id: Date.now(),
            deviceId: deviceId,
            message: message,
            timestamp: Date.now(),
            isSent: isSent
        };
        
        this.messages.unshift(messageItem);
        this.updateMessagesDisplay();
    }
    
    loadMessages() {
        // Por enquanto, usar mensagens locais
        // Em uma implementação real, isso viria do servidor
        this.updateMessagesDisplay();
    }
    
    updateMessagesDisplay() {
        const messagesContainer = document.getElementById('messagesContainer');
        messagesContainer.innerHTML = '';
        
        if (this.messages.length === 0) {
            messagesContainer.innerHTML = `
                <div class="no-messages">
                    <i class="fas fa-comments" style="font-size: 3rem; color: #a0aec0; margin-bottom: 20px;"></i>
                    <h3>Nenhuma mensagem</h3>
                    <p>As mensagens trocadas com os dispositivos aparecerão aqui.</p>
                </div>
            `;
            return;
        }
        
        this.messages.forEach(message => {
            const messageElement = this.createMessageElement(message);
            messagesContainer.appendChild(messageElement);
        });
    }
    
    createMessageElement(message) {
        const messageDiv = document.createElement('div');
        messageDiv.className = 'message-item';
        
        const time = this.formatTimestamp(message.timestamp);
        const direction = message.isSent ? 'Enviado para' : 'Recebido de';
        
        messageDiv.innerHTML = `
            <div class="message-header">
                <div class="message-sender">${direction} ${message.deviceId}</div>
                <div class="message-time">${time}</div>
            </div>
            <div class="message-content">${message.message}</div>
        `;
        
        return messageDiv;
    }
    
    clearMessages() {
        this.messages = [];
        this.updateMessagesDisplay();
        this.showToast('Mensagens limpas', 'success');
    }
    
    // Logs Management
    async loadLogs() {
        try {
            const response = await fetch('/api/logs');
            if (response.ok) {
                const data = await response.json();
                this.logs = data;
                this.updateLogsDisplay();
            }
        } catch (error) {
            console.error('Erro ao carregar logs:', error);
            this.showToast('Erro ao carregar logs', 'error');
        }
    }
    
    updateLogsDisplay() {
        const logsContainer = document.getElementById('logsContainer');
        logsContainer.innerHTML = '';
        
        if (this.logs.length === 0) {
            logsContainer.innerHTML = `
                <div class="no-logs">
                    <i class="fas fa-list-alt" style="font-size: 3rem; color: #a0aec0; margin-bottom: 20px;"></i>
                    <h3>Nenhum log disponível</h3>
                    <p>Os logs do sistema aparecerão aqui conforme as atividades forem registradas.</p>
                </div>
            `;
            return;
        }
        
        this.logs.forEach(log => {
            const logElement = this.createLogElement(log);
            logsContainer.appendChild(logElement);
        });
    }
    
    createLogElement(log) {
        const logDiv = document.createElement('div');
        logDiv.className = 'log-item';
        
        logDiv.innerHTML = `
            <div class="log-time">${log.formattedTime}</div>
            <div class="log-device">${log.deviceId}</div>
            <div class="log-command">${log.command}</div>
            <div class="log-details">${log.details || ''}</div>
        `;
        
        return logDiv;
    }
    
    // Utility Functions
    formatTimestamp(timestamp) {
        const date = new Date(timestamp);
        return date.toLocaleTimeString('pt-BR', {
            hour: '2-digit',
            minute: '2-digit',
            second: '2-digit'
        });
    }
    
    showToast(message, type = 'info') {
        const toastContainer = document.getElementById('toastContainer');
        
        const toast = document.createElement('div');
        toast.className = `toast ${type}`;
        toast.textContent = message;
        
        toastContainer.appendChild(toast);
        
        // Remover toast após 3 segundos
        setTimeout(() => {
            toast.remove();
        }, 3000);
    }
    
    // Public methods for buttons
    refreshDevices() {
        this.loadDevices();
        this.showToast('Dispositivos atualizados', 'success');
    }
    
    refreshLogs() {
        this.loadLogs();
        this.showToast('Logs atualizados', 'success');
    }
}

// Global functions for HTML onclick events
function showTab(tabName) {
    app.showTab(tabName);
}

function sendCommand(command) {
    app.sendCommand(command);
}

function sendMessage() {
    app.sendMessage();
}

function refreshDevices() {
    app.refreshDevices();
}

function refreshLogs() {
    app.refreshLogs();
}

function clearMessages() {
    app.clearMessages();
}

// Initialize app when page loads
let app;
document.addEventListener('DOMContentLoaded', () => {
    app = new EasyWayApp();
});
