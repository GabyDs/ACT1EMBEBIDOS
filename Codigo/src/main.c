/*PINES

Entradas:
PD2 arranque directo
PD3 cambiar el tiempo
PD4 arranque suave

Salidas:
PC0 LEDpwm
PC1 LED1
PC2 LED2
PC3 LED3
PC4 LED3
PC5 LED3

*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define F_CPU 16000000L

#define T 20 // ms, Periodo
#define pasos 20
#define tiempo_rebote 6 // ms

enum numero_pasos
{
    T1 = (5000 / T) / pasos,  // 12
    T2 = (8000 / T) / pasos,  // 20
    T3 = (11000 / T) / pasos, // 27
    T4 = (14000 / T) / pasos  // 35
};

int tiempo = T1;
volatile int flag = 0;
int encendido = 0;

void arranque_suave();
void arranque_directo();
void apagado_directo();
void ciclar_tiempo();

ISR(INT0_vect)
{
    _delay_ms(tiempo_rebote);
    if (!(PIND & (1 << PIND2)))
        flag = 1;
}

int main(void)
{
    // ENTRADAS
    // DDRD = 0x00;
    // PIND = 0xFF;

    PORTD = (1 << PORTD2) | (1 << PORTD3) | (1 << PORTD4);

    // SALIDAS
    DDRC = 0xFF;
    PORTC = 0x00;

    EICRA |= (1 << ISC01); // set INT0 to trigger on ANY logic change
    EIMSK |= (1 << INT0);  // Turns on INT0
    // EIFR = 0x00;
    sei(); // turn on interrupts

    int ultimo_estado_arranque_suave = 1;
    int estado_arranque_suave = 1;
    int ultimo_estado_cambio_tiempo = 1;
    int estado_cambio_tiempo = 1;

    while (1)
    {
        if (flag)
        {
            if (encendido)
            {
                apagado_directo();
            }
            else
            {
                arranque_directo();
            }
        }

        estado_cambio_tiempo = (PIND & (1 << PIND3));
        if (estado_cambio_tiempo != ultimo_estado_cambio_tiempo)
        {
            if (!estado_cambio_tiempo)
            {
                _delay_ms(6);
                if (!estado_cambio_tiempo & !encendido)
                {
                    ciclar_tiempo();
                }
            }
            else
                _delay_ms(100);
        }
        ultimo_estado_cambio_tiempo = estado_cambio_tiempo;

        estado_arranque_suave = (PIND & (1 << PIND4));
        // Si el pin de arranque suave esta en bajo, se arranca suave.
        if (estado_arranque_suave != ultimo_estado_arranque_suave)
        {
            if (!estado_arranque_suave)
            {
                _delay_ms(6);
                arranque_suave();
            }
            else
                _delay_ms(100);
        }
        ultimo_estado_arranque_suave = estado_arranque_suave;
    }
    return 0;
}

void ciclar_tiempo()
{
    switch (tiempo)
    {
    case T1:
        tiempo = T2;
        PORTC |= (1 << PORTC5);
        _delay_ms(1000);
        PORTC &= ~(1 << PORTC5);

        if (flag)
            return;

        break;
    case T2:
        tiempo = T3;
        PORTC |= (1 << PORTC4);
        _delay_ms(1000);
        PORTC &= ~(1 << PORTC4);

        if (flag)
            return;

        break;
    case T3:
        tiempo = T4;
        PORTC |= (1 << PORTC3);
        _delay_ms(1000);
        PORTC &= ~(1 << PORTC3);

        if (flag)
            return;

        break;
    case T4:
        tiempo = T1;
        PORTC |= (1 << PORTC2);
        _delay_ms(1000);
        PORTC &= ~(1 << PORTC2);

        if (flag)
            return;

        break;
    }
}

void arranque_suave()
{
    if (encendido)
    {
        apagado_directo();
        return;
    }

    encendido = 1;
    //  incrementa 1ms en alto por cada iteracion, arrancando en 0
    for (unsigned int i = 0; i < T; i++)
    {
        // repetimos
        for (unsigned int j = 0; j < tiempo; j++)
        {
            // prendemos el led pwm
            PORTC |= (1 << PORTC0);

            // PORTC &= ~(1 << PORTC0);

            for (unsigned int k = 0; k < i; k++)
            {
                _delay_ms(1);
            }

            // apagamos el led pwm
            PORTC &= ~(1 << PORTC0);

            // PORTC |= (1 << PORTC0);

            for (unsigned int k = 0; k <= (T - i); k++)
            {
                _delay_ms(1);
            }
            if (flag)
            {
                return;
            }
        }

        if (i <= 4)
        {
            PORTC |= (1 << PORTC1);
        }
        else if (i <= 8)
        {
            PORTC |= (1 << PORTC2);
        }
        else if (i <= 12)
        {
            PORTC |= (1 << PORTC3);
        }
        else if (i <= 16)
        {
            PORTC |= (1 << PORTC4);
        }
    }
    PORTC |= (1 << PORTC5) | (1 << PORTC0);
}

void arranque_directo()
{
    encendido = 1;
    flag = 0;
    // PORTC |= (1 << PORTC0) | (1 << PORTC1) | (1 << PORTC2) | (1 << PORTC3) | (1 << PORTC4) | (1 << PORTC5);
    PORTC = 0xFF;
}

void apagado_directo()
{
    encendido = 0;
    flag = 0;
    // PORTC &= ~((1 << PORTC0) | (1 << PORTC1) | (1 << PORTC2) | (1 << PORTC3) | (1 << PORTC4) | (1 << PORTC5));
    PORTC = 0x00;
}
