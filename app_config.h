#ifndef APP_CONFIG_H_
#define APP_CONFIG_H_

/* Encoder and distance parameters. Tune these after real field runs. */
#define ENCODER_COUNTS_PER_REV          (780.0f)
#define WHEEL_CIRCUMFERENCE_CM          (20.4f)

#define FIELD_AB_CM                     (40.0f)
#define FIELD_CD_CM                     (40.0f)
#define FIELD_AC_CM                     (70.7f)
#define FIELD_BD_CM                     (70.7f)

#define DIST_AB_CM                      (FIELD_AB_CM)
#define DIST_CD_CM                      (FIELD_CD_CM)
#define DIST_AC_CM                      (FIELD_AC_CM)
#define DIST_BD_CM                      (FIELD_BD_CM)
#define TASK1_TARGET_DISTANCE_CM        (FIELD_AB_CM)

/* Software yaw offsets. ATK-MS901M yaw is not hardware-zeroed here. */
#define YAW_A_TO_B_DEG                  (0.0f)
#define YAW_C_TO_D_DEG                  (0.0f)
#define YAW_A_TO_C_DEG                  (45.0f)
#define YAW_B_TO_D_DEG                  (45.0f)
#define YAW_B_TO_C_LINE_DEG             (90.0f)
#define YAW_C_TO_B_LINE_DEG             (-90.0f)
#define YAW_D_TO_A_LINE_DEG             (-90.0f)

#define YAW_OFFSET_AB                   (YAW_A_TO_B_DEG)
#define YAW_OFFSET_CD                   (YAW_C_TO_D_DEG)
#define YAW_OFFSET_AC                   (YAW_A_TO_C_DEG)
#define YAW_OFFSET_BD                   (YAW_B_TO_D_DEG)

#define LINE_B_TO_C_CM                  (100.0f)
#define LINE_C_TO_B_CM                  (100.0f)
#define LINE_D_TO_A_CM                  (100.0f)

/* Speed limits. Motor_SetLeft/Right keep their existing project interface. */
#define BLIND_FAST_SPEED                (250)
#define BLIND_SLOW_SPEED                (250)
#define BLIND_MAX_SPEED                 (300)
#define BLIND_SLOW_DOWN_DISTANCE_CM     (25.0f)

#define BLIND_BASE_SPEED_T2             (500)
#define BLIND_BASE_SPEED_T3             (500)
#define BLIND_BASE_SPEED_T4             (500)

#define T2_BLACK_SENSOR_DARK_TH         (160U)
#define T2_BLACK_SENSOR_NEED            (1U)
#define T2_BLIND_BLACK_CONFIRM_NEED     (10U)
#define T2_BLIND_BLACK_IGNORE_CM        (30.0f)
#define T2_BLIND_BLACK_IGNORE_MS        (350U)
#define T2_CD_LEFT_BIAS_ENABLE_CM       (30.0f)
#define T2_CD_LEFT_BIAS_DEG             (10.0f)

#define T2_LINE_EXIT_WHITE_MS           (400U)
#define T2_LINE_EXIT_FORWARD_MS         (200U)
#define T2_LINE_EXIT_ENABLE_CM          (37.0f)
#define T2_LINE_SLOW_AFTER_CM           (30.0f)
#define T2_LINE_LOST_SEARCH_SPEED       (150)
#define T2_LINE_SEARCH_TURN_CMD         (45)

#define LINE_BASE_SPEED                 (400)
#define LINE_SLOW_SPEED                 (260)
#define LINE_MAX_SPEED                  (600)
#define LINE_TURN_SIGN                  (1)

#define BLIND_ALIGN_ENTER_DEG           (5.0f)
#define BLIND_ALIGN_EXIT_DEG            (2.0f)
#define BLIND_ALIGN_BASE_CMD            (420)
#define BLIND_ALIGN_TURN_GAIN           (5.0f)
#define BLIND_ALIGN_MIN_TURN_CMD        (260)
#define ARC_BASE_SPEED                  (LINE_BASE_SPEED)
#define ARC_MAX_SPEED                   (LINE_MAX_SPEED)

/* Control gains. Keep conservative at first; tune on the real car. */
#define BLIND_KP_YAW                    (5.5f)
#define BLIND_KD_YAW                    (1.5f)
#define LINE_KP                         (1.5f)
#define LINE_KD                         (0.7f)
#define ARC_KP_LINE                     (LINE_KP)
#define ARC_KD_LINE                     (LINE_KD)

#define SEG_TIMEOUT_BLIND_MS            (8000U)
#define SEG_TIMEOUT_LINE_MS             (10000U)
#define TASK1_TIMEOUT_BLIND_MS          (12000U)

/* Gray sensor thresholds. White floor is near 255, black line is near 0. */
#define GRAY_BLACK_IS_LOW               (1U)
#define LINE_DARK_SUM_MIN               (80U)
#define LINE_FOUND_THRESHOLD            (LINE_DARK_SUM_MIN)
#define SINGLE_SENSOR_BLACK_TH          (60U)
#define BLACK_DARK_SUM_TH               (260U)
#define BLACK_STOP_THRESHOLD            (BLACK_DARK_SUM_TH)
#define BLACK_CONFIRM_NEED              (5U)
#define BLACK_STOP_CONFIRM_COUNT        (BLACK_CONFIRM_NEED)
#define WHITE_DARK_SUM_TH               (40U)
#define WHITE_DETECT_THRESHOLD          (WHITE_DARK_SUM_TH)
#define WHITE_CONFIRM_NEED              (2U)
#define WHITE_CONFIRM_COUNT             (WHITE_CONFIRM_NEED)
#define LINE_LOST_STOP_COUNT            (20U)
#define TASK1_BLACK_CONFIRM_COUNT       (BLACK_STOP_CONFIRM_COUNT)
#define TASK1_BLACK_STOP_ENABLE_CM      (95.0f)

/* Arc segment settings. A 40 cm radius half circle is about 125.7 cm. */
#define ARC_HALF_LENGTH_CM              (125.7f)
#define ARC_EXIT_ENABLE_CM              (100.0f)
#define ARC_LOST_LINE_LIMIT             (8U)

/* Event feedback timing. */
#define EVENT_BEEP_MS                   (80U)
#define EVENT_FLASH_MS                  (80U)
#define EVENT_GAP_MS                    (80U)
#define EVENT_START_DELAY_MS            (2000U)

#endif /* APP_CONFIG_H_ */
