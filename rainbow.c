#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <sched.h>
#include <errno.h>

/* Convert HSV values to RGB.
 * h: hue angle in degrees [0,360)
 * s: saturation [0,1]
 * v: value [0,1]
 * The resulting r, g, b values are in the range [0,255].
 */
void hsv_to_rgb(double h, double s, double v, int *r, int *g, int *b) {
    double C = v * s;  // Chroma
    double X = C * (1 - fabs(fmod(h / 60.0, 2) - 1));
    double m = v - C;
    double r_temp, g_temp, b_temp;

    if (h < 60) {
        r_temp = C; g_temp = X; b_temp = 0;
    } else if (h < 120) {
        r_temp = X; g_temp = C; b_temp = 0;
    } else if (h < 180) {
        r_temp = 0; g_temp = C; b_temp = X;
    } else if (h < 240) {
        r_temp = 0; g_temp = X; b_temp = C;
    } else if (h < 300) {
        r_temp = X; g_temp = 0; b_temp = C;
    } else { // h < 360
        r_temp = C; g_temp = 0; b_temp = X;
    }

    *r = (int)round((r_temp + m) * 255);
    *g = (int)round((g_temp + m) * 255);
    *b = (int)round((b_temp + m) * 255);
}

int main(void) {
    const char *led_path = "/sys/class/leds/mcu0/color";
    double hue = 0.0;
    const double hue_increment = 1.0;  // Increase hue by 1 degree each iteration.
    const int delay_us = 50000;        // Delay in microseconds (50ms) between updates.
    int r, g, b;

    /* --- Set Idle CPU Priority --- */
    struct sched_param sp = { .sched_priority = 0 };
    if (sched_setscheduler(0, SCHED_IDLE, &sp)) {
        perror("sched_setscheduler(SCHED_IDLE) failed");
        exit(EXIT_FAILURE);
    }

    /* Main loop: update LED color in a smooth cycle */
    while (1) {
        // Convert the current hue (with full saturation and value) to RGB.
        hsv_to_rgb(hue, 1.0, 1.0, &r, &g, &b);

        // Open the LED color file for writing.
        FILE *fp = fopen(led_path, "w");
        if (fp == NULL) {
            perror("Error opening LED device");
            exit(EXIT_FAILURE);
        }

        // Write the color in hex (e.g. "FF00FF").
        fprintf(fp, "%02X%02X%02X", r, g, b);
        fclose(fp);

        // Increment the hue and wrap around if needed.
        hue += hue_increment;
        if (hue >= 360.0)
            hue -= 360.0;

        // Sleep for a short period to make the transition smooth.
        usleep(delay_us);
    }

    return 0;
}
