/*
 * Este programa pretende simular el proceso de decision de un modelo de Machine Learning.
 * Para ello ocupamos 3 fases que son muy importantes en la generacion de un modelo de ML:
 *  - Adquisicion de los datos provenientes del IMU del Arduino nano33 BLE Sense
 *  - Preprocesamiento de los datos (Suavizar la señal y obtener las caracteristicas estadisticas)
 *  - Implementacion del arbol de decision en una estructura if-else
 *  - Manejo del LED RGB segun la actividad que se esta realizando
 * 
 * Las caracteristicas obtenidas en el programa se obtuvieron de acuerdo a la decision que toma el
 * arbol de decision, en el modelo entrenado en Matlab se asignan 14 caracteristicas estadisticas, pero
 * no todas son tomadas en cuenta.
 * 
 * El tipo de arbol de decision que implementamos es de tipo Medium Tree, debido a que el tipo Fine Tree 
 * es demasido extenso y no encontre como convertirlo a codigo. El arbol Coarse Tree tenia un 72.9% de
 * resultados positivos entonces no es conveniente implementarlo. El Medium Tree tiene una precision de
 * 82.6% por lo cual no es del todo exacto pero es lo suficientemente preciso.
 */

//Biblioteca para usar el IMU del Arduino nano33 BLE Sense
#include <Arduino_LSM9DS1.h>

// Vectores de almacenamiento temporal de la señal
float rAx[10];
float rAy[10];
float rAz[10];

float rGx[10];
float rGy[10];
float rGz[10];

// Contadores
int contador = 0;
int n = 0;

// Variable de información de la suma de las señales
float p_AxSum = 0;
float p_AySum = 0;
float p_AzSum = 0;

float p_GxSum = 0;
float p_GySum = 0;
float p_GzSum = 0;

// Variables finales de la señal preprocesada
float f_Ax = 0;
float f_Ay = 0;
float f_Az = 0;

float f_Gx = 0;
float f_Gy = 0;
float f_Gz = 0;

//LED RGB
const int LED_BUILTIN_RED = LEDR;
const int LED_BUILTIN_GREEN = LEDG;
const int LED_BUILTIN_BLUE = LEDB;


void setup() {
  // Inicialización del IMU
  Serial.begin(9600);
  while (!Serial)
    ;

  if (!IMU.begin()) {
    Serial.println("Fallo al iniciar el IMU");
    while (1)
      ;
  }

  //Configuramos el LED RGB como salida
  pinMode(LED_BUILTIN_RED, OUTPUT);
  pinMode(LED_BUILTIN_GREEN, OUTPUT);
  pinMode(LED_BUILTIN_BLUE, OUTPUT);
}

void loop() {
  // Lectura de los valores del IMU
  float ax, ay, az, gx, gy, gz;
  if (IMU.accelerationAvailable() && IMU.gyroscopeAvailable()) {
    IMU.readAcceleration(ax, ay, az);
    IMU.readGyroscope(gx, gy, gz);

    // Almacenamiento de los datos en los vectores
    if (contador < 10) {
      rAx[contador] = ax;
      rAy[contador] = ay;
      rAz[contador] = az;
      rGx[contador] = gx;
      rGy[contador] = gy;
      rGz[contador] = gz;
      contador++;
    } else {
      contador = 0;
    }

    // Reinicio de las sumas
    p_AxSum = 0;
    p_AySum = 0;
    p_AzSum = 0;
    p_GxSum = 0;
    p_GySum = 0;
    p_GzSum = 0;

    // Suma de los valores en los vectores
    for (n = 0; n < 10; n++) {
      p_AxSum += rAx[n];
      p_AySum += rAy[n];
      p_AzSum += rAz[n];
      p_GxSum += rGx[n];
      p_GySum += rGy[n];
      p_GzSum += rGz[n];
    }

    // Cálculo de promedios
    f_Ax = p_AxSum / 10;
    f_Ay = p_AySum / 10;
    f_Az = p_AzSum / 10;
    f_Gx = p_GxSum / 10;
    f_Gy = p_GySum / 10;
    f_Gz = p_GzSum / 10;

//    Serial.print(ax);
//    Serial.print('\t');
//    Serial.println(f_Ax);

    // Obtencion de las características estadísticas
    float max_Ax = obtenerMaximo(rAx, 10);
    float min_Ax = obtenerMinimo(rAx, 10);
    float max_Az = obtenerMaximo(rAz, 10);
    float min_Az = obtenerMinimo(rAz, 10);
    float min_Ay = obtenerMinimo(rAy, 10);
    float median_Gx = obtenerMediana(rGx, 10);
    float median_Gy = obtenerMediana(rGy, 10);
    float rms_Ax = obtenerRMS(rAx, 10);
    float rms_Ay = obtenerRMS(rAy, 10);
    float rms_Gx = obtenerRMS(rGx, 10);

    // Determinar la actividad usando el metodo conjunto
    String actividad = obtenerActividad(max_Ax, min_Ax, max_Az, min_Az, min_Ay, median_Gy, median_Gx, rms_Ax, rms_Ay, rms_Gx);

    // Control del LED según la actividad que se esta realizando
    controlarLED(actividad);
  }
}

// Función para obtener el máximo 
float obtenerMaximo(float arr[], int size) {
  float maxVal = arr[0];
  for (int i = 1; i < size; i++) {
    if (arr[i] > maxVal) {
      maxVal = arr[i];
    }
  }
  return maxVal;
}

// Función para obtener el mínimo 
float obtenerMinimo(float arr[], int size) {
  float minVal = arr[0];
  for (int i = 1; i < size; i++) {
    if (arr[i] < minVal) {
      minVal = arr[i];
    }
  }
  return minVal;
}

