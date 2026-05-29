#include "key.h"

#include <stdint.h>

#include "app_time.h"
#include "ti_msp_dl_config.h"

#define KEY_DEBOUNCE_MS     (20U)
#define KEY_LONG_PRESS_MS   (1000U)

#if defined(KEY_GPIO_KEY_MODE_PORT) && defined(KEY_GPIO_KEY_MODE_PIN)
#define KEY_MODE_PORT       KEY_GPIO_KEY_MODE_PORT
#define KEY_MODE_PIN        KEY_GPIO_KEY_MODE_PIN
#else
/* Here needs replacement with the real KEY_MODE macros from ti_msp_dl_config.h. */
#define KEY_MODE_PORT       GPIOB
#define KEY_MODE_PIN        DL_GPIO_PIN_21
#endif

#if defined(KEY_GPIO_KEY_START_PORT) && defined(KEY_GPIO_KEY_START_PIN)
#define KEY_START_PORT      KEY_GPIO_KEY_START_PORT
#define KEY_START_PIN       KEY_GPIO_KEY_START_PIN
#else
/* Here needs replacement with the real KEY_START macros from ti_msp_dl_config.h. */
#define KEY_START_PORT      GPIOA
#define KEY_START_PIN       DL_GPIO_PIN_12
#endif

typedef struct {
    uint8_t stable_pressed;
    uint8_t last_raw_pressed;
    uint8_t debounce_ms;
    uint16_t pressed_ms;
    uint8_t long_sent;
} Key_State;

static Key_State key_mode;
static Key_State key_start;
static volatile uint8_t key_mode_short_event = 0U;
static volatile uint8_t key_start_short_event = 0U;
static volatile uint8_t key_start_long_event = 0U;

static uint8_t Key_IsPressed(GPIO_Regs *port, uint32_t pin)
{
    return (DL_GPIO_readPins(port, pin) == 0U) ? 1U : 0U;
}

static void Key_Update(Key_State *key, uint8_t raw_pressed,
    volatile uint8_t *short_event, volatile uint8_t *long_event)
{
    if (raw_pressed == key->last_raw_pressed) {
        if (key->debounce_ms < KEY_DEBOUNCE_MS) {
            key->debounce_ms++;
        }
    } else {
        key->last_raw_pressed = raw_pressed;
        key->debounce_ms = 0U;
    }

    if (key->debounce_ms >= KEY_DEBOUNCE_MS &&
        raw_pressed != key->stable_pressed) {
        key->stable_pressed = raw_pressed;

        if (key->stable_pressed != 0U) {
            key->pressed_ms = 0U;
            key->long_sent = 0U;
        } else {
            if (key->long_sent == 0U) {
                *short_event = 1U;
            }
            key->pressed_ms = 0U;
        }
    }

    if (key->stable_pressed != 0U) {
        if (key->pressed_ms < 60000U) {
            key->pressed_ms++;
        }

        if (long_event != 0 &&
            key->long_sent == 0U &&
            key->pressed_ms >= KEY_LONG_PRESS_MS) {
            *long_event = 1U;
            key->long_sent = 1U;
        }
    }
}

void Key_Init(void)
{
    key_mode.stable_pressed = Key_IsPressed(KEY_MODE_PORT, KEY_MODE_PIN);
    key_mode.last_raw_pressed = key_mode.stable_pressed;
    key_mode.debounce_ms = 0U;
    key_mode.pressed_ms = 0U;
    key_mode.long_sent = 0U;

    key_start.stable_pressed = Key_IsPressed(KEY_START_PORT, KEY_START_PIN);
    key_start.last_raw_pressed = key_start.stable_pressed;
    key_start.debounce_ms = 0U;
    key_start.pressed_ms = 0U;
    key_start.long_sent = 0U;

    key_mode_short_event = 0U;
    key_start_short_event = 0U;
    key_start_long_event = 0U;
}

void Key_Task(void)
{
    static uint32_t last_key_tick = 0U;
    uint32_t now = app_millis();

    if (now == last_key_tick) {
        return;
    }

    last_key_tick = now;
    Key_Update(&key_mode, Key_IsPressed(KEY_MODE_PORT, KEY_MODE_PIN),
        &key_mode_short_event, 0);
    Key_Update(&key_start, Key_IsPressed(KEY_START_PORT, KEY_START_PIN),
        &key_start_short_event, &key_start_long_event);
}

uint8_t Key_GetModeShortPress(void)
{
    uint8_t event = key_mode_short_event;

    key_mode_short_event = 0U;
    return event;
}

uint8_t Key_GetStartShortPress(void)
{
    uint8_t event = key_start_short_event;

    key_start_short_event = 0U;
    return event;
}

uint8_t Key_GetStartLongPress(void)
{
    uint8_t event = key_start_long_event;

    key_start_long_event = 0U;
    return event;
}
