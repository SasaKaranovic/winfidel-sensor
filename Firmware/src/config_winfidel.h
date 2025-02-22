#ifndef __WINFIDEL_CFG__
#define __WINFIDEL_CFG__

#define MDNS_NAME       "winfidel"
#define WIFI_HOSTNAME   "WInFiDEL by SK"


#define CALIBRATION_POINT_SAMPLE_COUNT      32      // Number of ADC samples to take for new calibration point
#define CALIBRATION_POINT_ACCURACY_POINT    50      // We expect all CALIBRATION_POINT_SAMPLE_COUNT samples to be
                                                    // within CALIBRATION_POINT_ACCURACY_POINT of each other
                                                    // in order to have successful calibration point creation

#define CALIBRATION_BOUNDARY_ITEMS          2       // We need lower and upper boundary item for safety
#define MAX_CALIBRATION_POINTS              (10 + CALIBRATION_BOUNDARY_ITEMS)
#define ADC_MIN                             0       // Minimum value we can get from our ADC (should be always 0)
#define ADC_MIN_EQUALS_MM                   5.0f    // ADC_MIN should be considered this diameter (in mm)
#define ADC_MAX                             4095    // Maximum value we can get from our ADC (should be 2^n-bits)
#define ADC_MAX_EQUALS_MM                   0.0f    // ADC_MAX should be considered this diameter (in mm)

#define CAL_POINT_EMPTY                     { .adc = ADC_MIN, .mm = ADC_MIN_EQUALS_MM }
#define CAL_POINT_FIRST                     { .adc = ADC_MIN, .mm = ADC_MIN_EQUALS_MM }
#define CAL_POINT_LAST                      { .adc = ADC_MAX, .mm = ADC_MAX_EQUALS_MM }

#define PRINT_MEASUREMENT_OVER_SERIAL               // Define if we want serial printout for every measurement

#define ADC_MAX_SAMPLES                     64      // Maximum number of ADC samples we can take in one measurement cycle
#define ADC_SAMPLES_PER_MEASUREMENT_CYCLE   16      // How many samples to take for each measurement cycle
#define ADC_FINAL_ADC_VALUE_USING_MEAN              // Use mean instead of average ADC value

#define ADC_CHIP_IS_MCP3221                         // Specify which ADC chip we are using
// #define ADC_CHIP_MCP3021                            // Specify which ADC chip we are using
#define ADC_I2C_ADDRESS                     0x4D    // 7-bit I2C address of our ADC IC
#define I2C_SCL_PIN                         0       // SCL pin number
#define I2C_SDA_PIN                         1       // SDA pin number

#define LED_RED_PIN                         5       // RED LED pin number
#define LED_GREEN_PIN                       4       // GREEN LED pin number
#define LED_BLUE_PIN                        3       // BLUE LED pin number
#define LED_MEASUREMENT_PIN                 LED_GREEN_PIN
#define LED_SERIAL_PIN                      LED_BLUE_PIN

#define LED_RED_ON()                        do { digitalWrite(LED_RED_PIN, LOW); } while(0)
#define LED_RED_OFF()                       do { digitalWrite(LED_RED_PIN, HIGH); } while(0)
#define LED_GREEN_ON()                      do { digitalWrite(LED_GREEN_PIN, LOW); } while(0)
#define LED_GREEN_OFF()                     do { digitalWrite(LED_GREEN_PIN, HIGH); } while(0)
#define LED_BLUE_ON()                       do { digitalWrite(LED_BLUE_PIN, LOW); } while(0)
#define LED_BLUE_OFF()                      do { digitalWrite(LED_BLUE_PIN, HIGH); } while(0)
#define LED_MEASUREMENT_ON()                do { digitalWrite(LED_MEASUREMENT_PIN, LOW); } while(0)
#define LED_MEASUREMENT_OFF()               do { digitalWrite(LED_MEASUREMENT_PIN, HIGH); } while(0)
#define LED_SERIAL_ON()                     do { digitalWrite(LED_SERIAL_PIN, LOW); } while(0)
#define LED_SERIAL_OFF()                    do { digitalWrite(LED_SERIAL_PIN, HIGH); } while(0)

// String buffers
#define SENSOR_STR_MAX_LEN                  256
#define CALIBRATION_JSON_STR_MAX_LEN        512

#define EEPROM_MAGIC_KEY                    0x53    // Predefined value used to verify EEPROM has been initialized


// Enum holding calibration results
typedef enum winfidel_status
{
    WINFIDEL_OK,
    WINFIDEL_FAIL,
    WINFIDEL_FORBIDDEN,
    WINFIDEL_INVALID_ARGUMENT,
    WINFIDEL_MISSING,
    WINFIDEL_NO_SPACE,
    WINFIDEL_LOW_ACCURACY

} winfidel_status_t;


// Calibration point struct -> holds ADC value and equivalent mm diameter
typedef struct cal_point
{
    uint32_t adc;
    float mm;
} cal_point_t;

// Calibration struct -> holds all calibration points and total number of valid calibration points
typedef struct calibration
{
    cal_point_t points[MAX_CALIBRATION_POINTS];
    uint32_t numPoints;
    uint8_t valid;
} calibration_t;


#endif
