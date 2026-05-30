#include "encoder.h"

#include "app_config.h"
#include "app_time.h"
#include "ti_msp_dl_config.h"

volatile int32_t encoder_left_total = 0;
volatile int32_t encoder_right_total = 0;
volatile int32_t encoder_left_start = 0;
volatile int32_t encoder_right_start = 0;
volatile int32_t encoder_left_delta = 0;
volatile int32_t encoder_right_delta = 0;
volatile float encoder_distance_cm = 0.0f;
volatile uint8_t encoder_distance_source = 0U;
volatile uint32_t encoder_left_irq_count = 0U;
volatile uint32_t encoder_right_irq_count = 0U;
volatile uint8_t encoder_valid = 0U;
volatile uint32_t encoder_last_update_tick = 0U;

static int32_t Encoder_Abs32(int32_t value)
{
    return (value >= 0) ? value : -value;
}

static void Encoder_UpdateDistanceCache(void)
{
    int32_t left_abs;
    int32_t right_abs;
    float distance_counts;

    encoder_left_delta = encoder_left_total - encoder_left_start;
    encoder_right_delta = encoder_right_total - encoder_right_start;

    left_abs = Encoder_Abs32(encoder_left_delta);
    right_abs = Encoder_Abs32(encoder_right_delta);

    if (left_abs > 0 && right_abs > 0 &&
        left_abs * 4 >= right_abs && right_abs * 4 >= left_abs) {
        distance_counts = ((float)left_abs + (float)right_abs) * 0.5f;
        encoder_distance_source = 0U;
    } else if (right_abs >= left_abs) {
        distance_counts = (float)right_abs;
        encoder_distance_source = 2U;
    } else {
        distance_counts = (float)left_abs;
        encoder_distance_source = 1U;
    }

    encoder_distance_cm = distance_counts * WHEEL_CIRCUMFERENCE_CM /
        ENCODER_COUNTS_PER_REV;
    encoder_valid = 1U;
    encoder_last_update_tick = app_millis();
}

void Encoder_Init(void)
{
    encoder_left_total = 0;
    encoder_right_total = 0;
    encoder_left_irq_count = 0U;
    encoder_right_irq_count = 0U;
    encoder_distance_source = 0U;
    encoder_valid = 0U;
    encoder_last_update_tick = 0U;
    Encoder_ResetDistance();

#if defined(ENCODER_GPIO_ENC1_A_PORT) && defined(ENCODER_GPIO_ENC1_A_PIN)
    DL_GPIO_clearInterruptStatus(ENCODER_GPIO_ENC1_A_PORT,
        ENCODER_GPIO_ENC1_A_PIN);
    DL_GPIO_enableInterrupt(ENCODER_GPIO_ENC1_A_PORT,
        ENCODER_GPIO_ENC1_A_PIN);
#endif
#if defined(ENCODER_GPIO_ENC2_A_PORT) && defined(ENCODER_GPIO_ENC2_A_PIN)
    DL_GPIO_clearInterruptStatus(ENCODER_GPIO_ENC2_A_PORT,
        ENCODER_GPIO_ENC2_A_PIN);
    DL_GPIO_enableInterrupt(ENCODER_GPIO_ENC2_A_PORT,
        ENCODER_GPIO_ENC2_A_PIN);
#endif

#if defined(ENCODER_GPIO_GPIOA_INT_IRQN)
    NVIC_ClearPendingIRQ(ENCODER_GPIO_GPIOA_INT_IRQN);
    NVIC_EnableIRQ(ENCODER_GPIO_GPIOA_INT_IRQN);
#endif
#if defined(ENCODER_GPIO_GPIOB_INT_IRQN)
    NVIC_ClearPendingIRQ(ENCODER_GPIO_GPIOB_INT_IRQN);
    NVIC_EnableIRQ(ENCODER_GPIO_GPIOB_INT_IRQN);
#endif
}

void Encoder_ResetDistance(void)
{
    encoder_left_start = encoder_left_total;
    encoder_right_start = encoder_right_total;
    Encoder_UpdateDistanceCache();
}

