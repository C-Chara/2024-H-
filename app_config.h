#ifndef APP_CONFIG_H_
#define APP_CONFIG_H_

/* Encoder and distance parameters. Tune these after real field runs. */
#define ENCODER_COUNTS_PER_REV          (1560.0f)
#define WHEEL_CIRCUMFERENCE_CM          (20.4f)
#define DIST_AB_CM                      (100.0f)
#define DIST_CD_CM                      (100.0f)
#define DIST_AC_CM                      (141.4f)
#define DIST_BD_CM                      (141.4f)
#define TASK1_TARGET_DISTANCE_CM        (DIST_AB_CM)

/* Software yaw offsets. ATK-MS901M yaw is not hardware-zeroed here. */
#define YAW_OFFSET_AB                   (0.0f)
#define YAW_OFFSET_CD                   (0.0f)
#define YAW_OFFSET_AC                   (45.0f)
#define YAW_OFFSET_BD                   (45.0f)

/* Speed limits. Motor_SetLeft/Right keep their existing project interface. */
#define BLIND_FAST_SPEED                (260)
#define BLIND_SLOW_SPEED                (170)
#define BLIND_MAX_SPEED                 (500)
#define BLIND_SLOW_DOWN_DISTANCE_CM     (25.0f)

#define ARC_BASE_SPEED                  (120)
#define ARC_MAX_SPEED                   (300)

/* Control gains. Keep conservative at first; tune on the real car. */
#define BLIND_KP_YAW                    (4.0f)
#define BLIND_KD_YAW                    (1.0f)
#define ARC_KP_LINE                     (0.45f)
#define ARC_KD_LINE                     (0.0f)

/* Gray sensor thresholds. White floor is near 255, black line is near 0. */
#define LINE_FOUND_THRESHOLD            (80U)
#define BLACK_STOP_THRESHOLD            (180U)
#define BLACK_STOP_CONFIRM_COUNT        (4U)
#define WHITE_DETECT_THRESHOLD          (40U)
#define WHITE_CONFIRM_COUNT             (4U)
#define TASK1_BLACK_CONFIRM_COUNT       (BLACK_STOP_CONFIRM_COUNT)
#define TASK1_BLACK_STOP_ENABLE_CM      (80.0f)

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
