/*
 * The MIT License
 *
 * Copyright 2024 Alvaro Salazar <alvaro@denkitronik.com>.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef LIBDISPLAY_H
#define LIBDISPLAY_H

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include <time.h>

#define SCREEN_WIDTH 128    ///< Ancho de la pantalla (en pixeles)
#define SCREEN_HEIGHT 64    ///< Alto de la pantalla (en pixeles)

extern Adafruit_SSD1306 display; ///< Pantalla OLED vinculada al dispositivo

void startDisplay();                    ///< Vincula la pantalla al dispositivo y asigna el color de texto blanco como predeterminado. 
void displayNoSignal();                 ///< Imprime en la pantalla un mensaje de "No hay señal".
void displayHeader(time_t now);         ///< Agrega a la pantalla el header con mensaje "IOT Sensors" y en seguida la hora actual
void displayMeasures(float temp, float humi);  ///< Agrega los valores medidos de temperatura y humedad a la pantalla
void displayMessage(String message);    ///< Agrega el mensaje indicado a la pantalla. Si el mensaje es OK, se busca mostrarlo centrado.
void displayConnecting(String ssid);    ///< Muestra en la pantalla el mensaje de "Connecting to:" y luego el nombre de la red a la que se conecta.
void displayConnected(String ssid);     ///< Muestra en la pantalla la red WiFi a la que ya quedó conectado el dispositivo.
void displayLoop(String message, time_t now, float temp, float humi); ///< Muestra en la pantalla el mensaje recibido, las medidas de temperatura y humedad.

#endif /* LIBDISPLAY_H */