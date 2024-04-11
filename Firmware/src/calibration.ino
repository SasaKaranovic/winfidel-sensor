/*
    We will use a struct to hold all of our calibration points.
    Using known calibration points, we can interpolate what is the
    measured value for any ADC value as long as we have two calibration
    so that one is lower and the other is higher than the current reading.

    Our calibration points look like: ADC_VALUE -> CALIBRATED_MM_VALUE
    * We will keep all of our calibration vales sorted by ASC value of ADC_VALUE

    For example, let's assume we have below calibration points
    620 -> 2.0mm
    850 -> 1.5mm
    915 -> 1.0mm

    In order to calculate what is the `mm` equivalent for ADC_VALUE value of `750`
    1) Find calibration point that is higher than current ADC_VALUE and use it as HIGH_POINT
    1.1) Make sure there that there is a lower (we use boundary items to guarantee this) and use it as LOW_POINT
    2) Calculate the slope between two points (LOW_POINT and HIGH_POINT)
    3) Calculate the Y(mm) value using LOW_POINT and the calculated slope
*/
#include "config_winfidel.h"


static calibration_t cal = {0};
char calibration_json[CALIBRATION_JSON_STR_MAX_LEN] = {0};


void calibration_init(void)
{
    bool res;

    res = calibration_read_from_eeprom();
    if (res == false)
    {
        calibration_reset();
    }
}

bool calibration_read_from_eeprom(void)
{
    EEPROM.readBytes(0, (void *)&cal, sizeof(calibration_t));
    if (cal.valid == EEPROM_MAGIC_KEY)
    {
        Serial.println("Calibration loaded from EEPROM");
        return true;
    }
    Serial.println("Calibration in EEPROM is invalid");
    return false;
}

void calibration_save_to_eeprom(void)
{
    EEPROM.writeBytes(0, (void *)&cal, sizeof(calibration_t));
    EEPROM.commit();
}

void calibration_reset(void)
{
    for(uint8_t i=0; i<MAX_CALIBRATION_POINTS; i++)
    {
        cal.points[i] = CAL_POINT_EMPTY;
    }
    cal.points[0] = CAL_POINT_FIRST;
    cal.points[1] = CAL_POINT_LAST;
    cal.numPoints = 2;
    cal.valid = EEPROM_MAGIC_KEY;
    sort_calibration_points();
    calibration_save_to_eeprom();

    Serial.println("Default calibration loaded");
}


// Create new calibration point using provided `mm` value
// and sampled ADC value
winfidel_status_t create_calibration_point(float mm)
{
    // Check if we already have maximum number of point
    if(cal.numPoints >= MAX_CALIBRATION_POINTS)
    {
        return WINFIDEL_NO_SPACE;
    }

    uint32_t adc_samples[CALIBRATION_POINT_SAMPLE_COUNT] = {0};
    uint32_t adc_min = ADC_MAX;
    uint32_t adc_max = ADC_MIN;
    uint32_t adc_average = 0;

    // Sample the ADC value CALIBRATION_POINT_SAMPLE_COUNT times
    for(uint8_t i=0; i<CALIBRATION_POINT_SAMPLE_COUNT; i++)
    {
        // Sample ADC
        adc_samples[i] = adc_sample_data(1);

        // Update minimum
        if(adc_samples[i] < adc_min)
        {
            adc_min = adc_samples[i];
        }

        // Update maximum
        if(adc_samples[i] > adc_max)
        {
            adc_max = adc_samples[i];
        }
    }

    // Check our max-min is less than CALIBRATION_POINT_ACCURACY_POINT
    if( (adc_max-adc_min) > CALIBRATION_POINT_ACCURACY_POINT)
    {
        Serial.print("ADC accuracy was too low: ");
        Serial.print((adc_max-adc_min));
        Serial.print(" points between min and max!");

        return WINFIDEL_LOW_ACCURACY;
    }

    // Average out ADC
    for(uint8_t i=0; i<CALIBRATION_POINT_SAMPLE_COUNT; i++)
    {
        adc_average += adc_samples[i];
    }
    adc_average = adc_average/CALIBRATION_POINT_SAMPLE_COUNT;

    manually_create_calibration_point(adc_average, mm);
    return WINFIDEL_OK;
}

