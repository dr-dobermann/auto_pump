/*
  Данный код превратит вашу Digispark в спящий таймер. Через каждые PERIOD
  секунд система подаёт 5 вольт на протяжении WORK секунд. Всё время,
  кроме переключения пина, система спит и потребляет 28 микроампер
  на плате Digispark с отрезанным светодиодом, стабилизатором
  и двумя диодами на шину USB. На голом камне будет работать ещё лучше!

  Подправил на старт после включения с задержкой
  Убрал некоторый мусор
  dr-dobermann, 2018.
*/
#include <avr/wdt.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>

#define adc_disable() (ADCSRA &= ~(1<<ADEN)) // disable ADC (before power-off)
// #define adc_enable()  (ADCSRA |=  (1<<ADEN)) // re-enable ADC
// http://alexgyver.ru/arduino/DigiDrivers.rar

const uint32_t 
    PERIOD      = 12*60*60, // пауза между включениями помпы в секундах
    START_DELAY = 20,       // задержка перед стартом
    WORK        = 1.5*60,   // время работы помпы в секундах
    MOS         = 1;        // пин мосфета помпы

uint32_t mainTimer = PERIOD - START_DELAY, 
         myTimer = 0;
         
boolean state = false;

void setup() {
    // все пины как входы, экономия энергии
    DDRB = 0;
    adc_disable();          // отключить АЦП (экономия энергии)

    wdt_reset();            // инициализация ватчдога
    wdt_enable(WDTO_1S);    // разрешаем ватчдог на 1 секунду
    // 15MS, 30MS, 60MS, 120MS, 250MS, 500MS, 1S, 2S, 4S, 8S

    WDTCR |= _BV(WDIE);     // разрешаем прерывания по ватчдогу. Иначе будет резет.
    sei();                  // разрешаем прерывания
    set_sleep_mode(SLEEP_MODE_PWR_DOWN); // максимальный сон
}

void loop() {
    
    mainTimer++;

    if (!state) {                           // если помпа не включена
        if (mainTimer - myTimer >= PERIOD) {   // таймер периода
            myTimer = mainTimer;                // сброс таймера
            state = true;                       // флаг на запуск
            pinMode(MOS, OUTPUT);               // пин как выход
            digitalWrite(MOS, HIGH);            // врубить
        }
    } 
    else {                                  // если помпа включена
        if (mainTimer - myTimer >= WORK) {     // таймер времени работы
            myTimer = mainTimer;                // сброс
            state = false;                      // флаг на выкл
            digitalWrite(MOS, LOW);             // вырубить
            pinMode(MOS, INPUT);                // пин как вход (экономия энергии)
        }
    }

    sleep_enable();   // разрешаем сон
    sleep_cpu();      // спать!
}

ISR (WDT_vect) {
    WDTCR |= _BV(WDIE); // разрешаем прерывания по ватчдогу. Иначе будет реcет.
}
