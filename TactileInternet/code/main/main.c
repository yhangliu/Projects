/* Authors: Hayato Nakamura
            Yang Hang Liu
            Arnaud Harmange 
*/

#include "main.h"

void app_main() {
    // initialization
    init();
    
    // every INTERVAL ms, sample sensor values and send converted values to console
    while(1) {
        uint32_t ir_adc_reading = 0;
        uint32_t ultrasonic_adc_reading = 0;
        uint32_t thermistor_adc_reading = 0;

        for (int i = 0; i < NO_OF_SAMPLES; i++) {
            if (unit == ADC_UNIT_1) {
                ir_adc_reading += adc1_get_raw((adc1_channel_t)ir_channel);
                ultrasonic_adc_reading += adc1_get_raw((adc1_channel_t)ultrasonic_channel);
                thermistor_adc_reading += adc1_get_raw((adc1_channel_t)thermistor_channel);
            } else {
                int ir_raw;
                int ultrasonic_raw;
                int thermistor_raw;
                adc2_get_raw((adc2_channel_t)thermistor_adc_reading, ADC_WIDTH_BIT_12, &thermistor_raw);
                thermistor_adc_reading += thermistor_raw;
                adc2_get_raw((adc2_channel_t)ir_channel, ADC_WIDTH_BIT_12, &ir_raw);
                ir_adc_reading += ir_raw;
                adc2_get_raw((adc2_channel_t)ultrasonic_channel, ADC_WIDTH_BIT_12, &ultrasonic_raw);
                ultrasonic_adc_reading += ultrasonic_raw;
            }
            vTaskDelay(pdMS_TO_TICKS(INTERVAL/NO_OF_SAMPLES));
        }

        /* === For Thermistor === */
        // average adc reading over # of samples
        thermistor_adc_reading /= NO_OF_SAMPLES;
        // convert adc_reading to voltage in mV
        thermistor_voltage = esp_adc_cal_raw_to_voltage(thermistor_adc_reading, adc_chars);
        // calculate thermistor reg value from known reg value, input vol, and measured vol
        thermistor_reg = KNOWN_REGISTOR * ((float)INPUT_VOLTAGE/thermistor_voltage - 1);
        // Steinhart-Hart Equation
        log_reg = log(thermistor_reg);
        T = (1.0 / (c1 + c2*log_reg + c3*log_reg*log_reg*log_reg));
        Tc = T - 273.15; // Kelvin to Celcius
        
        /* === For ultrasonic sensor === */
        // average adc reading over # of samples
        ultrasonic_adc_reading /= NO_OF_SAMPLES;
        //Convert adc_reading to voltage in mV
        ultrasonic_voltage = esp_adc_cal_raw_to_voltage(ultrasonic_adc_reading, adc_chars);
        // Voltage scaling of MaxSonar-EZ: Vcc/1024 = Vi
        // where Vcc = Supplied Voltage, Vi = volts per 5mm
        // Range formula: 5*(Vm/Vi) = Ri
        // where Vm = Measured Voltage, Ri = Range in mm
        ultrasonic_detected_dist = (float)(ultrasonic_voltage / (3300 / 1024) * 5); // in mm
        ultrasonic_detected_dist /= 1000; // in m
        // printf("UL | range: %fmm\t and: %fm\n", ultrasonic_detected_dist, ultrasonic_detected_dist/1000);


        /* === For IR Rangefinder === */
        // average adc reading over # of samples
        ir_adc_reading /= NO_OF_SAMPLES;
        // convert adc_reading to voltage in mV
        ir_voltage = esp_adc_cal_raw_to_voltage(ir_adc_reading, adc_chars);
        // below equation is derived by curve fitting the plot of cm v.s. voltage
        ir_detected_dist = 160697 * pow(ir_voltage, -1.14); // in cm
        ir_detected_dist /= 100; // in m

        // temperature, ultrasonic, and IR
        printf("%f,%f,%f,%f\n", timer, Tc, ultrasonic_detected_dist, ir_detected_dist);
        timer += INTERVAL / 1000;
    }
}

void init() {
    //Check if Two Point or Vref are burned into eFuse
    check_efuse();

    // Configure ADC
    if (unit == ADC_UNIT_1) {
        adc1_config_width(ADC_WIDTH_BIT_12);
        adc1_config_channel_atten(thermistor_channel, atten);
        adc1_config_channel_atten(ultrasonic_channel, atten);
        adc1_config_channel_atten(ir_channel, atten);
    } else {
        adc2_config_channel_atten((adc2_channel_t)thermistor_channel, atten);
        adc2_config_channel_atten((adc2_channel_t)ultrasonic_channel, atten);
        adc2_config_channel_atten((adc2_channel_t)ir_channel, atten);
    }

    //Characterize ADC
    adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize(unit, atten, ADC_WIDTH_BIT_12, DEFAULT_VREF, adc_chars);
    print_char_val_type(val_type);
}

static void check_efuse(void) {
    //Check TP is burned into eFuse
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_TP) == ESP_OK)
        printf("eFuse Two Point: Supported\n");
    else
        printf("eFuse Two Point: NOT supported\n");

    //Check Vref is burned into eFuse
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_VREF) == ESP_OK)
        printf("eFuse Vref: Supported\n");
    else
        printf("eFuse Vref: NOT supported\n");
}

static void print_char_val_type(esp_adc_cal_value_t val_type) {
    if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP)
        printf("Characterized using Two Point Value\n");
    else if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF)
        printf("Characterized using eFuse Vref\n");
    else
        printf("Characterized using Default Vref\n");
}