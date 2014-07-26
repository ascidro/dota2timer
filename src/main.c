#include <pebble.h>
#include "dota2timer.h"

// RECURSOS DE UI:
// Ventana principal.
static Window *window;

// Layer principal del texto, muestra el tiempo que transcurrió.
static TextLayer *main_text;

// Layer de texto que solo dice "Roshan".
static TextLayer *roshan_label;

// Estado de Roshan: muerto, vivo, chances.
static TextLayer *roshan_status_text;

// Barra que muestra qué hace cada botón.
static ActionBarLayer *action_bar;

// Botones de la barra de acción.
static GBitmap* button_image_pause;
static GBitmap* button_image_roshan;
static GBitmap* button_image_start;
static GBitmap* button_image_stop;

// VARIABLES DE ESTADO:
// Búfer de texto para el tiempo transcurrido.
static char buffer[TIME_BUFFER_SIZE];

static char roshan_status_buffer[ROSHAN_STATUS_BUFFER_SIZE];

// Tiempo en el que se inicia el cronómetro.
static int start_time;

// Segundo en el que mataron a Roshan.
static int roshan_killed_time;

// Estado del timer.
static bool started;
static bool paused;

// Probabilidad de que Roshan esté vivo, en porcentaje.
static int roshan_status;

// Tiempo que transcurrió desde el arranque.
static int elapsed_time;

// VARIABLES DE CONFIGURACIÓN:
enum config_keys {
  ISCANCEL, ALERT53
};
static bool alert53;

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  if (!started)
    return;

  // Adelantamos el tiempo de arranque para compensar por la pausa.
  if (paused)
    start_time++;

  elapsed_time = seconds() - start_time;
  int minutes = elapsed_time / 60;
  int seconds_elapsed = elapsed_time % 60;

  // Si está pausado, que titile una vez por segundo.
  if (paused && start_time % 2 == 0)
    buffer[0] = '\0';
  else   // Si no, mostrar la hora normalmente.
    snprintf(buffer, 6, "%02d:%02d", minutes, seconds_elapsed);

  text_layer_set_text(main_text, buffer);

  if (paused)
    return;

  // Calculo las posibilidades de que Roshan esté vivo.
  if (roshan_status < ROSHAN_ALIVE) {
    int roshan_killed_elapsed = elapsed_time - roshan_killed_time;
    if (roshan_killed_elapsed > ROSHAN_RESPAWN_TIME_LOWER)
      roshan_status = (roshan_killed_elapsed - ROSHAN_RESPAWN_TIME_LOWER) * 100
          / (ROSHAN_RESPAWN_TIME_UPPER - ROSHAN_RESPAWN_TIME_LOWER);
    if (roshan_status > ROSHAN_ALIVE)
      roshan_status = ROSHAN_ALIVE;
    get_string_for_roshan(roshan_status, roshan_status_buffer);
    text_layer_set_text(roshan_status_text, roshan_status_buffer);
  }

  if (alert53 && elapsed_time % 60 == 53)
    vibes_short_pulse();
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (paused)
    return;
  started = !started;
  if (!started) {
    // Desaparecen los iconos de pausa / roshan.
    action_bar_layer_set_icon(action_bar, BUTTON_ID_UP, NULL);
    action_bar_layer_set_icon(action_bar, BUTTON_ID_DOWN, NULL);
    action_bar_layer_set_icon(action_bar, BUTTON_ID_SELECT, button_image_start);
    text_layer_set_text(roshan_status_text, "--");
    return;
  }

  start_time = seconds();
  roshan_status = ROSHAN_ALIVE;

  // Aparecen los iconos de pausa / roshan.
  action_bar_layer_set_icon(action_bar, BUTTON_ID_UP, button_image_pause);
  action_bar_layer_set_icon(action_bar, BUTTON_ID_DOWN, button_image_roshan);
  action_bar_layer_set_icon(action_bar, BUTTON_ID_SELECT, button_image_stop);
  text_layer_set_text(roshan_status_text, "ALIVE");
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  paused = !paused;
  if (paused) {
    action_bar_layer_set_icon(action_bar, BUTTON_ID_UP, button_image_start);
    action_bar_layer_set_icon(action_bar, BUTTON_ID_SELECT, NULL);
    action_bar_layer_set_icon(action_bar, BUTTON_ID_DOWN, NULL);
    return;
  }
  action_bar_layer_set_icon(action_bar, BUTTON_ID_UP, button_image_pause);
  action_bar_layer_set_icon(action_bar, BUTTON_ID_DOWN, button_image_roshan);
  action_bar_layer_set_icon(action_bar, BUTTON_ID_SELECT, button_image_stop);
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  roshan_status = ROSHAN_DEAD;
  roshan_killed_time = seconds() - start_time;;
  get_string_for_roshan(roshan_status, roshan_status_buffer);
  text_layer_set_text(roshan_status_text, roshan_status_buffer);
}

