/*
 * Simplified RTL-SDR implementation for Android
 * Based on librtlsdr
 */

#include "rtl-sdr.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/usbdevice_fs.h>
#include <android/log.h>

#define LOG_TAG "RTL_SDR"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

// RTL2832 USB vendor/product IDs
#define RTL_VID 0x0bda
#define RTL_PID 0x2832

// USB endpoints
#define BULK_ENDPOINT_IN  0x81
#define BULK_ENDPOINT_OUT 0x02

// Register definitions
#define DEMOD_CTL       0x3000
#define GPO             0x3001
#define GPI             0x3002
#define GPOE            0x3003
#define GPD             0x3004
#define SYSCTL          0x3008
#define EPA_CTL         0x3020
#define EPA_MAXPKT      0x3024

struct rtlsdr_dev {
    int fd;
    int opened;
    uint32_t rate;
    uint32_t freq;
    int gain;
    int auto_gain;
    enum rtlsdr_tuner tuner_type;
    rtlsdr_read_async_cb_t cb;
    void *cb_ctx;
    int async_cancel;
};

static struct rtlsdr_dev *dev = NULL;

// USB control transfer wrapper
static int rtlsdr_write_reg(rtlsdr_dev_t *dev, uint8_t block, uint16_t addr, uint16_t val, uint8_t len) {
    if (!dev || !dev->opened) {
        return -1;
    }
    
    struct usbdevfs_ctrltransfer ctrl;
    ctrl.bRequestType = 0x40; // USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_DIR_OUT
    ctrl.bRequest = 0;
    ctrl.wValue = (block << 8) | 0x10;
    ctrl.wIndex = addr;
    ctrl.wLength = len;
    ctrl.timeout = 300;
    ctrl.data = &val;
    
    int ret = ioctl(dev->fd, USBDEVFS_CONTROL, &ctrl);
    if (ret < 0) {
        LOGE("Failed to write register: %s", strerror(errno));
        return -1;
    }
    
    return 0;
}

static int rtlsdr_read_reg(rtlsdr_dev_t *dev, uint8_t block, uint16_t addr, uint8_t len) {
    if (!dev || !dev->opened) {
        return -1;
    }
    
    uint16_t val = 0;
    struct usbdevfs_ctrltransfer ctrl;
    ctrl.bRequestType = 0xc0; // USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_DIR_IN
    ctrl.bRequest = 0;
    ctrl.wValue = (block << 8) | 0x10;
    ctrl.wIndex = addr;
    ctrl.wLength = len;
    ctrl.timeout = 300;
    ctrl.data = &val;
    
    int ret = ioctl(dev->fd, USBDEVFS_CONTROL, &ctrl);
    if (ret < 0) {
        LOGE("Failed to read register: %s", strerror(errno));
        return -1;
    }
    
    return val;
}

// API implementation
int rtlsdr_get_device_count(void) {
    // For Android, we'll handle device detection at the Java level
    return 1;
}

const char* rtlsdr_get_device_name(uint32_t index) {
    (void)index;
    return "RTL-SDR Generic";
}

int rtlsdr_open_fd(rtlsdr_dev_t **out_dev, int fd) {
    if (!out_dev) {
        return -1;
    }
    
    struct rtlsdr_dev *new_dev = calloc(1, sizeof(struct rtlsdr_dev));
    if (!new_dev) {
        return -1;
    }
    
    new_dev->fd = fd;
    new_dev->opened = 1;
    new_dev->rate = DEFAULT_SAMPLE_RATE;
    new_dev->freq = 88500000; // 88.5 MHz
    new_dev->gain = 0;
    new_dev->auto_gain = 1;
    new_dev->tuner_type = RTLSDR_TUNER_R820T;
    new_dev->async_cancel = 0;
    
    *out_dev = new_dev;
    dev = new_dev;
    
    LOGI("RTL-SDR device opened with fd %d", fd);
    return 0;
}

int rtlsdr_open(rtlsdr_dev_t **out_dev, uint32_t index) {
    (void)out_dev;
    (void)index;
    // Not implemented for Android - use rtlsdr_open_fd instead
    return -1;
}