int32_t Encoder_GetLeftTotal(void)
{
    return encoder_left_total;
}

int32_t Encoder_GetRightTotal(void)
{
    return encoder_right_total;
}

int32_t Encoder_GetLeftDelta(void)
{
    encoder_left_delta = encoder_left_total - encoder_left_start;
    return encoder_left_delta;
}

int32_t Encoder_GetRightDelta(void)
{
    encoder_right_delta = encoder_right_total - encoder_right_start;
    return encoder_right_delta;
}

float Encoder_GetDistanceCm(void)
{
    Encoder_UpdateDistanceCache();
    return encoder_distance_cm;
}

void Encoder_Task(void)
{
    Encoder_UpdateDistanceCache();
}

static void Encoder_HandleLeftA(void)
{
#if defined(ENCODER_GPIO_ENC1_A_PORT) && defined(ENCODER_GPIO_ENC1_A_PIN) && \
    defined(ENCODER_GPIO_ENC1_B_PORT) && defined(ENCODER_GPIO_ENC1_B_PIN)
    if ((DL_GPIO_readPins(ENCODER_GPIO_ENC1_B_PORT,
        ENCODER_GPIO_ENC1_B_PIN) & ENCODER_GPIO_ENC1_B_PIN) != 0U) {
        encoder_left_total += ENCODER_LEFT_SIGN;
    } else {
        encoder_left_total -= ENCODER_LEFT_SIGN;
    }

    encoder_left_irq_count++;
    DL_GPIO_clearInterruptStatus(ENCODER_GPIO_ENC1_A_PORT,
        ENCODER_GPIO_ENC1_A_PIN);
#else
    /* Replace with real ENC1_A/ENC1_B macros from ti_msp_dl_config.h. */
#endif
}

static void Encoder_HandleRightA(void)
{
#if defined(ENCODER_GPIO_ENC2_A_PORT) && defined(ENCODER_GPIO_ENC2_A_PIN) && \
    defined(ENCODER_GPIO_ENC2_B_PORT) && defined(ENCODER_GPIO_ENC2_B_PIN)
    if ((DL_GPIO_readPins(ENCODER_GPIO_ENC2_B_PORT,
        ENCODER_GPIO_ENC2_B_PIN) & ENCODER_GPIO_ENC2_B_PIN) != 0U) {
        encoder_right_total += ENCODER_RIGHT_SIGN;
    } else {
        encoder_right_total -= ENCODER_RIGHT_SIGN;
    }

    encoder_right_irq_count++;
    DL_GPIO_clearInterruptStatus(ENCODER_GPIO_ENC2_A_PORT,
        ENCODER_GPIO_ENC2_A_PIN);
#else
    /* Replace with real ENC2_A/ENC2_B macros from ti_msp_dl_config.h. */
#endif
}

void GROUP1_IRQHandler(void)
{
    uint32_t group_iidx;
    uint8_t guard = 0U;

    do {
        group_iidx = DL_Interrupt_getPendingGroup(DL_INTERRUPT_GROUP_1);

        switch (group_iidx) {
#if defined(ENCODER_GPIO_GPIOA_INT_IIDX) && defined(ENCODER_GPIO_ENC1_A_IIDX)
        case ENCODER_GPIO_GPIOA_INT_IIDX:
            if (DL_GPIO_getPendingInterrupt(ENCODER_GPIO_ENC1_A_PORT) ==
                ENCODER_GPIO_ENC1_A_IIDX) {
                Encoder_HandleLeftA();
            }
            break;
#endif

#if defined(ENCODER_GPIO_GPIOB_INT_IIDX) && defined(ENCODER_GPIO_ENC2_A_IIDX)
        case ENCODER_GPIO_GPIOB_INT_IIDX:
            if (DL_GPIO_getPendingInterrupt(ENCODER_GPIO_ENC2_A_PORT) ==
                ENCODER_GPIO_ENC2_A_IIDX) {
                Encoder_HandleRightA();
            }
            break;
#endif

        default:
            break;
        }

        guard++;
    } while ((group_iidx != 0U) && (guard < 4U));
}
