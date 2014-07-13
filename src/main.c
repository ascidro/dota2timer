#include <pebble.h>

// Constantes.
#define TIME_BUFFER_SIZE 6

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

// Tiempo en el que se inicia el cronómetro.
static int start_time;

// Estado del timer.
static bool started;
static bool paused;

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

  // Adelantamos el tiempo de arranque para compensar por la pausa.
  if (paused)
    start_time++;

  elapsed_time = seconds() - start_time;
  int minutes = elapsed_time / 60;
  int seconds = elapsed_time % 60;

  if (paused && start_time % 2 == 0)
    buffer[0] = '\0';
  else
    snprintf(buffer, 6, "%02d:%02d", minutes, seconds);

  text_layer_set_text(main_text, buffer);
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
    return;
  }

  start_time = seconds();

  // Aparecen los iconos de pausa / roshan.
  action_bar_layer_set_icon(action_bar, BUTTON_ID_UP, button_image_pause);
  action_bar_layer_set_icon(action_bar, BUTTON_ID_DOWN, button_image_roshan);
  action_bar_layer_set_icon(action_bar, BUTTON_ID_SELECT, button_image_stop);
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
  //text_layer_set_text(main_text, "Down");

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
      fonts_get_system_font(FONT_KEY_BITHAM_34_MEDIUM_NUMBERS));
  layer_add_child(window_layer, text_layer_get_layer(main_text));

  roshan_label = text_layer_create(
        (GRect) { .origin = { 0, 50 }, .size = { bounds.size.w - 14, 60 } });
  text_layer_set_text(roshan_label, "Roshan");
  text_layer_set_text_alignment(roshan_label, GTextAlignmentCenter);
  text_layer_set_font(roshan_label,
      fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK));
  layer_add_child(window_layer, text_layer_get_layer(roshan_label));

  roshan_status_text = text_layer_create(
        (GRect) { .origin = { 0, 90 }, .size = { bounds.size.w - 14, 60 } });
  text_layer_set_text(roshan_status_text, "ALIVE");
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
  // Cargo los bitmaps.
  button_image_pause = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BUTTON_PAUSE);
  button_image_roshan = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BUTTON_ROSHAN);
  button_image_start = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BUTTON_START);
  button_image_stop = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BUTTON_STOP);

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
