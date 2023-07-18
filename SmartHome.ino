#include <SimpleDHT.h>

//pinout arduino
#define dht11_pin 13

#define photo_pin A0
#define photo_led_pin 12

#define fan_enable_pin 3
#define fan_in1_pin 4
#define fan_in2_pin 5
#define fan_led_pin 11

#define button_water_pump_pin 2
#define water_level_pin A1
#define water_level_enable_pin 6
#define water_pump_enable_pin 7

//variabili di sato sensori
typedef enum {
  SPENTO,
  ACCESO
} led_state;
led_state fan_led = SPENTO;
led_state photo_led = SPENTO;

typedef enum {
  SPENTA,
  ACCESA,
  MAL_FUNZIONAMENTO,
} water_pump_state;
water_pump_state water_pump = SPENTA;

//oggetti utilizzati
SimpleDHT11 dht11;

//variabili numeriche
int temperature;
int set_point_temperature = 20;
long start_pump = 0;
int period_on_pump = 10;

//variabili uart
//numero di byte scambiati per pacchetto
#define num_byte 7

//char inviati
#define sof 'a'
#define eof 'r'
#define padding 's'

#define get_info 'b'
#define send_value 'c'

#define fontana 'f'
#define luce_esterna 'h'
#define temperatura 'k'
#define configurazione_fontana 'm'
#define configurazione_temperatura 'p'
#define ventola 'q'

typedef enum {
  IN_ATTESA_SOF,
  IN_ATTESA_OPERAZIONE,
  IN_ATTESA_IDENTIFICATIVO,
  IN_ATTESA_VALORE_1,
  IN_ATTESA_VALORE_2,
  IN_ATTESA_VALORE_3,
  IN_ATTESA_EOF
} uart_state;
uart_state uart = IN_ATTESA_SOF;

char send_byte[num_byte];
char receive_byte[num_byte];
int index_byte = 0;

void setup() {
  Serial.begin(9600);

  pinMode(photo_led_pin, OUTPUT);

  pinMode(fan_enable_pin, OUTPUT);
  pinMode(fan_in1_pin, OUTPUT);
  pinMode(fan_in2_pin, OUTPUT);

  pinMode(water_level_enable_pin, OUTPUT);
  pinMode(water_pump_enable_pin, OUTPUT);
}

void loop() {
  rileva_sensori();
  check_uart();
}

void rileva_sensori() {
  rileva_temperatura();
  delay(1500);
  rileva_fotoresistenza();
  rileva_acqua();
  check_timer();
}

void rileva_temperatura() {
  byte u = 0;
  byte t = 0;
  int err = SimpleDHTErrSuccess;
  err = dht11.read(dht11_pin, &t, &u, NULL);
  if (err != SimpleDHTErrSuccess) {
  } else {
    temperature = (int)t;
  }

  if (temperature > set_point_temperature) {
    if (fan_led == SPENTO) {
      fan_led = ACCESO;
      prepare_data_temperature(send_value);
      prepare_data_fan(send_value);
      analogWrite(fan_enable_pin, 255);
      digitalWrite(fan_in1_pin, HIGH);
      digitalWrite(fan_in2_pin, LOW);
      digitalWrite(fan_led_pin, HIGH);
    }
  } else {
    if (fan_led == ACCESO) {
      prepare_data_temperature(send_value);
      prepare_data_fan(send_value);
      analogWrite(fan_enable_pin, 0);
      digitalWrite(fan_in1_pin, LOW);
      digitalWrite(fan_in2_pin, LOW);
      fan_led = SPENTO;
      digitalWrite(fan_led_pin, LOW);
    }
  }
}

void rileva_fotoresistenza() {
  int light = analogRead(photo_pin);
  if (light < 512) {
    if (photo_led == SPENTO) {
      photo_led = ACCESO;
      prepare_data_external_light(send_value);
      digitalWrite(photo_led_pin, HIGH);
    }
  } else {
    if (photo_led == ACCESO) {
      photo_led = SPENTO;
      prepare_data_external_light(send_value);
      digitalWrite(photo_led_pin, LOW);
    }
  }
}

void rileva_acqua() {
  if (digitalRead(button_water_pump_pin) == HIGH && (water_pump == SPENTA || water_pump == MAL_FUNZIONAMENTO)) {
    digitalWrite(water_level_enable_pin, HIGH);
    int level = analogRead(water_level_pin);
    delay(250);
    level = analogRead(water_level_pin);
    delay(250);
    level = analogRead(water_level_pin);
    digitalWrite(water_level_enable_pin, LOW);
    if (level > 250) {
      water_pump = ACCESA;
      prepare_data_water_pump(send_value);
      digitalWrite(water_pump_enable_pin, HIGH);
      start_pump = millis();
    } else {
      water_pump = MAL_FUNZIONAMENTO;
      prepare_data_water_pump(send_value);
    }
  }
}

void check_timer() {
  if (water_pump == ACCESA) {
    if (millis() - start_pump >= (period_on_pump * 1000)) {
      water_pump = SPENTA;
      prepare_data_water_pump(send_value);
      digitalWrite(water_pump_enable_pin, LOW);
    }
  }
}

void prepare_data_water_pump(char operazione) {
  send_byte[0] = sof;
  send_byte[1] = operazione;
  send_byte[2] = fontana;
  if (water_pump == SPENTA) {
    send_byte[3] = '0';
  } else {
    if (water_pump == ACCESA) {
      send_byte[3] = '1';
    } else {
      if (water_pump == MAL_FUNZIONAMENTO) {
        send_byte[3] = '2';
      }
    }
  }
  send_byte[4] = padding;
  send_byte[5] = padding;
  send_byte[6] = eof;

  send_data();
}