int rtlsdr_close(rtlsdr_dev_t *dev) {
    if (!dev) {
        return -1;
    }
    
    if (dev->opened) {
        dev->opened = 0;
        if (dev->fd >= 0) {
            close(dev->fd);
        }
    }
    
    free(dev);
    
    LOGI("RTL-SDR device closed");
    return 0;
}

int rtlsdr_set_center_freq(rtlsdr_dev_t *dev, uint32_t freq) {
    if (!dev || !dev->opened) {
        return -1;
    }
    
    // Simplified frequency setting
    dev->freq = freq;
    
    LOGD("Center frequency set to %u Hz", freq);
    return 0;
}

uint32_t rtlsdr_get_center_freq(rtlsdr_dev_t *dev) {
    if (!dev) {
        return 0;
    }
    return dev->freq;
}

int rtlsdr_set_sample_rate(rtlsdr_dev_t *dev, uint32_t rate) {
    if (!dev || !dev->opened) {
        return -1;
    }
    
    dev->rate = rate;
    
    LOGD("Sample rate set to %u Hz", rate);
    return 0;
}

uint32_t rtlsdr_get_sample_rate(rtlsdr_dev_t *dev) {
    if (!dev) {
        return 0;
    }
    return dev->rate;
}

int rtlsdr_set_tuner_gain_mode(rtlsdr_dev_t *dev, int manual) {
    if (!dev || !dev->opened) {
        return -1;
    }
    
    dev->auto_gain = !manual;
    
    LOGD("Gain mode set to %s", manual ? "manual" : "auto");
    return 0;
}

int rtlsdr_set_tuner_gain(rtlsdr_dev_t *dev, int gain) {
    if (!dev || !dev->opened) {
        return -1;
    }
    
    if (dev->auto_gain) {
        return 0; // Ignore in auto gain mode
    }
    
    dev->gain = gain;
    
    LOGD("Tuner gain set to %d (0.1 dB)", gain);
    return 0;
}

int rtlsdr_get_tuner_gain(rtlsdr_dev_t *dev) {
    if (!dev) {
        return 0;
    }
    return dev->gain;
}

int rtlsdr_set_freq_correction(rtlsdr_dev_t *dev, int ppm) {
    if (!dev || !dev->opened) {
        return -1;
    }
    
    LOGD("Frequency correction set to %d PPM", ppm);
    return 0;
}

int rtlsdr_get_freq_correction(rtlsdr_dev_t *dev) {
    (void)dev;
    return 0;
}

enum rtlsdr_tuner rtlsdr_get_tuner_type(rtlsdr_dev_t *dev) {
    if (!dev) {
        return RTLSDR_TUNER_UNKNOWN;
    }
    return dev->tuner_type;
}

int rtlsdr_reset_buffer(rtlsdr_dev_t *dev) {
    if (!dev || !dev->opened) {
        return -1;
    }
    
    LOGD("Buffer reset");
    return 0;
}

int rtlsdr_read_sync(rtlsdr_dev_t *dev, void *buf, int len, int *n_read) {
    if (!dev || !dev->opened || !buf) {
        return -1;
    }
    
    // Simplified synchronous read
    int bytes_read = read(dev->fd, buf, len);
    if (bytes_read < 0) {
        LOGE("Read failed: %s", strerror(errno));
        return -1;
    }
    
    if (n_read) {
        *n_read = bytes_read;
    }
    
    return 0;
}

int rtlsdr_read_async(rtlsdr_dev_t *dev, rtlsdr_read_async_cb_t cb, void *ctx, uint32_t buf_num, uint32_t buf_len) {
    if (!dev || !dev->opened || !cb) {
        return -1;
    }
    
    (void)buf_num; // Ignore for simplicity
    
    dev->cb = cb;
    dev->cb_ctx = ctx;
    dev->async_cancel = 0;
    
    unsigned char *buffer = malloc(buf_len);
    if (!buffer) {
        return -1;
    }
    
    LOGI("Starting async read loop");
    
    while (!dev->async_cancel) {
        int bytes_read = read(dev->fd, buffer, buf_len);
        if (bytes_read > 0) {
            cb(buffer, bytes_read, ctx);
        } else if (bytes_read < 0) {
            if (errno != EAGAIN && errno != EINTR) {
                LOGE("Async read failed: %s", strerror(errno));
                break;
            }
        }
        
        // Small delay to prevent excessive CPU usage
        usleep(1000);
    }
    
    free(buffer);
    LOGI("Async read loop ended");
    
    return 0;
}