// Create calibration point wih given ADC value and MM diameter
winfidel_status_t manually_create_calibration_point(uint32_t adc_average, float mm)
{
    bool existing_point = false;

    // Check if we already have maximum number of point
    if(cal.numPoints >= MAX_CALIBRATION_POINTS)
    {
        return WINFIDEL_NO_SPACE;
    }

    // Check if we are creating new point or updating existing `mm` point with new ADC value
    for(uint8_t i=0; i<MAX_CALIBRATION_POINTS; i++)
    {
        if(cal.points[i].mm == mm || cal.points[i].adc == adc_average)
        {
            existing_point = true;
            cal.points[i].adc = adc_average;
            cal.points[i].mm = mm;
            break;
        }
    }

    // We didn't find existing calibration point with same `mm` value
    if (existing_point == false)
    {
        // Create new point
        cal.points[cal.numPoints++] = { .adc = adc_average, .mm = mm };
    }

    sort_calibration_points();
    calibration_save_to_eeprom();
    return WINFIDEL_OK;
}

// Remove calibration point by using given mm value
// We will find the position of this calibration point
// and then use `remove_calibration_point_pos` to remove it
winfidel_status_t remove_calibration_point_mm(float mm)
{
    for(uint8_t i=0; i<MAX_CALIBRATION_POINTS; i++)
    {
        if(cal.points[i].mm == mm)
        {
            return remove_calibration_point_pos(i);
        }
    }

    return WINFIDEL_MISSING;
}


// Remove calibration point at index N
// We need to POP the N-th calibration point and shift all
// calibration values up
winfidel_status_t remove_calibration_point_pos(uint8_t pos)
{
    // We don't allow removing first and last (cal.numPoints) calibration points
    if(pos == 0 || pos == cal.numPoints-1)
    {
        return WINFIDEL_FORBIDDEN;
    }

    // Shift up all calibration points starting from pos
    for(uint8_t i=pos; i<MAX_CALIBRATION_POINTS-1; i++)
    {
        cal.points[i] = cal.points[i+1];
    }

    // Decrement number of calibration points
    cal.numPoints--;
    calibration_save_to_eeprom();

    return WINFIDEL_OK;
}

// This function should only be called inside `sort_calibration_points`
static void swap_calibration_points(uint8_t i, uint8_t j)
{
    cal_point_t temp = cal.points[i];
    cal.points[i] = cal.points[j];
    cal.points[j] = temp;
}

// Sort all calibration points in ASC order using ADC value
void sort_calibration_points(void)
{
    uint8_t i, j;
    for (i = 0; i < cal.numPoints - 1; i++)
    {
        // Last i elements are already
        // in place
        for (j = 0; j < cal.numPoints - i - 1; j++)
        {
            if (cal.points[j].adc > cal.points[j + 1].adc)
            {
                swap_calibration_points(j, j + 1);
            }
        }
    }
}

// Format JSON string that contains all calibration points
char *get_calibration_json(void)
{
    int32_t pos = 0;
    int32_t len = 0;

    // Start of JSON
    len = snprintf(calibration_json, CALIBRATION_JSON_STR_MAX_LEN-pos, "{\"status\":\"ok\",\"data\":[");
    pos += len;

    // Append each calibration point
#if 1
    for (uint8_t i=0; i<cal.numPoints; i++) // Send ONLY currently used calibration points
#else
    for (uint8_t i=0; i<MAX_CALIBRATION_POINTS; i++) // Send ALL (used and unused) calibration points
#endif
    {
        len = snprintf(&calibration_json[pos], CALIBRATION_JSON_STR_MAX_LEN-pos, "[%d, \"%.3f\"],",
                       cal.points[i].adc, cal.points[i].mm);
        pos += len;
    }

    // End of JSON
    snprintf(&calibration_json[pos-1], CALIBRATION_JSON_STR_MAX_LEN-pos, "]}");

    return calibration_json;
}

// Convert given ADC value to mm using available calibration points
float adc_to_mm(uint32_t adc)
{
    for (uint8_t i=0; i<cal.numPoints; i++)
    {
        if(cal.points[i].adc > adc)
        {
            float slope = ((float)cal.points[i].mm - cal.points[i-1].mm) / ((float)cal.points[i].adc - cal.points[i-1].adc);
            float indiff = ((float)adc - cal.points[i-1].adc);
            float outdiff = slope * indiff;
            float out = outdiff + cal.points[i-1].mm;
            return (out);
        }
    }

    // We should never reach this section
    Serial.println("Failed to find calibration point. Returning 0.0mm");
    return 0.0f;
}
