/*
 * Stub implementation for R82xx tuner
 */

#include <stdint.h>
#include <android/log.h>

#define LOG_TAG "R82xx_Tuner"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

// Stub functions for R82xx tuner
int r82xx_init(void *dev) {
    (void)dev;
    LOGI("R82xx tuner initialized (stub)");
    return 0;
}

int r82xx_standby(void *dev) {
    (void)dev;
    return 0;
}

int r82xx_set_freq(void *dev, uint32_t freq) {
    (void)dev;
    (void)freq;
    return 0;
}

int r82xx_set_gain(void *dev, int gain) {
    (void)dev;
    (void)gain;
    return 0;
}

int r82xx_set_bandwidth(void *dev, uint32_t bandwidth) {
    (void)dev;
    (void)bandwidth;
    return 0;
}
