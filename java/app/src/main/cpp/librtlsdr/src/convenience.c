/*
 * Convenience functions for RTL-SDR
 */

#include "rtl-sdr.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int verbose_device_search(char *s) {
    int device_count;
    int dev_index = 0;
    char vendor[256], product[256], serial[256];

    device_count = rtlsdr_get_device_count();
    if (!device_count) {
        fprintf(stderr, "No supported devices found.\n");
        return -1;
    }

    fprintf(stderr, "Found %d device(s):\n", device_count);
    for (int i = 0; i < device_count; i++) {
        rtlsdr_get_device_usb_strings(i, vendor, product, serial);
        fprintf(stderr, "  %d:  %s, %s, SN: %s\n", i, vendor, product, serial);
    }
    fprintf(stderr, "\n");

    /* does string look like raw id number */
    if (s && strlen(s) > 0) {
        dev_index = (int)strtol(s, NULL, 0);
    }

    if (dev_index >= device_count) {
        dev_index = 0;
    }

    return dev_index;
}

int verbose_set_frequency(rtlsdr_dev_t *dev, uint32_t frequency) {
    int r;
    r = rtlsdr_set_center_freq(dev, frequency);
    if (r < 0) {
        fprintf(stderr, "WARNING: Failed to set center freq.\n");
    } else {
        fprintf(stderr, "Tuned to %u Hz.\n", frequency);
    }
    return r;
}

int verbose_set_sample_rate(rtlsdr_dev_t *dev, uint32_t samp_rate) {
    int r;
    r = rtlsdr_set_sample_rate(dev, samp_rate);
    if (r < 0) {
        fprintf(stderr, "WARNING: Failed to set sample rate.\n");
    } else {
        fprintf(stderr, "Sampling at %u S/s.\n", samp_rate);
    }
    return r;
}

int verbose_direct_sampling(rtlsdr_dev_t *dev, int on) {
    int r;
    r = rtlsdr_set_direct_sampling(dev, on);
    if (r != 0) {
        fprintf(stderr, "WARNING: Failed to set direct sampling mode.\n");
        return r;
    }
    if (on == 0) {
        fprintf(stderr, "Direct sampling mode disabled.\n");
    }
    if (on == 1) {
        fprintf(stderr, "Enabled direct sampling mode, input 1/I.\n");
    }
    if (on == 2) {
        fprintf(stderr, "Enabled direct sampling mode, input 2/Q.\n");
    }
    return r;
}

int verbose_offset_tuning(rtlsdr_dev_t *dev) {
    int r;
    r = rtlsdr_set_offset_tuning(dev, 1);
    if (r != 0) {
        fprintf(stderr, "WARNING: Failed to set offset tuning.\n");
    } else {
        fprintf(stderr, "Offset tuning mode enabled.\n");
    }
    return r;
}

int verbose_auto_gain(rtlsdr_dev_t *dev) {
    int r;
    r = rtlsdr_set_tuner_gain_mode(dev, 0);
    if (r != 0) {
        fprintf(stderr, "WARNING: Failed to set tuner gain.\n");
    } else {
        fprintf(stderr, "Tuner gain set to automatic.\n");
    }
    return r;
}

int verbose_gain_set(rtlsdr_dev_t *dev, int gain) {
    int r;
    r = rtlsdr_set_tuner_gain_mode(dev, 1);
    if (r < 0) {
        fprintf(stderr, "WARNING: Failed to enable manual gain.\n");
        return r;
    }
    r = rtlsdr_set_tuner_gain(dev, gain);
    if (r != 0) {
        fprintf(stderr, "WARNING: Failed to set tuner gain.\n");
    } else {
        fprintf(stderr, "Tuner gain set to %f dB.\n", gain/10.0);
    }
    return r;
}

int verbose_ppm_set(rtlsdr_dev_t *dev, int ppm_error) {
    int r;
    if (ppm_error == 0) {
        return 0;
    }
    r = rtlsdr_set_freq_correction(dev, ppm_error);
    if (r < 0) {
        fprintf(stderr, "WARNING: Failed to set ppm error.\n");
    } else {
        fprintf(stderr, "Tuner error set to %i ppm.\n", ppm_error);
    }
    return r;
}

int verbose_reset_buffer(rtlsdr_dev_t *dev) {
    int r;
    r = rtlsdr_reset_buffer(dev);
    if (r < 0) {
        fprintf(stderr, "WARNING: Failed to reset buffers.\n");
    }
    return r;
}

int verbose_device_close(rtlsdr_dev_t *dev) {
    int r;
    r = rtlsdr_close(dev);
    if (r < 0) {
        fprintf(stderr, "WARNING: Failed to close device.\n");
    }
    return r;
}
