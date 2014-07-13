#include <pebble.h>

// Constantes.
#define TIME_BUFFER_SIZE 6

// Recursos: ventana.
static Window *window;

// Layer principal del texto, muestra el tiempo que transcurrió.
static TextLayer *main_text;

// Búfer de texto para el tiempo transcurrido.
static char buffer[TIME_BUFFER_SIZE];

// Tiempo en el que se inicia el cronómetro.
static int start_time;

// Indica si el timer arrancó.
static bool started;

// Tiempo que transcurrió desde el arranque.
static int elapsed_time;

// TODO mover a otro lado.
// Helper para obtener los segundos transcurridos
int seconds() {
  time_t seconds;
  seconds = time(NULL);
  return (int)seconds;
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  if (!started)
    return;
    
  elapsed_time = seconds() - start_time;
  int minutes = elapsed_time / 60;
  int seconds = elapsed_time % 60;
  snprintf(buffer, 6, "%02d:%02d", minutes, seconds);

  text_layer_set_text(main_text, buffer);
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  started = !started;
  if (!started)
    return;
  start_time = seconds();
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_text(main_text, "Up");
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_text(main_text, "Down");
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
      (GRect) { .origin = { 0, 10 }, .size = { bounds.size.w, 60 } });
  text_layer_set_text(main_text, "Press SELECT to start");
  text_layer_set_text_alignment(main_text, GTextAlignmentCenter);
  text_layer_set_font(main_text,
      fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  layer_add_child(window_layer, text_layer_get_layer(main_text));
}

static void window_unload(Window *window) {
  text_layer_destroy(main_text);
}

static void init(void) {
  window = window_create();
  window_set_click_config_provider(window, click_config_provider);
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  // TODO investigar qué hace la siguiente línea.
  const bool animated = true;
  // Al principio, no estamos ejecutando.
  started = false;
  // Registro el timer.
  tick_timer_service_subscribe(SECOND_UNIT, (TickHandler) tick_handler);
  window_stack_push(window, animated);
}

static void deinit(void) {
  window_destroy(window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
