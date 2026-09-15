flowchart TD
    IRQ["ISR / DMA Callback"]
    SENSOR["SensorTask<br/>I2C・ADC Owner"]
    ALARM["AlarmTask<br/>Deadline Critical"]
    CMD["CommandTask<br/>UART RX Parser"]
    STORE["StorageTask<br/>SPI Owner"]
    TX["UartTxTask<br/>UART TX Owner"]
    HEALTH["HealthTask<br/>Supervisor"]
    FLASH["W25Q64"]
    USER["PC / ESP8266"]

    IRQ --> SENSOR
    IRQ --> CMD
    SENSOR --> ALARM
    SENSOR --> STORE
    SENSOR --> TX
    CMD --> ALARM
    CMD --> STORE
    CMD --> TX
    ALARM --> STORE
    ALARM --> TX
    STORE --> FLASH
    TX --> USER
    SENSOR --> HEALTH
    ALARM --> HEALTH
    STORE --> HEALTH
    CMD --> HEALTH

    