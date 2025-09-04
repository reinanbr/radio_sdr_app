#ifndef __RTL_SDR_H
#define __RTL_SDR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef struct rtlsdr_dev rtlsdr_dev_t;

#define DEFAULT_SAMPLE_RATE		2048000
#define DEFAULT_ASYNC_BUF_NUMBER	32
#define DEFAULT_BUF_LENGTH		(16 * 16384)
#define MINIMAL_BUF_LENGTH		512
#define MAXIMAL_BUF_LENGTH		(256 * 16384)

enum rtlsdr_tuner {
	RTLSDR_TUNER_UNKNOWN = 0,
	RTLSDR_TUNER_E4000,
	RTLSDR_TUNER_FC0012,
	RTLSDR_TUNER_FC0013,
	RTLSDR_TUNER_FC2580,
	RTLSDR_TUNER_R820T,
	RTLSDR_TUNER_R828D
};

typedef void(*rtlsdr_read_async_cb_t)(unsigned char *buf, uint32_t len, void *ctx);

/*!
 * Get USB device strings.
 * NOTE: The string arguments must provide space for up to 256 bytes.
 */
int rtlsdr_get_device_usb_strings(uint32_t index, char *manufact, char *product, char *serial);

/*!
 * Get device index by USB serial string descriptor.
 */
int rtlsdr_get_index_by_serial(const char *serial);

int rtlsdr_get_device_count(void);

const char* rtlsdr_get_device_name(uint32_t index);

/*!
 * Open RTL-SDR device by index.
 */
int rtlsdr_open(rtlsdr_dev_t **dev, uint32_t index);

/*!
 * Open RTL-SDR device by file descriptor.
 */
int rtlsdr_open_fd(rtlsdr_dev_t **dev, int fd);

int rtlsdr_close(rtlsdr_dev_t *dev);

/* configuration functions */

/*!
 * Set crystal oscillator frequencies used for the RTL2832 and the tuner IC.
 */
int rtlsdr_set_xtal_freq(rtlsdr_dev_t *dev, uint32_t rtl_freq, uint32_t tuner_freq);

/*!
 * Get crystal oscillator frequencies used for the RTL2832 and the tuner IC.
 */
int rtlsdr_get_xtal_freq(rtlsdr_dev_t *dev, uint32_t *rtl_freq, uint32_t *tuner_freq);

int rtlsdr_get_usb_strings(rtlsdr_dev_t *dev, char *manufact, char *product, char *serial);

/*!
 * Write the device EEPROM
 */
int rtlsdr_write_eeprom(rtlsdr_dev_t *dev, uint8_t *data, uint8_t offset, uint16_t len);

/*!
 * Read the device EEPROM
 */
int rtlsdr_read_eeprom(rtlsdr_dev_t *dev, uint8_t *data, uint8_t offset, uint16_t len);

int rtlsdr_set_center_freq(rtlsdr_dev_t *dev, uint32_t freq);

/*!
 * Get actual frequency the device is tuned to.
 */
uint32_t rtlsdr_get_center_freq(rtlsdr_dev_t *dev);

/*!
 * Set the frequency correction value for the device.
 */
int rtlsdr_set_freq_correction(rtlsdr_dev_t *dev, int ppm);

/*!
 * Get actual frequency correction value of the device.
 */
int rtlsdr_get_freq_correction(rtlsdr_dev_t *dev);

enum rtlsdr_tuner rtlsdr_get_tuner_type(rtlsdr_dev_t *dev);

/*!
 * Get a list of gains supported by the tuner.
 */
int rtlsdr_get_tuner_gains(rtlsdr_dev_t *dev, int *gains);

/*!
 * Set the gain for the device.
 * Manual gain mode must be enabled for this to work.
 */
int rtlsdr_set_tuner_gain(rtlsdr_dev_t *dev, int gain);

/*!
 * Set the bandwidth for the device.
 */
int rtlsdr_set_tuner_bandwidth(rtlsdr_dev_t *dev, uint32_t bw);

/*!
 * Get actual gain the device is configured to.
 */
int rtlsdr_get_tuner_gain(rtlsdr_dev_t *dev);

/*!
 * Set the intermediate frequency gain for the device.
 */
int rtlsdr_set_tuner_if_gain(rtlsdr_dev_t *dev, int stage, int gain);

/*!
 * Set the gain mode (automatic/manual) for the device.
 * Manual gain mode must be enabled for the gain setter function to work.
 */
int rtlsdr_set_tuner_gain_mode(rtlsdr_dev_t *dev, int manual);

/*!
 * Set the sample rate for the device, also selects the baseband filters
 * according to the requested sample rate for tuners where this is possible.
 */
int rtlsdr_set_sample_rate(rtlsdr_dev_t *dev, uint32_t rate);

/*!
 * Get actual sample rate the device is configured to.
 */
uint32_t rtlsdr_get_sample_rate(rtlsdr_dev_t *dev);

/*!
 * Enable test mode that returns an 8 bit counter instead of the samples.
 * The counter is generated inside the RTL2832.
 */
int rtlsdr_set_testmode(rtlsdr_dev_t *dev, int on);

/*!
 * Enable or disable the internal digital AGC of the RTL2832.
 */
int rtlsdr_set_agc_mode(rtlsdr_dev_t *dev, int on);

/*!
 * Enable or disable the direct sampling mode. When enabled, the IF mode
 * of the RTL2832 is activated, and rtlsdr_set_center_freq() will control
 * the IF-frequency of the DDC, which can be used to tune from 0 to 28.8 MHz
 * (xtal frequency of the RTL2832).
 */
int rtlsdr_set_direct_sampling(rtlsdr_dev_t *dev, int on);

/*!
 * Get state of the direct sampling mode
 */
int rtlsdr_get_direct_sampling(rtlsdr_dev_t *dev);

/*!
 * Enable or disable offset tuning for zero-IF tuners, which allows to avoid
 * problems caused by the DC offset of the ADCs and 1/f noise.
 */
int rtlsdr_set_offset_tuning(rtlsdr_dev_t *dev, int on);

/*!
 * Get state of the offset tuning mode
 */
int rtlsdr_get_offset_tuning(rtlsdr_dev_t *dev);

/* streaming functions */

int rtlsdr_reset_buffer(rtlsdr_dev_t *dev);

int rtlsdr_read_sync(rtlsdr_dev_t *dev, void *buf, int len, int *n_read);

int rtlsdr_wait_async(rtlsdr_dev_t *dev, rtlsdr_read_async_cb_t cb, void *ctx);

int rtlsdr_read_async(rtlsdr_dev_t *dev,
				   rtlsdr_read_async_cb_t cb,
				   void *ctx,
				   uint32_t buf_num,
				   uint32_t buf_len);

int rtlsdr_cancel_async(rtlsdr_dev_t *dev);

/*!
 * Enable or disable the bias tee on GPIO PIN 0.
 */
int rtlsdr_set_bias_tee(rtlsdr_dev_t *dev, int on);

#ifdef __cplusplus
}
#endif

#endif /* __RTL_SDR_H */
