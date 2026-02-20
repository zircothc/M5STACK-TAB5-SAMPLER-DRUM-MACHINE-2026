/**
 * @brief Envía un mensaje MIDI de 3 bytes al dispositivo conectado.
 * 
 * @param status_byte El byte de estado (ej. 0x90 para Note On, 0x80 para Note Off).
 * @param channel Canal MIDI (1-16).
 * @param data1 Primer byte de datos (ej. número de nota).
 * @param data2 Segundo byte de datos (ej. velocidad).
 */
void send_midi_message(uint8_t status_byte, uint8_t channel, uint8_t data1, uint8_t data2) {
  // Comprobar si el dispositivo MIDI está listo para enviar datos
  if (!isMIDIReady || MIDIOut == NULL) {
    return;
  }

  uint8_t midi_packet[4];
  
  uint8_t command = status_byte & 0xF0;
  midi_packet[0] = command >> 4;
  midi_packet[1] = status_byte | (channel - 1);
  midi_packet[2] = data1;
  midi_packet[3] = data2;

  // Copiar el paquete al buffer de transferencia de salida
  memcpy(MIDIOut->data_buffer, midi_packet, 4);
  MIDIOut->num_bytes = 4;

  // Enviar
  esp_err_t err = usb_host_transfer_submit(MIDIOut);
  if (err != ESP_OK) {
    ESP_LOGE("send_midi_message", "Fallo al enviar el paquete MIDI: %x", err);
  }
}

const uint8_t real_key[32]={
24,25,26,27,28,29,30,31,
16,17,18,19,20,21,22,23,  
8,9,10,11,12,13,14,15,  
0,1,2,3,4,5,6,7
};

static void midi_transfer_cb(usb_transfer_t *transfer) {
  if (Device_Handle == transfer->device_handle) {
    // Comprobar si es una transferencia de ENTRADA
    if (transfer->bEndpointAddress & USB_B_ENDPOINT_ADDRESS_EP_DIR_MASK) {
      if (transfer->status == 0) {
        uint8_t *const p = transfer->data_buffer;
        for (int i = 0; i < transfer->actual_num_bytes; i += 4) {
          parse_midi_message(&p[i]);
        }
        // Volver a poner el buffer a escuchar
        esp_err_t err = usb_host_transfer_submit(transfer);
        if (err != ESP_OK) {
          ESP_LOGE("", "usb_host_transfer_submit In fail: %x", err);
        }
      } else if (transfer->status != USB_TRANSFER_STATUS_NO_DEVICE) {
          ESP_LOGW("", "Transferencia IN con error, status %d", transfer->status);
      }
    } else {
      if (transfer->status != 0) {
        ESP_LOGW("", "Transferencia OUT con error, status %d", transfer->status);
      }
    }
  }
}

void check_interface_desc_MIDI(const void *p) {
  const usb_intf_desc_t *intf = (const usb_intf_desc_t *)p;
  if ((intf->bInterfaceClass == USB_CLASS_AUDIO) &&
      (intf->bInterfaceSubClass == 3) &&
      (intf->bInterfaceProtocol == 0))
  {
    isMIDI = true;
    ESP_LOGI("", "Dispositivo MIDI encontrado. Reclamando interfaz...");
    esp_err_t err = usb_host_interface_claim(Client_Handle, Device_Handle,
        intf->bInterfaceNumber, intf->bAlternateSetting);
    if (err != ESP_OK) ESP_LOGE("", "usb_host_interface_claim failed: %x", err);
  }
}

void prepare_endpoints(const void *p) {
  const usb_ep_desc_t *endpoint = (const usb_ep_desc_t *)p;
  esp_err_t err;

  if ((endpoint->bmAttributes & USB_BM_ATTRIBUTES_XFERTYPE_MASK) != USB_BM_ATTRIBUTES_XFER_BULK) {
    ESP_LOGW("", "Endpoint no es de tipo BULK: 0x%02x", endpoint->bmAttributes);
    return;
  }
  
  if (endpoint->bEndpointAddress & USB_B_ENDPOINT_ADDRESS_EP_DIR_MASK) { // Endpoint IN
    ESP_LOGI("", "Configurando %d buffers para Endpoint IN 0x%02x...", MIDI_IN_BUFFERS, endpoint->bEndpointAddress);
    for (int i = 0; i < MIDI_IN_BUFFERS; i++) {
      err = usb_host_transfer_alloc(endpoint->wMaxPacketSize, 0, &MIDIIn[i]);
      if (err != ESP_OK) {
        MIDIIn[i] = NULL;
        ESP_LOGE("", "usb_host_transfer_alloc In fail: %x", err);
      }
      else {
        MIDIIn[i]->device_handle = Device_Handle;
        MIDIIn[i]->bEndpointAddress = endpoint->bEndpointAddress;
        MIDIIn[i]->callback = midi_transfer_cb;
        MIDIIn[i]->context = (void *)i;
        MIDIIn[i]->num_bytes = endpoint->wMaxPacketSize;
        err = usb_host_transfer_submit(MIDIIn[i]);
        if (err != ESP_OK) {
          ESP_LOGE("", "usb_host_transfer_submit In fail: %x", err);
        }
      }
    }
  }
  else { // Endpoint OUT
    ESP_LOGI("", "Configurando Endpoint OUT 0x%02x...", endpoint->bEndpointAddress);
    err = usb_host_transfer_alloc(endpoint->wMaxPacketSize, 0, &MIDIOut);
    if (err != ESP_OK) {
      MIDIOut = NULL;
      ESP_LOGE("", "usb_host_transfer_alloc Out fail: %x", err);
      return;
    }
    MIDIOut->device_handle = Device_Handle;
    MIDIOut->bEndpointAddress = endpoint->bEndpointAddress;
    MIDIOut->callback = midi_transfer_cb;
    MIDIOut->context = NULL;
  }

  if ((MIDIOut != NULL) && (MIDIIn[0] != NULL)) {
    isMIDIReady = true;
    ESP_LOGI("", "Dispositivo MIDI listo para enviar y recibir.");
  }
}

