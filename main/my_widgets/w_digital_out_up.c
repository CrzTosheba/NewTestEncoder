
#include "w_digital_out_up.h"
#include "esp_log.h"

static const char *TAG = "W_DIGITAL_OUT";

lv_obj_t* digital_out_up(lv_obj_t *parent)  // Принимаем parent как параметр
{
    if (parent == NULL || !lv_obj_is_valid(parent)) {
        ESP_LOGE(TAG, "Invalid parent for digital_out_up");
        return NULL;
    }
    
    lv_obj_t * img = lv_img_create(parent);  // Создаем на переданном parent
    if (img == NULL || !lv_obj_is_valid(img)) {
        ESP_LOGE(TAG, "Failed to create image for digital_out_up");
        return NULL;
    }
    
    lv_img_set_src(img, &lv_im_digital_out);
    return img;
}