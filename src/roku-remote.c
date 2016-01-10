#include <pebble.h>

static Window *window;
static Layer *col_indicator;

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
  //{"Backspace", "Search", "Enter"},
};
BitmapLayer *icons[4][3];
const uint32_t icon_resources[][3] = {
  {RESOURCE_ID_ROKU_PLAYPAUSE, RESOURCE_ID_ROKU_REV, RESOURCE_ID_ROKU_FWD},
  {RESOURCE_ID_ROKU_HOME, RESOURCE_ID_ROKU_BACK, RESOURCE_ID_ROKU_SELECT},
  {RESOURCE_ID_ROKU_LEFT, RESOURCE_ID_ROKU_RIGHT, RESOURCE_ID_ROKU_INFO},
  {RESOURCE_ID_ROKU_UP, RESOURCE_ID_ROKU_DOWN, RESOURCE_ID_ROKU_REPLAY},
};
const int vpos[] = {50, 95, 140};
const int hpos[] = {18, 54, 90, 126};

static void init_columns(void) {
  selected_col = 0;
}

static PropertyAnimation *s_property_animation;

static void set_column_bar(int col) {
  // Set start and end
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  GRect from_frame = layer_get_frame(col_indicator);
  GRect to_frame = GRect(hpos[selected_col]-20, 0, 40, bounds.size.h);

  // Create the animation
  s_property_animation = property_animation_create_layer_frame(
                  col_indicator, &from_frame, &to_frame);
  animation_set_duration((Animation *) s_property_animation, 100);

  animation_schedule((Animation*) s_property_animation);
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
  //text_layer_set_text(text_layer, columns[selected_col][1]);
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  sendKey(columns[selected_col][0]);
 // text_layer_set_text(text_layer, columns[selected_col][0]);
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  sendKey(columns[selected_col][2]);
 // text_layer_set_text(text_layer, columns[selected_col][2]);
}

/* Column-changing handlers */
char label[10];

static void drawCol(void) {
  snprintf(label, 10, "Column %d", selected_col);
  set_column_bar(selected_col);
  // text_layer_set_text(text_layer, label);
}

static void nextcol_click_handler(ClickRecognizerRef recognizer, void *context) {
  selected_col = (selected_col + 1) % 4;
  drawCol();
}

static void prevcol_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (selected_col == 0) {
    selected_col = 3;
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

/***********************************/
/* Window and Graphics Management  */
/***********************************/
// Define list of icons
// Define overlay bar
// Set method to slide bar left/right on press
// Set method to Bounce icon when clicked


static void col_indicator_update(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);

  graphics_context_set_fill_color(ctx, GColorFromRGB(100,100,255));
  graphics_fill_rect(ctx, bounds, 5, GCornersAll);
}


static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  window_set_background_color(window, GColorFromRGB(0, 0, 0));

  // Set up column indicator
  col_indicator = layer_create((GRect) {
    .origin = { hpos[selected_col] - 18, 0 },
    .size = {36, bounds.size.h},
  });
  layer_set_update_proc(col_indicator, col_indicator_update);
  layer_add_child(window_layer, col_indicator);

  // Set up Icons
  for (int row=0; row < 3; row++) {
    for (int col=0; col < 4; col++) {
      icons[col][row] = bitmap_layer_create((GRect) {
        .origin = {hpos[col] - 10, vpos[row] - 10},
        .size = {20, 20}
      });
      bitmap_layer_set_alignment(icons[col][row], GAlignCenter);
      bitmap_layer_set_compositing_mode(icons[col][row], GCompOpSet);
      
      bitmap_layer_set_bitmap(icons[col][row],
              gbitmap_create_with_resource(icon_resources[col][row]));
      layer_add_child(window_layer, bitmap_layer_get_layer(icons[col][row]));
    }
  }
}


static void window_unload(Window *window) {
  layer_destroy(col_indicator);
  for (int row=0; row < 3; row++) {
    for (int col=0; col < 4; col++) {
      bitmap_layer_destroy(icons[col][row]);
    }
  }
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
