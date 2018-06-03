/*
The MIT License (MIT)

Copyright (c) 2018 Maxim Kulkin

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#include "led_status_private.h"

typedef void *led_status_t;

led_status_t *led_status_init(int gpio);
void led_status_done(led_status_t * status);

void led_status_set(led_status_t * status, led_status_pattern_t * pattern);

// Steady blinking one sec on, one sec off
int waitingWifiDelays[] = { 1000, 1000 };

led_status_pattern_t waitingWifiPattern = {
	2,
	waitingWifiDelays,
};

// OTA indicator - three short 100ms-on-off blinks, 700ms off
int otaPatternDelays[] = { 100, 100, 100, 100, 100, 700 };

led_status_pattern_t otaUpdatePattern = {
	6,
	otaPatternDelays,
};

// HAP Identify pattern.
int identifyPatternDelays[] = { 100, 100, 200, 600 };

led_status_pattern_t identifyPattern = {
	4,
	identifyPatternDelays,
};

// Sensor error otaPatternDelays
int sensorErrorPatternDelays[] = { 100, 100 };

led_status_pattern_t sensorErrorPattern = {
	2,
	sensorErrorPatternDelays,
};
