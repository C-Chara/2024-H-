#include "app_line.h"

#include "Algorithm/gray.h"

volatile int16_t line_error = 0;
volatile uint8_t line_found = 0U;
volatile uint16_t line_confidence = 0U;

static const int16_t line_weights[8] = {
    -350, -250, -150, -50, 50, 150, 250, 350
};

#define LINE_DARK_SUM_THRESHOLD  (80U)

void AppLine_Init(void)
{
    line_error = 0;
    line_found = 0U;
    line_confidence = 0U;
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
        return;
    }

    for (uint8_t i = 0U; i < 8U; i++) {
        /*
         * The field is white floor with a narrow black arc line.
         * Invert normalized gray values so the black-line strength is larger.
         */
        uint8_t dark = (uint8_t)(255U - gray_values[i]);

        weighted_sum += (int32_t)dark * line_weights[i];
        sum = (uint16_t)(sum + dark);
    }

    line_confidence = sum;

    if (sum < LINE_DARK_SUM_THRESHOLD) {
        line_found = 0U;
        line_error = 0;
        return;
    }

    line_found = 1U;
    line_error = (int16_t)(weighted_sum / (int32_t)sum);
}
