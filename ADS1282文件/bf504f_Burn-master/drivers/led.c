#include "main.h"
#include "utils.h"
#include "led.h"


#define LED_TOGGLE_DELAY_MS		2	/* �������� �� ����� ������������ - ���������! */
#define LED_TOGGLE_MIN_PAUSE		500	/* ����� ����� ��������� ������� */
#define LED_TEST_MIN_PAUSE		100	/* ����� ����� ��������� ������� � ����� */

/*******************************************************************
 *   ����������� ����������
 *******************************************************************/

/** 
 * ��������� � �������� ���������� - ���������� � ������� 
 * ����� ��������� ������!!
 */
static struct {
    u32 timer;
    u8 leds[4];

#define  led1   	leds[0]
#define  led2   	leds[1]
#define  led3   	leds[2]
#define  led4   	leds[3]
#define  led_power	leds[2]
} led_state;


/*******************************************************************
 *   Function:    LED_init - �������� ������ ������ ����� CORE
 *******************************************************************/
#pragma section("FLASH_code")
void LED_init(void)
{
#if 0
    // � ��������� ��� ������ init
    pin_clr(LED1_PORT, LED1_PIN);	/* 1 �� ����� ������ PORTG */
    pin_clr(LED2_PORT, LED2_PIN);	/* 1 �� ����� ������ PORTG */
    pin_clr(LED3_PORT, LED3_PIN);	/* 1 �� ����� ������ PORTG */
    pin_clr(LED4_PORT, LED4_PIN);	/* 1 �� ����� ������ PORTG */
#endif
    led_state.led1 = LED_OFF_STATE;
    led_state.led2 = LED_OFF_STATE;
    led_state.led3 = LED_OFF_STATE;
    led_state.led4 = LED_OFF_STATE;
}


/************************************************************************************* 
 * �������� �� ������ 
 **************************************************************************************/
#pragma section("FLASH_code")
void LED_on(u8 led)
{
    pin_set(LEDS_PORT, led);
}

/************************************************************************************* 
 * ��������� ��-������ 
 **************************************************************************************/
#pragma section("FLASH_code")
void LED_off(u8 led)
{
    pin_clr(LEDS_PORT, led);	/* "0" �� ����� ����� */
}

/*************************************************************************************
 * "�������" ������� - ����������� ����������
 *************************************************************************************/
#pragma section("FLASH_code")
void LED_set_state(u8 l1, u8 l2, u8 l3, u8 l4)
{
    led_state.led1 = l1;
    led_state.led2 = l2;
    led_state.led3 = l3;
    led_state.led4 = l4;
}

/*************************************************************************************
 * "�������" ������� - ���������� ��������� ����
 *************************************************************************************/
void LED_get_state(u8 * l1, u8 * l2, u8 * l3, u8 * l4)
{
    *l1 = led_state.led1;
    *l2 = led_state.led2;
    *l3 = led_state.led3;
    *l4 = led_state.led4;
}

/*************************************************************************************
 * "�������" ������� - ���������� ��������� ������ �����
 *************************************************************************************/
#pragma section("FLASH_code")
u8 LED_get_power_led_state(void)
{
    return led_state.led_power;
}

/*************************************************************************************
 * "�������" ������� - ����������� ����������
 *************************************************************************************/
#pragma section("FLASH_code")
void LED_set_power_led_state(u8 led)
{
    led_state.led_power = led;
}

/************************************************************************************* 
 * ����������� ��-������ 
 **************************************************************************************/
#pragma section("FLASH_code")
void LED_toggle(u8 led)
{
    pin_set(LEDS_PORT, led);
    delay_ms(LED_TOGGLE_DELAY_MS);
    pin_clr(LEDS_PORT, led);
}


/*************************************************************************************
 * "�������" ������� - ��������� ����������, ��� ��� ����������� � led_state
 *************************************************************************************/
#pragma section("FLASH_code")
void LED_blink(void)
{
    u32 i, j;


    /* ������� ����� */
    if (led_state.leds[0] == LED_TEST_STATE || led_state.leds[1] == LED_TEST_STATE ||
	led_state.leds[2] == LED_TEST_STATE || led_state.leds[3] == LED_TEST_STATE) {
	LED_test();
    } else {

	/* ����� ���� ����� �� ������� ���� ��� � 1/2 ������� */
	j = get_msec_ticks() / LED_TOGGLE_MIN_PAUSE;

	if (j != led_state.timer) {
	    led_state.timer = j;

	    /* ������������ ��� ���� ����. i ����������� � ������� ����� ������ ��� ���� �����.  */
	    for (i = 0; i < NUM_LEDS; i++) {
		u8 led = NUM_LEDS - i - 1;
		if (led_state.leds[i] == LED_OFF_STATE) {
		    LED_off(led);	/* ���������, ���� ��� �������� */
		} else if (led_state.leds[i] == LED_ON_STATE) {
		    LED_on(led);	/* ��������, ���� ����� ��������� - "1" �� ����� ����� PORTG0 */
		} else if (led_state.leds[i] == LED_SLOW_STATE && led_state.timer % 4 == 0) {
		    LED_toggle(led);	/* ��� �����  ����������� 1 ��� � 2 ������� */
		} else if (led_state.leds[i] == LED_QUICK_STATE) {
		    LED_toggle(led);
		}
	    }
	}
    }
}


/*************************************************************************************
 *"�������" ������� - ������� ������
 * ����� ���� ����� �� ������� ���� ��� � 1/10 ������� � ��������� ���.
 * ����� ���� ����� �� ������� ���� ��� � 1/2 ������� �� ���� ���������
 *************************************************************************************/
#pragma section("FLASH_code")
void LED_test(void)
{
    u32 i;

    i = get_msec_ticks() / LED_TEST_MIN_PAUSE;

    if (i != led_state.timer) {
	led_state.timer = i;
	LED_all_off();
	LED_toggle(led_state.leds[0]++ % NUM_LEDS);	/* ��������, ���� ����� ��������� - "1" �� ����� ����� PORTG0 */
    }
}
