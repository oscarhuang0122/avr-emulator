// adc.c
#include "adc.h"
#include "interrupts.h"
#include "memory.h"
#include "stdio.h"

void adc_init(ADC *adc)
{
    adc->admux = 0x00;
    adc->adcsra = 0x00;
    adc->adc_data = 0;
    adc->conversion_in_progress = 0;
    adc->conversion_timer = 0;
}

void adc_step(ADC *adc, Memory *mem)
{
    if (!adc->conversion_in_progress)
        return;
    
    if (adc->conversion_timer > 0)
    {
        adc->conversion_timer--;
        return;
    }

    // Fake analog value based on channel
    uint16_t fake_value = ((adc->admux & 0x07) * 128) + 200;
    adc->adc_data = fake_value & 0x3FF;

    // Clear ADSC, set ADIF
    adc->adcsra &= ~(1 << 6);  // Clear ADSC
    adc->adcsra |= (1 << 4);  // Set ADIF (conversion complete)

    // Raise interrupt if enabled
    if (adc->adcsra & (1 << 3))
    {
        raise_interrupt(mem, ADC_INT_VECTOR);
    }

    adc->conversion_in_progress = 0;
}

uint8_t adc_read(ADC *adc, uint16_t addr)
{
    switch(addr)
    {
        case ADMUX_ADDR: return adc->admux;
        case ADCSRA_ADDR: return adc->adcsra;
        case ADCL_ADDR: return adc->adc_data & 0xFF;
        case ADCH_ADDR: return (adc->adc_data >> 8) & 0x03;
        default: return 0xFF; 
    }
}

void adc_write(ADC *adc, uint16_t addr, uint8_t value)
{
    switch (addr)
    {
        case ADMUX_ADDR: 
            adc->admux = value;
            break;
            
        case ADCSRA_ADDR:
        {
            // ADIF is write-one-to-clear.
            uint8_t old_adif = adc->adcsra & (1 << 4);

            adc->adcsra = (value & ~(1 << 4)) | old_adif;

            if (value & (1 << 4))
                adc->adcsra &= (uint8_t)~(1 << 4);

            // ADSC starts a conversion when the ADC is enabled.
            if ((value & (1 << 6)) && (value & (1 << 7)))
            {
                adc->conversion_in_progress = 1;
                adc->conversion_timer = 10;
                adc->adcsra |= (1 << 6);
            }
            break;
        }
        
        case ADCL_ADDR:
        case ADCH_ADDR:
            // Read-only
            break;
    }
}