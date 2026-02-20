#include <Arduino.h>
#include <esp_timer.h>

class ESP32Sequencer {
  private:
    // Puntero a la función del usuario
    void (*userCallback)(void) = nullptr;

    // Variables de control
    float _bpm = 120.0;
    bool _running = false;
    esp_timer_handle_t _timer = nullptr;
    TaskHandle_t _taskHandle = nullptr;
    
    // Constant: 24 PPQN
    const int PPQN = 24;

    // ISR
    static void IRAM_ATTR onTimerTick(void* arg) {
      ESP32Sequencer* self = (ESP32Sequencer*)arg;
      if (self->_taskHandle != nullptr) {
        // Despierta a la tarea en el Core 0
        vTaskNotifyGiveFromISR(self->_taskHandle, NULL);
      }
    }

    // Task
    static void sequencerTask(void* arg) {
      ESP32Sequencer* self = (ESP32Sequencer*)arg;
      
      while (true) {
        // Espera a ser "despertada" por el timer. 
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        // Run SEQ code
        if (self->_running && self->userCallback != nullptr) {
          self->userCallback();
        }
      }
    }

  public:
    ESP32Sequencer() {}

    void setCallback(void (*func)(void)) {
      userCallback = func;
    }
    void setBPM(float bpm) {
      if (bpm < 1.0) bpm = 1.0;
      _bpm = bpm;
      // Si está corriendo, actualizamos el timer al vuelo
      if (_running && _timer != nullptr) {
        stop();
        start();
      }
    }

    void start() {
      if (_running) return;

      if (_taskHandle == nullptr) {
        xTaskCreatePinnedToCore(
          sequencerTask,   // Función de la tarea
          "SeqTask",       // Nombre
          4096,            // Stack size 
          this,            // Parámetros
          10,              // Prioridad
          &_taskHandle,    // Handle
          0                // CORE 0
        );
      }

      // Calcular microsegundos por tick
      // 60 segundos * 1,000,000 us / (BPM * 24)
      uint64_t interval = (60000000UL) / (_bpm * PPQN);

      //Configurar e iniciar el Timer de Hardware de Alta Resolución
      const esp_timer_create_args_t timer_args = {
        .callback = &onTimerTick,
        .arg = this,
        .name = "seq_timer"
      };

      esp_timer_create(&timer_args, &_timer);
      esp_timer_start_periodic(_timer, interval);

      _running = true;

      xTaskNotifyGive(_taskHandle);
    }

    void stop() {
      if (!_running) return;
      
      if (_timer != nullptr) {
        esp_timer_stop(_timer);
        esp_timer_delete(_timer);
        _timer = nullptr;
      }
      _running = false;
    }

    bool isRunning() {
      return _running;
    }
};