void prepare_data_external_light(char operazione) {
  send_byte[0] = sof;
  send_byte[1] = operazione;
  send_byte[2] = luce_esterna;
  if (photo_led == SPENTO) {
    send_byte[3] = '0';
  } else {
    if (photo_led == ACCESO) {
      send_byte[3] = '1';
    }
  }
  send_byte[4] = padding;
  send_byte[5] = padding;
  send_byte[6] = eof;

  send_data();
}

void prepare_data_temperature(char operazione) {
  send_byte[0] = sof;
  send_byte[1] = operazione;
  send_byte[2] = temperatura;

  int power = 10;
  while (abs(temperature) >= power) {
    power *= 10;
  }
  power /= 10;

  if (temperature < 0) {
    send_byte[3] = '-';
    temperature *= -1;
    if (power == 1) {
      //1 cifra
      send_byte[4] = temperature + '0';
      send_byte[5] = padding;
    } else {
      if (power == 10) {
        //2 cifre
        int cifra = temperature / 10;
        send_byte[4] = cifra + '0';
        temperature -= cifra * 10;
        send_byte[5] = temperature + '0';
      }
    }

  } else {
    if (power == 1) {
      //1 cifra
      send_byte[3] = temperature + '0';
      send_byte[4] = padding;
      send_byte[5] = padding;
    }
    if (power == 10) {
      //2 cifre
      int cifra = temperature / 10;
      send_byte[3] = cifra + '0';
      temperature -= cifra * 10;
      send_byte[4] = temperature + '0';
      send_byte[5] = padding;
    } else {
      if (power == 100) {
        int cifra = temperature / 100;
        send_byte[3] = cifra + '0';
        temperature -= cifra * 100;
        cifra = temperature / 10;
        send_byte[4] = cifra + '0';
        temperature -= cifra * 10;
        send_byte[5] = temperature + '0';
      }
    }
  }
  send_byte[6] = eof;

  send_data();
}

void prepare_data_configuration_water_pump(char operazione) {
  send_byte[0] = sof;
  send_byte[1] = operazione;
  send_byte[2] = configurazione_fontana;
  send_byte[3] = (period_on_pump / 10) + '0';
  send_byte[4] = (period_on_pump - ((period_on_pump / 10) * 10)) + '0';
  send_byte[5] = padding;
  send_byte[6] = eof;

  send_data();
}

void prepare_data_configurazion_temperature(char operazione) {
  send_byte[0] = sof;
  send_byte[1] = operazione;
  send_byte[2] = configurazione_temperatura;
  send_byte[3] = (set_point_temperature / 10) + '0';
  send_byte[4] = (set_point_temperature - ((set_point_temperature / 10) * 10)) + '0';
  send_byte[5] = padding;
  send_byte[6] = eof;

  send_data();
}

void prepare_data_fan(char operazione) {
  send_byte[0] = sof;
  send_byte[1] = operazione;
  send_byte[2] = ventola;
  if (fan_led == SPENTO) {
    send_byte[3] = '0';
  } else {
    if (fan_led == ACCESO) {
      send_byte[3] = '1';
    }
  }
  send_byte[4] = padding;
  send_byte[5] = padding;
  send_byte[6] = eof;

  send_data();
}

void send_data() {
  for (int i = 0; i < num_byte; i++) {
    Serial.print(send_byte[i]);
  }
}

void check_uart() {
  while (Serial.available() > 0) {
    char data = (char)Serial.read();
    if (process_data(data)) {
      receive_byte[index_byte] = data;
      index_byte++;
      if (index_byte == num_byte) {
        index_byte = 0;
        prepare_data_water_pump(get_info);
        prepare_data_external_light(get_info);
        prepare_data_temperature(get_info);
        prepare_data_configuration_water_pump(get_info);
        prepare_data_configurazion_temperature(get_info);
        prepare_data_fan(get_info);
      }
    } else {
      uart = IN_ATTESA_SOF;
      index_byte = 0;
    }
  }
}

int process_data(char c) {
  int ok = false;
  switch (uart) {
    case IN_ATTESA_SOF:
      if (c == sof) {
        uart = IN_ATTESA_OPERAZIONE;
        ok = true;
      }
      break;
    case IN_ATTESA_OPERAZIONE:
      if (c == get_info) {
        uart = IN_ATTESA_IDENTIFICATIVO;
        ok = true;
      }
      break;
    case IN_ATTESA_IDENTIFICATIVO:
      if (c == padding) {
        ok = true;
        uart = IN_ATTESA_VALORE_1;
      }
      break;
    case IN_ATTESA_VALORE_1:
      if (c == padding) {
        ok = true;
        uart = IN_ATTESA_VALORE_2;
      }
      break;
    case IN_ATTESA_VALORE_2:
      if (c == padding) {
        ok = true;
        uart = IN_ATTESA_VALORE_3;
      }
      break;
    case IN_ATTESA_VALORE_3:
      if (c == padding) {
        ok = true;
        uart = IN_ATTESA_EOF;
      }
      break;
    case IN_ATTESA_EOF:
      if (c == eof) {
        uart = IN_ATTESA_SOF;
        ok = true;
      }
      break;
  }
  return ok;
}
