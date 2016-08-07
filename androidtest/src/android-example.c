#include <pebble.h>

#define KEY_BUTTON    0
#define KEY_VIBRATE   1
#define KEY_AXISX    2
#define KEY_AXISY   3
#define KEY_AXISZ    4
#define KEY_KM       5


#define BUTTON_UP     0
#define BUTTON_SELECT 1
#define BUTTON_DOWN   2

static Window *s_main_window;
static TextLayer *s_text_layer;
static TextLayer *ey;
static char texto[15] ="NADA";

/******************************* AppMessage ***********************************/

static void send(int key, int message) {
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  dict_write_int(iter, key, &message, sizeof(int), true);

  app_message_outbox_send();
}

static void inbox_received_handler(DictionaryIterator *iterator, void *context) {
  // Get the first pair
  Tuple *t = dict_read_first(iterator);
  
  snprintf(texto, sizeof(texto), "KM: %d", (int)t->value->int32);
  //snprintf(texto, sizeof(texto), "KM");
  text_layer_set_text(ey, texto);
  
  APP_LOG(APP_LOG_LEVEL_INFO, "--------------%d", (int)t->value->int32);
  APP_LOG(APP_LOG_LEVEL_INFO, "----TEXTOOO----%s", texto);
  
  // Process all pairs present
  while(t != NULL) {
    // Process this pair's key
    switch(t->key) {
      case KEY_VIBRATE:
        // Trigger vibration
        //text_layer_set_text(s_text_layer, "KM!");
      //text_layer_set_text(ey, "PEPE");
        //vibes_short_pulse();
        break;
      default:
        APP_LOG(APP_LOG_LEVEL_INFO, "Unknown key: %d", (int)t->key);
        break;
    }

    // Get next pair, if any
    t = dict_read_next(iterator);
  }
}

static void inbox_dropped_handler(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_handler(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_handler(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

/********************************* Buttons ************************************/

//static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
//  text_layer_set_text(s_text_layer, "Select");
//
//  send(KEY_BUTTON, BUTTON_SELECT);
//}

//static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
//  text_layer_set_text(s_text_layer, "Up");
//
//  send(KEY_BUTTON, BUTTON_UP);
//}

//static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
//  text_layer_set_text(s_text_layer, "Down");
//
//  send(KEY_BUTTON, BUTTON_DOWN);
//}



/********************************** Axis **************************************/
static void SendAxes(int x, int y, int z) {
  send(KEY_AXISX, x);
  //send(KEY_AXISX, y);
  //send(KEY_AXISX, z);
}

static void accel_data_handler(AccelData *data, uint32_t num_samples) {
  // Read sample 0's x, y, and z values
  int16_t x = data[0].x;
  int16_t y = data[0].y;
  int16_t z = data[0].z;

  // Determine if the sample occured during vibration, and when it occured
  bool did_vibrate = data[0].did_vibrate;
  long unsigned timestamp = data[0].timestamp;

  if(!did_vibrate) {
    // Print it out
    APP_LOG(APP_LOG_LEVEL_INFO, "t: %lu , x: %d, y: %d, z: %d",timestamp, x, y, z);
    send(KEY_AXISX, x);
     //snprintf(ax, sizeof(ax), "%d", x);
      //text_layer_set_text(ex, ax);
         //snprintf(ay, sizeof(ay), "%d", y);
      //text_layer_set_text(ey, ay);
         //snprintf(az, sizeof(az), "%d", z);
      //text_layer_set_text(ez, az);
 // } else {
    // Discard with a warning
  //  APP_LOG(APP_LOG_LEVEL_WARNING, "Vibration occured during collection");
  }
}

//static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
//SendAxes();
//}

static void click_config_provider(void *context) {
  // Assign button handlers
  //window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  //window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  //window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
  accel_data_service_subscribe(5, accel_data_handler);
}

/******************************* main_window **********************************/

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);
  
  //Create main TextLayer
  s_text_layer = text_layer_create(bounds);
      ey = text_layer_create(
      GRect(0, PBL_IF_ROUND_ELSE(110, 110), bounds.size.w, 50));
  //
  text_layer_set_font(s_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text(s_text_layer, "00000000");
  text_layer_set_text_alignment(s_text_layer, GTextAlignmentCenter);
  
  
  //

    text_layer_set_background_color(ey, GColorClear);
  text_layer_set_text_color(ey, GColorBlack);
  text_layer_set_text(ey, texto);
  text_layer_set_font(ey, fonts_get_system_font(FONT_KEY_GOTHIC_09));
  text_layer_set_text_alignment(ey, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_text_layer));
  layer_add_child(window_layer, text_layer_get_layer(ey));
#ifdef PBL_ROUND
  //text_layer_enable_screen_text_flow_and_paging(s_text_layer, 5);
#endif
}

static void main_window_unload(Window *window) {
  // Destroy main TextLayer
  text_layer_destroy(s_text_layer);
}

static void init(void) {
  // Register callbacks
  app_message_register_inbox_received(inbox_received_handler);
  app_message_register_inbox_dropped(inbox_dropped_handler);
  app_message_register_outbox_failed(outbox_failed_handler);
  app_message_register_outbox_sent(outbox_sent_handler);
  //Accelerometer
  accel_data_service_subscribe(5, accel_data_handler);
  // Open AppMessage
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());

  // Create main Window
  s_main_window = window_create();
  window_set_click_config_provider(s_main_window, click_config_provider);
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload,
  });
  window_stack_push(s_main_window, true);
  
  
  // Register with TickTimerService
  //tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
}

static void deinit(void) {
  // Destroy main Window
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
