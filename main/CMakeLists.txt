idf_component_register(
    SRCS 
    # -------------- IMAGES---------------
    "lv_image/lv_im_bubble_01.c"
    "lv_image/lv_im_bubble_02.c"
    "lv_image/lv_im_drop.c"
    "lv_image/lv_im_module_heat.c"
    "lv_image/lv_im_module_hotwater.c"
    "lv_image/lv_im_module_input_output.c"
    "lv_image/lv_im_module_lock.c"
    "lv_image/lv_im_module_off.c"
    "lv_image/lv_im_module_on.c"
    "lv_image/lv_im_module_podp.c"
    "lv_image/lv_im_pump_off.c"
    "lv_image/lv_im_pump_on.c"
    "lv_image/lv_im_radial_mask.c"
    "lv_image/lv_im_scale_marker.c"
    "lv_image/lv_im_scale.c"
    "lv_image/lv_im_scheme.c"
    "lv_image/lv_im_time.c"
    "lv_image/lv_im_valve_off.c"
    "lv_image/lv_im_valve_on.c"
    "lv_image/lv_im_arrow_down.c"
    "lv_image/lv_im_arrow_left.c"
    "lv_image/lv_im_arrow_right.c"
    "lv_image/lv_im_arrow_up.c"
    "lv_image/lv_im_p_arrow_up.c"
    "lv_image/lv_im_p_arrow_down.c"
    "lv_image/lv_im_comfort.c"
    "lv_image/lv_im_day.c"
    "lv_image/lv_im_hand.c"
    "lv_image/lv_im_night.c"
    "lv_image/lv_im_point_yellow_y.c"
    "lv_image/lv_im_point_yellow_x.c"
    "lv_image/lv_im_point_yellow_change_y.c"
    "lv_image/lv_im_point_yellow_change_x.c"
    "lv_image/lv_im_point_white.c"
    "lv_image/lv_im_all_in_out.c"
    "lv_image/lv_im_analog_out.c"
    "lv_image/lv_im_controller.c"
    "lv_image/lv_im_digital_out.c"
    "lv_image/lv_im_light_gray.c"
    "lv_image/lv_im_light_green.c"
    "lv_image/lv_im_light_red.c"
    "lv_image/lv_im_module_inout.c"
    "lv_image/lv_im_universal_in.c"
    "lv_image/lv_im_radius_yellow.c"
    "lv_image/module_unlock.c"
    "lv_image/lv_im_radius_gray.c"
    "lv_image/lv_im_big_lock_open.c"
    "lv_image/lv_im_big_lock_close.c"

    
    
    # ------------FONTS-----------------------


    "lv_fonts/Roboto_bold_18.c"
    "lv_fonts/Roboto_bold_24.c"
    "lv_fonts/Roboto_bold_36.c"
    "lv_fonts/Roboto_bold_48.c"

        # ------------My_WIDGETS---------------------
        "my_widgets/w_b_small.c"
        "my_widgets/w_b_big.c"
        "my_widgets/w_drop_label.c"
        "my_widgets/w_time_label.c"
        "my_widgets/w_rad_mask.c"
        "my_widgets/w_scale_img.c"
        "my_widgets/w_scale_marker_img.c"
        "my_widgets/w_pump_on_img.c"
        "my_widgets/w_pump_off_img.c"
        "my_widgets/w_valve_on.c"
        "my_widgets/w_valve_off.c"
        "my_widgets/w_heat_exch.c"
        "my_widgets/w_lock_big_close.c"
        "my_widgets/w_lock_big_open.c"
        "my_widgets/w_in_out_main.c"


        #--------Screens-------------------------
   "screens/S_Co/screen_CO.c"
   "screens/S_Gvs/screen_Gvs.c"
   "screens/S_In_Out/screen_In_Out.c"
   "screens/S_Pass/screen_Pass.c"
    "screens/S_Pass/password_screen.c"
   "screens/S_Podp/screen_Podp.c"
   "screens/S_Uv/screen_Uv.c"


            # --------Other------------------------------
    "main.c"
    "encoder/encoder.c" 
    "encoder/encoder_manager.c"    
    "lv_bg_color/lv_bg_main_screen.c"   
    "dialog_screen/screen_YES_NO/yes_no_screen.c"
    "screen_logic/arc_menu.c"
    "screen_logic/screen_navigation.c"
    "counters/count_test.c"
    "menu_layer/main_menu/main_menu.c"
    "scale_logic_time/time_scale.c"




    INCLUDE_DIRS "."
    PRIV_REQUIRES "cmake_utilities" "lvgl" "esp32_s3_lcd_ev_board" "esp_lvgl_port"
)
