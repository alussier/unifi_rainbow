#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <math.h>
#include <sched.h>
#include <errno.h>

#define TAU (2 * M_PI)

// adapted from https://github.com/FastLED/FastLED/blob/master/src/hsv2rgb.cpp
void hue_to_rgb(uint8_t hue, uint8_t *p_r, uint8_t *p_g, uint8_t *p_b) {
    // Yellow has a higher inherent brightness than
    // any other color; 'pure' yellow is perceived to
    // be 93% as bright as white.  In order to make
    // yellow appear the correct relative brightness,
    // it has to be rendered brighter than all other
    // colors.

    uint8_t offset = hue & 0x1F; // 0..31

    // offset8 = offset * 8
    uint8_t offset8 = offset << 3;

    uint8_t third = offset8 / 3; // max = 85

    uint8_t r, g, b;

    if( ! (hue & 0x80) ) {
        // 0XX
        if( ! (hue & 0x40) ) {
            // 00X
            //section 0-1
            if( ! (hue & 0x20) ) {
                // 000
                //case 0: // R -> O
                r = 255 - third;
                g = third;
                b = 0;
            } else {
                // 001
                //case 1: // O -> Y
                r = 171;
                g = 85 + third ;
                b = 0;
            }
        } else {
            //01X
            // section 2-3
            if( !  (hue & 0x20) ) {
                // 010
                //case 2: // Y -> G
                uint8_t twothirds = (third << 1);
                r = 171 - twothirds;
                g = 170 + third;
                b = 0;
            } else {
                // 011
                // case 3: // G -> A
                r = 0;
                g = 255 - third;
                b = third;
            }
        }
    } else {
        // section 4-7
        // 1XX
        if( ! (hue & 0x40) ) {
            // 10X
            if( ! ( hue & 0x20) ) {
                // 100
                //case 4: // A -> B
                r = 0;
                uint8_t twothirds = (third << 1);
                g = 171 - twothirds; //K170?
                b = 85  + twothirds;
            } else {
                // 101
                //case 5: // B -> P
                r = third;
                g = 0;
                b = 255 - third;
            }
        } else {
            if( !  (hue & 0x20)  ) {
                // 110
                //case 6: // P -- K
                r = 85 + third;
                g = 0;
                b = 171 - third;
            } else {
                // 111
                //case 7: // K -> R
                r = 170 + third;
                g = 0;
                b = 85 - third;
            }
        }
    }

    *p_r = r;
    *p_g = g;
    *p_b = b;
}

int main(void) {
    const char *led_path = "/sys/class/leds/mcu0/color";
    const uint8_t hue_increment = 1;
    const useconds_t delay_us = 50000;        // Delay in microseconds (50ms) between updates.
    uint8_t r, g, b;

    /* --- Set Idle CPU Priority --- */
    struct sched_param sp = { .sched_priority = 0 };
    if (sched_setscheduler(0, SCHED_IDLE, &sp)) {
        perror("sched_setscheduler(SCHED_IDLE) failed");
        exit(EXIT_FAILURE);
    }

    /* hue from 0 to 255 */
    for (uint8_t hue = 0; ; hue = (hue + hue_increment) % 256) {
        hue_to_rgb(hue, &r, &g, &b);

        // Open the LED color file for writing.
        FILE *fp = fopen(led_path, "w");
        if (fp == NULL) {
            perror("Error opening LED device");
            exit(EXIT_FAILURE);
        }

        // Write the color in hex (e.g. "FF00FF").
        fprintf(fp, "%02X%02X%02X", r, g, b);
        fclose(fp);

        // Sleep for a short period to make the transition smooth.
        usleep(delay_us);
    }

    return 0;
}