void show_config_desc_full(const usb_config_desc_t *config_desc)
{
  const uint8_t *p = &config_desc->val[0];
  uint8_t bLength;
  for (int i = 0; i < config_desc->wTotalLength; i+=bLength, p+=bLength) {
    bLength = *p;
    if ((i + bLength) <= config_desc->wTotalLength) {
      const uint8_t bDescriptorType = *(p + 1);
      switch (bDescriptorType) {
        case USB_B_DESCRIPTOR_TYPE_INTERFACE:
          if (!isMIDI) check_interface_desc_MIDI(p);
          break;
        case USB_B_DESCRIPTOR_TYPE_ENDPOINT:
          if (isMIDI && !isMIDIReady) {
            prepare_endpoints(p);
          }
          break;
        default:
          break;
      }
    }
    else {
      ESP_LOGE("", "Descriptor USB invalido!");
      return;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// Procesar mensaje MIDI

void parse_midi_message(const uint8_t* p) {
  if ((p[0] + p[1] + p[2] + p[3]) == 0) return;
  
  uint8_t cin = p[0] & 0x0F;
  uint8_t status = p[1];
  uint8_t channel = (status & 0x0F) + 1;
  uint8_t data1 = p[2];
  uint8_t data2 = p[3];

  switch (cin) {

    case 0x09: { ////////////////////////////////////////////////////////// Note On
      uint8_t note = data1;
      uint8_t velocity = data2;
      if (velocity > 0) {
        //Serial.printf("%3d, %3d, %2d\n", note, velocity, channel);  

        if (channel==1) {
          //Serial.printf("%3d, %3d, %2d\n", note, velocity, channel);
          if (note<8) {
       
          }

          if (note==91) { // play ("PLAY/PAUSE")
              if (playing){
                seq.stop();
                sstep=firstStep;
                recording=false;
                clearPADSTEP=true;
                pattern_song_counter=0; 
              } else {
                if (sync_state==2){ // if this machine is slave dont start playing now
                  pre_playing=true;
                } else {
                if (songing) pattern_song_counter=selected_pattern;
                  tick = 0;
                  seq.start();
                  sstep=firstStep;
                  refreshPADSTEP=true; 
                  Serial.println("go"); 
                }
              }
              playing=!playing; 
          } 
                 
          if (note==81) { // panic ("STOP ALL CLIPS")
            //MIDI.sendControlChange(ALL_NOTES_OFF, 127, 1);
            delay(1);
            //MIDI.sendControlChange(ALL_SOUND_OFF, 127, 1);
            delay(1);
            //MIDI.sendControlChange(RESET_ALL_CTRLS, 127, 1);
          }

          for (uint8_t f = 0; f < 5; f++) {
            if (note==mNN_pageRot[f]){
              //send_midi_led(old_cc_page_note, 0);
              pageRot=f;
              //send_midi_led(note, 1);
              old_cc_page_note=note;
              //Serial.println(note);
              if (pageRot<2) rPage=0;
              if (pageRot==2) rPage=1;
              if (pageRot>2) rPage=2;
              refreshMODES=true;
              refresh_rPage=true;

            }
          }
          
          // Sequencer 8 x 4 grid
          if (note>7 && note<40) {
            // uint8_t rnote=real_key[note-8];
            // if (pattern[rnote]!=0) {
            //   pattern[rnote]=0;
            //   // Turn oFF note LED
            //   send_midi_message(0x90, channel, note, 0);              
            // } else {
            //   // Turn on GREEN LED
            //   send_midi_message(0x90, channel, note, 1);
            //   pattern[rnote]=lastNotePlayed;
            // }
          }

        } else {
          //MIDI.sendNoteOn(note, 127, 1);
          //lastNotePlayed=note;
          synthESP32_TRIGGER_P(selected_sound,note);
          if (recording){
            bitWrite(pattern[selected_sound],sstep,1);
            melodic[selected_sound][sstep]=note;
          }
          //refreshPATTERN=true;
        }
      }
      break;
    }
    
    case 0x08: { ////////////////////////////////////////////////////////// Note Off
      uint8_t note = data1;
      uint8_t velocity = data2;
      //Serial.printf("Note OFF: Nota: %3d, Velocidad: %3d, Canal: %2d\n", note, velocity, channel);

      if (channel==2) {
        //MIDI.sendNoteOff(note, 0, 1);
      } else {
        if (note<8) {
          //send_midi_message(0x90, channel, note, 3); // Turn oN YELLOW LED  
        } 
      }
      break;
    }

    case 0x0B: { ////////////////////////////////////////////////////////// Control Change (CC)
      uint8_t controller = data1;
      uint8_t value = data2;

      // Buscar controlador en los rot que correspondan a la página actual.
      for (int f = 0; f < MAX_BARS; f++) {
        if (mRot[f]->pageRot==pageRot && mRot[f]->cc==controller){
          selected_rot=f;
          select_rot();
          old_counter1=counter1;
          counter1=mapRounded(value,0,127,min_values[selected_rot],max_values[selected_rot]);
          //Serial.printf("counter1: %3d",counter1);
          if (rPage!=mRot[f]->rPage){
              rPage=mRot[f]->rPage;
              refreshMODES=true;
              refresh_rPage=true;
          }
          do_rot();
          break;          
        }
      }

    }
    default: {
      Serial.printf("OTRO:     Paquete: %02x %02x %02x %02x\n", p[0], p[1], p[2], p[3]);
      break;
    }
  }
}
