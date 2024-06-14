#include <string.h>
#include "esp_log.h"
#include "driver/i2c.h"
#include "driver/gpio.h"
#include "board.h"

#include "codec.h"

static const char *TAG = "sond_codec";

static bool codec_init_flag;

audio_hal_func_t AUDIO_SOND_CODEC_DEFAULT_HANDLE = {
    .audio_codec_initialize = sond_codec_init,
    .audio_codec_deinitialize = sond_codec_deinit,
    .audio_codec_ctrl = sond_codec_ctrl_state,
    .audio_codec_config_iface = sond_codec_config_i2s,
    .audio_codec_set_mute = sond_codec_set_voice_mute,
    .audio_codec_set_volume = sond_codec_set_voice_volume,
    .audio_codec_get_volume = sond_codec_get_voice_volume,
};

bool sond_codec_initialized()
{
    return codec_init_flag;
}

esp_err_t sond_codec_init(audio_hal_codec_config_t *cfg)
{
    ESP_LOGI(TAG, "sond_codec init");
    return ESP_OK;
}

esp_err_t sond_codec_deinit(void)
{
    return ESP_OK;
}

esp_err_t sond_codec_ctrl_state(audio_hal_codec_mode_t mode, audio_hal_ctrl_t ctrl_state)
{
    return ESP_OK;
}

esp_err_t sond_codec_config_i2s(audio_hal_codec_mode_t mode, audio_hal_codec_i2s_iface_t *iface)
{
    return ESP_OK;
}

esp_err_t sond_codec_set_voice_mute(bool mute)
{
    return ESP_OK;
}

esp_err_t sond_codec_set_voice_volume(int volume)
{
    return ESP_OK;
}

esp_err_t sond_codec_get_voice_volume(int *volume)
{
    return ESP_OK;
}