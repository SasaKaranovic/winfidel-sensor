#include <Wire.h>

uint32_t adc_samples[ADC_MAX_SAMPLES] = {0};
uint8_t rxBuff[2] = {0};
uint8_t rxPos = 0;
uint32_t nADCTimeoutTick = 0;

void adc_init(void)
{
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
}


// Helper function to indicate (in the UI) which
// algo is used for calculating final ADC value
String adc_get_algo(void)
{
#ifdef ADC_FINAL_ADC_VALUE_USING_MEAN
    return String("MEAN");
#else
    return String("AVERAGE");
#endif
}

String adc_get_chip(void)
{
#ifdef ADC_CHIP_IS_MCP3221
    return String("MCP3221");
#else
    return String("MCP3021");
#endif
}

// This function will take `N` samples and store them into an array
// After that we should use one of the available functions to calculate value
// ie. calculate average, mean, trimmed_mean etc.
uint32_t adc_sample_data(uint8_t nSamples)
{
    if(nSamples > ADC_MAX_SAMPLES)
    {
        Serial.print("Limiting number of samples to ");
        Serial.println(ADC_MAX_SAMPLES);
        nSamples = ADC_MAX_SAMPLES;
    }
    else if (nSamples <= 0)
    {
        Serial.println("Requested invalid number of samples. Aborting.");
        return 0;
    }

    uint32_t accumulator = 0;
    uint32_t sample = 0;
    rxPos = 0;

    while(rxPos++ < nSamples)
    {
        // Request two bytes
        Wire.requestFrom(ADC_I2C_ADDRESS, 2);
        // // Wait for data to become available
        nADCTimeoutTick = millis() + 100;
        while((!Wire.available()) && (millis()<=nADCTimeoutTick));

        // Read two bytes
        rxBuff[0] = Wire.read();
        rxBuff[1] = Wire.read();

        // Accumulate value
        #ifdef ADC_CHIP_IS_MCP3221
        // Using MCP3221
        sample = ( (rxBuff[0] << 8 | (rxBuff[1])));
        #else
        // Using MCP3021
        sample = ( (rxBuff[0] << 6 | (rxBuff[1]) >> 2));
        #endif

        adc_samples[rxPos] = sample;
        accumulator += sample;
    }

    // Divide value with number of samples
    return (accumulator/nSamples);
}

int qsort_compare( const void* a, const void* b)
{
   uint32_t int_a = * ( (uint32_t*) a );
   uint32_t int_b = * ( (uint32_t*) b );

   // an easy expression for comparing
   return (int_a > int_b) - (int_a < int_b);
}

uint32_t adc_get_mean(void)
{
    // We want to have 3 or more samples
    if (rxPos <= 2)
    {
        Serial.println("Insufficient number of samples. Returing the first value");
        return adc_samples[0];
    }
    // Sort samples
    qsort(adc_samples, rxPos, sizeof(uint32_t), qsort_compare);

    // Get the "middle" element
    // (we will round up/down on even number of samples, but we can live with that for now)
    return adc_samples[rxPos/2];
}



void i2c_scan_bus(void)
{
  uint8_t error, address;
  int nDevices;

    Serial.println("Scanning...");

  nDevices = 0;
  for(address = 1; address < 127; address++ )
  {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0)
    {
      Serial.print("I2C device found at address 0x");
      if (address<16)
        Serial.print("0");
      Serial.print(address,HEX);
      Serial.println("  !");

      nDevices++;
    }
    else if (error==4)
    {
      Serial.print("Unknown error at address 0x");
      if (address<16)
        Serial.print("0");
      Serial.println(address,HEX);
    }
  }
  if (nDevices == 0)
    Serial.println("No I2C devices found\n");
  else
    Serial.println("done\n");
}
