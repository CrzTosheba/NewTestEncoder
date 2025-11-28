#include "w_in_out_main.h"
#include "esp_log.h"

static const char *TAG = "W_IN_OUT_MAIN";

lv_obj_t* in_out_pic_main(lv_obj_t *parent)  // Принимаем parent как параметр
{
    if (parent == NULL || !lv_obj_is_valid(parent)) {
        ESP_LOGE(TAG, "Invalid parent for in_out_pic_main");
        return NULL;
    }
    
    lv_obj_t * img = lv_img_create(parent);  // Создаем на переданном parent
    if (img == NULL || !lv_obj_is_valid(img)) {
        ESP_LOGE(TAG, "Failed to create image for in_out_pic_main");
        return NULL;
    }
    
    lv_img_set_src(img, &lv_im_controller);
    
    return img;
}