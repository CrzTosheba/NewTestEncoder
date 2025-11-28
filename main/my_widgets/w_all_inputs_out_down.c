
#include "w_all_inputs_out_down.h"
#include "esp_log.h"

static const char *TAG = "W_ALL_IN_OUT";

lv_obj_t* all_in_out_down(lv_obj_t *parent)  // Принимаем parent как параметр
{
    if (parent == NULL || !lv_obj_is_valid(parent)) {
        ESP_LOGE(TAG, "Invalid parent for all_in_out_down");
        return NULL;
    }
    
    lv_obj_t * img = lv_img_create(parent);  // Создаем на переданном parent
    if (img == NULL || !lv_obj_is_valid(img)) {
        ESP_LOGE(TAG, "Failed to create image for all_in_out_down");
        return NULL;
    }
    
    lv_img_set_src(img, &lv_im_all_in_out);
    return img;
}