// Función para obtener la mediana
float obtenerMediana(float arr[], int size) {
  float tempArr[size];
  for (int i = 0; i < size; i++) {
    tempArr[i] = arr[i];
  }
  // Ordenar el arreglo en orden ascendente
  for (int i = 0; i < size - 1; i++) {
    for (int j = 0; j < size - i - 1; j++) {
      if (tempArr[j] > tempArr[j + 1]) {
        float temp = tempArr[j];
        tempArr[j] = tempArr[j + 1];
        tempArr[j + 1] = temp;
      }
    }
  }
  // Obtener la mediana
  if (size % 2 == 0) {
    return (tempArr[size / 2 - 1] + tempArr[size / 2]) / 2;
  } else {
    return tempArr[size / 2];
  }
}

// Función para obtener el valor RMS
float obtenerRMS(float arr[], int size) {
  float sum = 0;
  for (int i = 0; i < size; i++) {
    sum += arr[i] * arr[i];
  }
  float meanSquare = sum / size;
  return sqrt(meanSquare);
}

// Función para obtener la actividad según el arbol de decision
String obtenerActividad(float max_Ax, float min_Ax, float max_Az, float min_Az, float min_Ay, float median_Gx, float median_Gy, float rms_Ax, float rms_Ay, float rms_Gx) {
  if (max_Ax < -0.642727) {
    if (f_Ay < -0.132967) {
      return "acostado";
    } else {
      return "abdominales";
    }
  } else {
    if (rms_Gx < 2.60253) {
      if (rms_Ay < 0.411199) {
        if (f_Ax < -0.251121) {
          return "acostado";
        } else {
          return "sentado";
        }
      } else {
        if (median_Gy < 3.26798) {
          if (f_Ay < 0.55239) {
            return "abdominales";
          } else {
            return "trotanto";
          }
        } else {
          if (f_Ax < 0.570841) {
            return "bote_balon";
          } else {
            return "Tiro_canasta";
          }
        }
      }
    } else {
      if (max_Ax < 0.986111) {
        if (min_Ay < -0.325606) {
          if (min_Ax < 0.462273) {
            if (min_Az < -0.0262121) {
              return "acostado";
            } else {
              return "Trote";
            }
          } else {
            if (min_Az < 0.0906566) {
              return "Trote";
            } else {
              return "bote_balon";
            }
          }
        } else {
          if (min_Ax < 0.147172) {
            return "Tiro_canasta";
          } else {
            if (max_Az < 0.308434) {
              return "caminando";
            } else {
              return "bote_balon";
            }
          }
        }
      } else {
        if (median_Gx < 15.7931) {
          if (rms_Ax < 0.951489) {
            return "bote_balon";
          } else {
            return "salto_payaso";
          }
        } else {
          return "trote";
        }
      }
    }
  }
}

// Función para controlar el LED según la actividad
void controlarLED(String actividad) {
  
  if (actividad == "trote") {
    // Encender el LED en morado
    analogWrite(LED_BUILTIN_RED, 255);
    analogWrite(LED_BUILTIN_GREEN, 0);
    analogWrite(LED_BUILTIN_BLUE, 255);
    Serial.println("trote");
  } else if (actividad == "caminando") {
    // Encender el LED en rosa
    analogWrite(LED_BUILTIN_RED, 255);
    analogWrite(LED_BUILTIN_GREEN, 192);
    analogWrite(LED_BUILTIN_BLUE, 203);
    Serial.println("caminando");
  } else if (actividad == "bote_balon") {
    // Encender el LED en naranja
    analogWrite(LED_BUILTIN_RED, 255);
    analogWrite(LED_BUILTIN_GREEN, 165);
    analogWrite(LED_BUILTIN_BLUE, 0);
    Serial.println("botando");
  } else if (actividad == "salto_payaso") {
    // Encender el LED en azul claro
    analogWrite(LED_BUILTIN_RED, 135);
    analogWrite(LED_BUILTIN_GREEN, 206);
    analogWrite(LED_BUILTIN_BLUE, 250);
    Serial.println("saltando de payaso");
  } else if (actividad == "acostado") {
    // Encender el LED en verde
    analogWrite(LED_BUILTIN_RED, 0);
    analogWrite(LED_BUILTIN_GREEN, 255);
    analogWrite(LED_BUILTIN_BLUE, 0);
    Serial.println("acostado");
  } else if (actividad == "sentado") {
    // Encender el LED en azul marino
    analogWrite(LED_BUILTIN_RED, 0);
    analogWrite(LED_BUILTIN_GREEN, 0);
    analogWrite(LED_BUILTIN_BLUE, 128);
    Serial.println("sentado");
  } else if (actividad == "abdominales") {
    // Encender el LED en rojo
    analogWrite(LED_BUILTIN_RED, 255);
    analogWrite(LED_BUILTIN_GREEN, 0);
    analogWrite(LED_BUILTIN_BLUE, 0);
    Serial.println("abdominales");
  } else if (actividad == "Tiro_canasta") {
    // Encender el LED en café
    analogWrite(LED_BUILTIN_RED, 165);
    analogWrite(LED_BUILTIN_GREEN, 42);
    analogWrite(LED_BUILTIN_BLUE, 42);
    Serial.println("tiro a canasta");
  } else {
    // Apagar el LED
    analogWrite(LED_BUILTIN_RED, 0);
    analogWrite(LED_BUILTIN_GREEN, 0);
    analogWrite(LED_BUILTIN_BLUE, 0);
    Serial.println("NR");
  }
}