static void in_received_handler(DictionaryIterator *received, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Received data");
  Tuple *alert_tuple = dict_find(received, ALERT53);

  if (alert_tuple) {
    alert53 = strcmp(alert_tuple->value->cstring, "true") == 0;
    APP_LOG(APP_LOG_LEVEL_INFO, "Alert status: %d", alert53);
  } else {
    APP_LOG(APP_LOG_LEVEL_INFO, "It was unknown data");
  }
}

static void in_dropped_handler(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped: %d", reason);
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  main_text = text_layer_create(
      (GRect) { .origin = { 0, 3 }, .size = { bounds.size.w - 14, 60 } });
  text_layer_set_text(main_text, "00:00");
  text_layer_set_text_alignment(main_text, GTextAlignmentCenter);
  text_layer_set_font(main_text,
      fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT));
  layer_add_child(window_layer, text_layer_get_layer(main_text));

  roshan_label = text_layer_create(
        (GRect) { .origin = { 0, 76 }, .size = { bounds.size.w - 14, 60 } });
  text_layer_set_text(roshan_label, "Roshan");
  text_layer_set_text_alignment(roshan_label, GTextAlignmentCenter);
  text_layer_set_font(roshan_label,
      fonts_get_system_font(FONT_KEY_GOTHIC_14));
  layer_add_child(window_layer, text_layer_get_layer(roshan_label));

  roshan_status_text = text_layer_create(
        (GRect) { .origin = { 0, 90 }, .size = { bounds.size.w - 14, 60 } });
  text_layer_set_text(roshan_status_text, "--");
  text_layer_set_text_alignment(roshan_status_text, GTextAlignmentCenter);
  text_layer_set_font(roshan_status_text,
      fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  layer_add_child(window_layer, text_layer_get_layer(roshan_status_text));

  action_bar = action_bar_layer_create();
  action_bar_layer_add_to_window(action_bar, window);
  // Al principio solo muestro el icono de start.
  action_bar_layer_set_icon(action_bar, BUTTON_ID_SELECT, button_image_start);
  // Registro los botones.
  action_bar_layer_set_click_config_provider(action_bar, click_config_provider);
}

static void window_unload(Window *window) {
  text_layer_destroy(main_text);
}

static void init(void) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Starting app...");
  // Registrar el handler de entrada de mensajes.
  app_message_register_inbox_received(in_received_handler);
  app_message_register_inbox_dropped(in_dropped_handler);

  app_message_open(INBOUND_SIZE, OUTBOUND_SIZE);

  // Cargo los bitmaps.
  button_image_pause = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BUTTON_PAUSE);
  button_image_roshan = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BUTTON_ROSHAN);
  button_image_start = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BUTTON_START);
  button_image_stop = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BUTTON_STOP);

  // Cargo la configuración por defecto.
  alert53 = false;

  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });

  // Animar la entrada a la aplicación.
  const bool animated = true;
  // Al principio, no estamos ejecutando.
  started = false;
  paused = false;

  // Registro el timer.
  tick_timer_service_subscribe(SECOND_UNIT, (TickHandler) tick_handler);
  window_stack_push(window, animated);
}

static void deinit(void) {
  action_bar_layer_destroy(action_bar);
  window_destroy(window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
