float gReadingMax = 0.0;            // Holds maximum reading
float gReadingMin = 99999.9;        // Holds minimum reading
float gReadingAvg = 0.0;            // Holds average reading
float gReadingLast = 0.0;           // Holds value of last reading
uint32_t nLastADC = 0;              // Last sampled ADC value
uint32_t numMeasurements = 0;       // Counts how many readings we had
uint32_t nNextMeasurementTick = 0;  // When next measurement should occur


uint32_t adc_sample(void)
{
    return random(512, 520);
}

uint32_t adc_sample_RNG(void)
{
    return random(470, 780);
}

float get_last(void)
{
    return gReadingLast;
}
float get_min(void)
{
    return gReadingMin;
}
float get_max(void)
{
    return gReadingMax;
}
float get_avg(void)
{
    return gReadingAvg;
}
uint32_t get_adc(void)
{
    return nLastADC;
}
uint32_t get_measurements_count(void)
{
    return numMeasurements;
}

void reset_stats(void)
{
    gReadingMax = 0.0;
    gReadingMin = 99999.9;
    gReadingAvg = 0.0;
    numMeasurements = 0;
}


void Measurements_Tick(void)
{
    if (millis() >= nNextMeasurementTick)
    {
        // Get latest measurement
        // nLastADC = adc_sample_RNG();

        // Take `ADC_SAMPLES_PER_MEASUREMENT_CYCLE` number of ADC samples
        nLastADC = adc_sample_data(ADC_SAMPLES_PER_MEASUREMENT_CYCLE);

        // Should we use mean or average (already calculated above)?
        #ifdef ADC_FINAL_ADC_VALUE_USING_MEAN
        // Use ADC mean
        nLastADC = adc_get_mean();
        #endif

        if (nLastADC >= ADC_MAX)
        {
            Serial.print("Invalid ADC value of `");
            Serial.print(nLastADC);
            Serial.print("`. Clipping to `");
            Serial.print(ADC_MAX);
            Serial.println("`.");
            nLastADC = ADC_MAX;
        }

        gReadingLast = adc_to_mm(nLastADC);

        // Update average
        if (numMeasurements > 1)
        {
            gReadingAvg = (gReadingAvg + gReadingLast) / 2;
        }
        else
        {
            gReadingAvg = gReadingLast;
        }

        // Update min
        if (gReadingLast < gReadingMin)
        {
            gReadingMin = gReadingLast;
        }

        // Update max
        if (gReadingLast > gReadingMax)
        {
            gReadingMax = gReadingLast;
        }

        // Update reading counter
        numMeasurements++;

        // Update the status LED
        if (bStatusLED)
        {
            bStatusLED = false;
            LED_MEASUREMENT_OFF();
        }
        else
        {
            bStatusLED = true;
            LED_MEASUREMENT_ON();
        }

        // Measurement printing over USB-CDC
        #ifdef CONFIG_PRINT_MEASUREMENTS_USB_CDC
        if (bSerialPrintoutRequested && (Serial.availableForWrite()>9))
        {
            Serial.print(">");
            Serial.print(gReadingLast);
            Serial.print("mm\r\n");

            if (bStatusLED)
            {
                LED_SERIAL_ON();
            }
            else
            {
                LED_SERIAL_OFF();
            }
        }
        #endif // CONFIG_PRINT_MEASUREMENTS_USB_CDC

        // Measurement printing over Serial
        #ifdef CONFIG_PRINT_MEASUREMENTS_UART_GPIO
        Serial0.print(">");
        Serial0.print(gReadingLast);
        Serial0.print("mm\r\n");
        #endif //CONFIG_PRINT_MEASUREMENTS_UART_GPIO


        // Set next update timestamp
        nNextMeasurementTick = millis() + 200;
    }
}
