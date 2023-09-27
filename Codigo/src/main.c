#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define F_CPU 16000000L

#define T 20 // ms, pasos definidos

enum numero_pasos
{
    T1 = 12,
    T2 = 20,
    T3 = 27,
    T4 = 35
};

int tiempo = T1;
volatile int flag = 0;
int encendido = 0;

void arranque_suave();
void arranque_directo();
void apagado_directo();
void ciclar_tiempo();

void prender(int);

ISR(INT0_vect)
{
    flag = 1;
}

int main(void)
{
    // ENTRADAS
    DDRD = 0x00;
    PORTD = 0xFF;

    // SALIDAS
    DDRC = 0xFF;
    PORTC = 0xFF;

    EICRA |= (1 << ISC01); // set INT0 to trigger on ANY logic change
    EIMSK |= (1 << INT0);  // Turns on INT0
    EIFR = 0x00;
    // PCICR |= (1 << PCIE1);
    // PCIFR |= (1 << PCIF1);
    // PCMSK1 |= (1 << PCINT13);

    sei(); // turn on interrupts

    while (1)
    {
        if (flag)
        {
            _delay_ms(6);
            if (flag)
            {
                if (!encendido)
                {
                    arranque_directo();
                    flag = 0;
                }
                else
                {
                    apagado_directo();
                    flag = 0;
                }
            }
        }

        if (!(PIND & (1 << PIND3)))
        {
            _delay_ms(6);

            if (!(PIND & (1 << PIND3)))
            {
                ciclar_tiempo();
            }
            else
            {
                return 0;
            }
        }

        // Si el pin de arranque suave esta en bajo, se arranca suave.
        if (!(PIND & (1 << PIND4)))
        {
            _delay_ms(6);

            if (!(PIND & (1 << PIND4)))
            {
                arranque_suave();
            }
            else
            {
                return 0;
            }
        }
    }

    return 0;
}

void ciclar_tiempo()
{
    switch (tiempo)
    {
    case T1:

        PORTC |= (1 << PORTC5);
        _delay_ms(1000);
        PORTC &= ~(1 << PORTC5);

        tiempo = T2;
        break;
    case T2:
        PORTC |= (1 << PORTC4);
        _delay_ms(1000);
        PORTC &= ~(1 << PORTC4);

        tiempo = T3;
        break;
    case T3:
        PORTC |= (1 << PORTC3);
        _delay_ms(1000);
        PORTC &= ~(1 << PORTC3);

        tiempo = T4;
        break;
    }
}

void arranque_suave()
{
    encendido = 1;
    // incrementa 1ms en alto por cada iteracion, arrancando en 0
    for (unsigned int i = 0; i < T; i++)
    {
        // repetimos
        for (unsigned int j = 0; j < tiempo; j++)
        {
            if (!flag)
            {
                // prendemos el led pwm
                PORTC |= (1 << PORTC0);

                for (unsigned int k = 0; k < i; k++)
                {
                    _delay_ms(1);
                }

                // apagamos el led pwm
                PORTC &= ~(1 << PORTC0);

                for (unsigned int k = 0; k <= (T - i); k++)
                {
                    _delay_ms(1);
                }
            }
            else
            {
                return;
            }
        }
        prender(i);
    }
    PORTC |= (1 << PORTC0);
}

void prender(int i)
{
    if (i <= 4)
    {
        PORTC |= (1 << PORTC1);
    }
    else if (i <= 8)
    {
        PORTC |= (1 << PORTC1) | (1 << PORTC2);
    }
    else if (i <= 12)
    {
        PORTC |= (1 << PORTC1) | (1 << PORTC2) | (1 << PORTC3);
    }
    else if (i <= 16)
    {
        PORTC |= (1 << PORTC1) | (1 << PORTC2) | (1 << PORTC3) | (1 << PORTC4);
    }
    else
    {
        // PORTC |= (1 << PORTC1) | (1 << PORTC2) | (1 << PORTC3) | (1 << PORTC4) | (1 << PORTC5);
        PORTC = 0xFF;
    }
}

void arranque_directo()
{
    encendido = 1;
    // PORTC |= (1 << PORTC0) | (1 << PORTC1) | (1 << PORTC2) | (1 << PORTC3) | (1 << PORTC4) | (1 << PORTC5);
    PORTC = 0xFF;
}

void apagado_directo()
{
    encendido = 0;
    // PORTC &= ~((1 << PORTC0) | (1 << PORTC1) | (1 << PORTC2) | (1 << PORTC3) | (1 << PORTC4) | (1 << PORTC5));
    PORTC = 0x00;
}
