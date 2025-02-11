#!/bin/sh

# Function to convert HSL hue to RGB
hsl_to_rgb() {
    HUE=$1
    awk -v HUE="$HUE" '
    function hue_to_rgb(p, q, t) {
        if (t < 0) t += 1
        if (t > 1) t -= 1
        if (t < 1/6) return p + (q - p) * 6 * t
        if (t < 1/2) return q
        if (t < 2/3) return p + (q - p) * (2/3 - t) * 6
        return p
    }
    
    BEGIN {
        h = HUE / 360
        s = 1
        l = 0.5
        q = l < 0.5 ? l * (1 + s) : l + s - l * s
        p = 2 * l - q
        
        r = hue_to_rgb(p, q, h + 1/3)
        g = hue_to_rgb(p, q, h)
        b = hue_to_rgb(p, q, h - 1/3)

        # Convert to 8-bit hex
        printf "%02X%02X%02X\n", int(r * 255), int(g * 255), int(b * 255)
    }'
}

HUE=0  # Start hue

while true; do
    COLOR=$(hsl_to_rgb "$HUE")
    echo "$COLOR" > /sys/class/leds/mcu0/color
    HUE=$(( (HUE + 1) % 360 ))  # Increment hue for smooth rotation
    #sleep 0.01  # Adjust speed for smoother transition
done