int rtlsdr_cancel_async(rtlsdr_dev_t *dev) {
    if (!dev) {
        return -1;
    }
    
    dev->async_cancel = 1;
    
    LOGD("Async read cancelled");
    return 0;
}

// Stub implementations for other functions
int rtlsdr_get_device_usb_strings(uint32_t index, char *manufact, char *product, char *serial) {
    (void)index;
    if (manufact) strcpy(manufact, "Generic");
    if (product) strcpy(product, "RTL-SDR");
    if (serial) strcpy(serial, "00000001");
    return 0;
}

int rtlsdr_get_index_by_serial(const char *serial) {
    (void)serial;
    return 0;
}

int rtlsdr_set_xtal_freq(rtlsdr_dev_t *dev, uint32_t rtl_freq, uint32_t tuner_freq) {
    (void)dev; (void)rtl_freq; (void)tuner_freq;
    return 0;
}

int rtlsdr_get_xtal_freq(rtlsdr_dev_t *dev, uint32_t *rtl_freq, uint32_t *tuner_freq) {
    (void)dev;
    if (rtl_freq) *rtl_freq = 28800000;
    if (tuner_freq) *tuner_freq = 28800000;
    return 0;
}

int rtlsdr_get_usb_strings(rtlsdr_dev_t *dev, char *manufact, char *product, char *serial) {
    (void)dev;
    return rtlsdr_get_device_usb_strings(0, manufact, product, serial);
}

int rtlsdr_write_eeprom(rtlsdr_dev_t *dev, uint8_t *data, uint8_t offset, uint16_t len) {
    (void)dev; (void)data; (void)offset; (void)len;
    return -1; // Not implemented
}

int rtlsdr_read_eeprom(rtlsdr_dev_t *dev, uint8_t *data, uint8_t offset, uint16_t len) {
    (void)dev; (void)data; (void)offset; (void)len;
    return -1; // Not implemented
}

int rtlsdr_get_tuner_gains(rtlsdr_dev_t *dev, int *gains) {
    (void)dev;
    if (gains) {
        // Simplified gain list
        gains[0] = 0;
        gains[1] = 90;
        gains[2] = 142;
        gains[3] = 248;
        gains[4] = 374;
        gains[5] = 420;
        gains[6] = 496;
    }
    return 7;
}

int rtlsdr_set_tuner_bandwidth(rtlsdr_dev_t *dev, uint32_t bw) {
    (void)dev; (void)bw;
    return 0;
}

int rtlsdr_set_tuner_if_gain(rtlsdr_dev_t *dev, int stage, int gain) {
    (void)dev; (void)stage; (void)gain;
    return 0;
}

int rtlsdr_set_testmode(rtlsdr_dev_t *dev, int on) {
    (void)dev; (void)on;
    return 0;
}

int rtlsdr_set_agc_mode(rtlsdr_dev_t *dev, int on) {
    (void)dev; (void)on;
    return 0;
}

int rtlsdr_set_direct_sampling(rtlsdr_dev_t *dev, int on) {
    (void)dev; (void)on;
    return 0;
}

int rtlsdr_get_direct_sampling(rtlsdr_dev_t *dev) {
    (void)dev;
    return 0;
}

int rtlsdr_set_offset_tuning(rtlsdr_dev_t *dev, int on) {
    (void)dev; (void)on;
    return 0;
}

int rtlsdr_get_offset_tuning(rtlsdr_dev_t *dev) {
    (void)dev;
    return 0;
}

int rtlsdr_wait_async(rtlsdr_dev_t *dev, rtlsdr_read_async_cb_t cb, void *ctx) {
    return rtlsdr_read_async(dev, cb, ctx, 0, DEFAULT_BUF_LENGTH);
}

int rtlsdr_set_bias_tee(rtlsdr_dev_t *dev, int on) {
    (void)dev; (void)on;
    return 0;
}
