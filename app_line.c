#include "app_line.h"

#include "Algorithm/gray.h"
#include "app_config.h"

volatile int16_t line_error = 0;
volatile uint8_t line_found = 0U;
volatile uint16_t line_confidence = 0U;
volatile uint16_t line_dark_sum = 0U;
volatile uint8_t black_detected = 0U;
volatile uint8_t white_detected = 0U;

static uint8_t black_confirm_count = 0U;
static uint8_t white_confirm_count = 0U;

static const int16_t line_weights[8] = {
    -350, -250, -150, -50, 50, 150, 250, 350
};

void AppLine_Init(void)
{
    line_error = 0;
    line_found = 0U;
    line_confidence = 0U;
    line_dark_sum = 0U;
    black_detected = 0U;
    white_detected = 0U;
    black_confirm_count = 0U;
    white_confirm_count = 0U;
}

void AppLine_Task(void)
{
    const uint8_t gray_values[8] = {
        gray_value_0, gray_value_1, gray_value_2, gray_value_3,
        gray_value_4, gray_value_5, gray_value_6, gray_value_7
    };
    int32_t weighted_sum = 0;
    uint16_t sum = 0U;

    if (gray_read_ok == 0U) {
        line_found = 0U;
        line_error = 0;
        line_confidence = 0U;
        line_dark_sum = 0U;
        black_detected = 0U;
        white_detected = 0U;
        black_confirm_count = 0U;
        white_confirm_count = 0U;
        return;
    }

    for (uint8_t i = 0U; i < 8U; i++) {
        /*
         * I2C gray values are 0..255: white is high, black is low.
         * Use black strength for the centroid because the line is black.
         */
        uint8_t dark = (uint8_t)(255U - gray_values[i]);

        weighted_sum += (int32_t)dark * line_weights[i];
        sum = (uint16_t)(sum + dark);
    }

    line_dark_sum = sum;
    line_confidence = sum;

    if (sum >= LINE_FOUND_THRESHOLD) {
        line_found = 1U;
        line_error = (int16_t)(weighted_sum / (int32_t)sum);
    } else {
        line_found = 0U;
        line_error = 0;
    }

    if (sum >= BLACK_STOP_THRESHOLD) {
        if (black_confirm_count < 255U) {
            black_confirm_count++;
        }
    } else {
        black_confirm_count = 0U;
    }
    black_detected =
        (black_confirm_count >= BLACK_STOP_CONFIRM_COUNT) ? 1U : 0U;

    if (sum <= WHITE_DETECT_THRESHOLD) {
        if (white_confirm_count < 255U) {
            white_confirm_count++;
        }
    } else {
        white_confirm_count = 0U;
    }
    white_detected =
        (white_confirm_count >= WHITE_CONFIRM_COUNT) ? 1U : 0U;
}
