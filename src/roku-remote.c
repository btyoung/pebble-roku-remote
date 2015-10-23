#include <pebble.h>

static Window *window;
static TextLayer *text_layer;

enum {
  KEYPRESS = 0,
};

/*****************************/
/* Data Model                */
/*****************************/
short selected_col;
const char *columns[][3] = {
  {"Play", "Rev", "Fwd"},
  {"Home", "Back", "Select"},
  {"Left", "Right", "Info"},
  {"Up", "Down", "InstantReplay"},
  {"Backspace", "Search", "Enter"},
};

static void init_columns(void) {
  selected_col = 0;
}


/*****************************/
/* CLICK HANDLERS            */
/*****************************/
/* Single click handlers that are remote button presses */
static void sendKey(const char *keylabel) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Sending key");
  APP_LOG(APP_LOG_LEVEL_INFO, keylabel);
  DictionaryIterator* iter = NULL;
  app_message_outbox_begin(&iter);
  dict_write_cstring(iter, KEYPRESS, keylabel);
  dict_write_end(iter);
  app_message_outbox_send();
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  sendKey(columns[selected_col][1]);
  text_layer_set_text(text_layer, columns[selected_col][1]);
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  sendKey(columns[selected_col][0]);
  text_layer_set_text(text_layer, columns[selected_col][0]);
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  sendKey(columns[selected_col][2]);
  text_layer_set_text(text_layer, columns[selected_col][2]);
}

/* Column-changing handlers */
char label[10];

static void drawCol(void) {
  snprintf(label, 10, "Column %d", selected_col);
  text_layer_set_text(text_layer, label);
}

static void nextcol_click_handler(ClickRecognizerRef recognizer, void *context) {
  selected_col = (selected_col + 1) % 5;
  drawCol();
}

static void prevcol_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (selected_col == 0) {
    selected_col = 4;
  } else {
    selected_col = (selected_col - 1);
  }
  drawCol();
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);

  window_long_click_subscribe(BUTTON_ID_UP, 500, NULL, prevcol_click_handler);
  window_long_click_subscribe(BUTTON_ID_DOWN, 500, NULL, nextcol_click_handler);
  window_multi_click_subscribe(BUTTON_ID_UP, 2, 0, 300, true, prevcol_click_handler);
  window_multi_click_subscribe(BUTTON_ID_DOWN, 2, 0, 300, true, nextcol_click_handler);
}

/*******************************/
/* Window Management           */
/*******************************/
static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  text_layer = text_layer_create((GRect) { .origin = { 0, 72 }, .size = { bounds.size.w, 20 } });
  text_layer_set_text(text_layer, "Press a button");
  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(text_layer));
}

static void window_unload(Window *window) {
  text_layer_destroy(text_layer);
}


/*******************************/
/* Message Management          */
/*******************************/
static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message received!");
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}


/*******************************/
/* App Startup/shutdown        */
/*******************************/
static void init(void) {
  // Set up data
  init_columns();

  // Set up messages
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  app_message_open(app_message_inbox_size_maximum(),
                   app_message_outbox_size_maximum());

  // Set up windows
  window = window_create();
  window_set_click_config_provider(window, click_config_provider);
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  const bool animated = true;
  window_stack_push(window, animated);
}

static void deinit(void) {
  window_destroy(window);
}

int main(void) {
  // APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);
  init();
  app_event_loop();
  deinit();
}
