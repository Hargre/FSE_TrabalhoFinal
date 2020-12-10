#include "nvs.h"
#include "nvs_flash.h"
#include "esp_system.h"
#include "esp_log.h"

#include "storage.h"

int get_initialization_status(char *room) {
    nvs_handle partition_handle;
    esp_err_t res_nvs = nvs_open("store", NVS_READONLY, &partition_handle);
    if (res_nvs == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGE("NVS", "Namespace not found");
        return 0;
    }
    ESP_LOGI("NVS", "%d", res_nvs);

    size_t required_size = 0;

    esp_err_t res = nvs_get_str(partition_handle, "room", NULL, &required_size);
    res = nvs_get_str(partition_handle, "room", room, &required_size);
    ESP_LOGI("NVS", "%d", required_size);
    if (res == ESP_OK) {
        nvs_close(partition_handle);
        return 1;
    }
    ESP_ERROR_CHECK_WITHOUT_ABORT(res);

    nvs_close(partition_handle);
    return 0;
}

void write_room_data(char *data) {
    nvs_handle partition_handle;
    esp_err_t res_nvs = nvs_open("store", NVS_READWRITE, &partition_handle);
    esp_err_t res = nvs_set_str(partition_handle, "room", data);
    nvs_commit(partition_handle);
    nvs_close(partition_handle);
}