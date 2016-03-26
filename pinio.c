#include	"pinio.h"
#include	"delay.h"

static char ps_is_on = 0;

/// step/psu timeout
volatile uint8_t	psu_timeout = 0;

/** Initialise all I/O.

  This sets pins as input or output, appropriate for their usage.
*/
void pinio_init(void) {
  /// X Stepper.
  SET_OUTPUT(X_STEP_PIN); WRITE(X_STEP_PIN, 0);
  SET_OUTPUT(X_DIR_PIN); WRITE(X_DIR_PIN, 0);
  #ifdef X_MIN_PIN
    SET_INPUT(X_MIN_PIN);
    #ifndef X_MIN_PULLUP
      PULLUP_OFF(X_MIN_PIN);
    #else
      PULLUP_ON(X_MIN_PIN);
    #endif
  #endif
  #ifdef X_MAX_PIN
    SET_INPUT(X_MAX_PIN);
    #ifndef X_MAX_PULLUP
      PULLUP_OFF(X_MAX_PIN);
    #else
      PULLUP_ON(X_MAX_PIN);
    #endif
  #endif

  /// Y Stepper.
  SET_OUTPUT(Y_STEP_PIN); WRITE(Y_STEP_PIN, 0);
  SET_OUTPUT(Y_DIR_PIN); WRITE(Y_DIR_PIN, 0);
  #ifdef Y_MIN_PIN
    SET_INPUT(Y_MIN_PIN);
    #ifndef Y_MIN_PULLUP
      PULLUP_OFF(Y_MIN_PIN);
    #else
      PULLUP_ON(Y_MIN_PIN);
    #endif
  #endif
  #ifdef Y_MAX_PIN
    SET_INPUT(Y_MAX_PIN);
    #ifndef Y_MAX_PULLUP
      PULLUP_OFF(Y_MAX_PIN);
    #else
      PULLUP_ON(Y_MAX_PIN);
    #endif
  #endif

  /// Z Stepper.
  #if defined Z_STEP_PIN && defined Z_DIR_PIN
    SET_OUTPUT(Z_STEP_PIN); WRITE(Z_STEP_PIN, 0);
    SET_OUTPUT(Z_DIR_PIN); WRITE(Z_DIR_PIN, 0);
  #endif
  #ifdef Z_MIN_PIN
    SET_INPUT(Z_MIN_PIN);
    #ifndef Z_MIN_PULLUP
      PULLUP_OFF(Z_MIN_PIN);
    #else
      PULLUP_ON(Z_MIN_PIN);
    #endif
  #endif
  #ifdef Z_MAX_PIN
    SET_INPUT(Z_MAX_PIN);
    #ifndef Z_MAX_PULLUP
      PULLUP_OFF(Z_MAX_PIN);
    #else
      PULLUP_ON(Z_MAX_PIN);
    #endif
  #endif

  #if defined E_STEP_PIN && defined E_DIR_PIN
    SET_OUTPUT(E_STEP_PIN); WRITE(E_STEP_PIN, 0);
    SET_OUTPUT(E_DIR_PIN); WRITE(E_DIR_PIN, 0);
  #endif

  /// Common Stepper Enable.
  #ifdef STEPPER_ENABLE_PIN
    SET_OUTPUT(STEPPER_ENABLE_PIN);
    #ifdef STEPPER_INVERT_ENABLE
      WRITE(STEPPER_ENABLE_PIN, 0);
    #else
      WRITE(STEPPER_ENABLE_PIN, 1);
    #endif
  #endif

  /// X Stepper Enable.
  #ifdef X_ENABLE_PIN
    SET_OUTPUT(X_ENABLE_PIN);
    #ifdef X_INVERT_ENABLE
      WRITE(X_ENABLE_PIN, 0);
    #else
      WRITE(X_ENABLE_PIN, 1);
    #endif
  #endif

  /// Y Stepper Enable.
  #ifdef Y_ENABLE_PIN
    SET_OUTPUT(Y_ENABLE_PIN);
    #ifdef Y_INVERT_ENABLE
      WRITE(Y_ENABLE_PIN, 0);
    #else
      WRITE(Y_ENABLE_PIN, 1);
    #endif
  #endif

  /// Z Stepper Enable.
  #ifdef Z_ENABLE_PIN
    SET_OUTPUT(Z_ENABLE_PIN);
    #ifdef Z_INVERT_ENABLE
      WRITE(Z_ENABLE_PIN, 0);
    #else
      WRITE(Z_ENABLE_PIN, 1);
    #endif
  #endif

  /// E Stepper Enable.
  #ifdef E_ENABLE_PIN
    SET_OUTPUT(E_ENABLE_PIN);
    #ifdef E_INVERT_ENABLE
      WRITE(E_ENABLE_PIN, 0);
    #else
      WRITE(E_ENABLE_PIN, 1);
    #endif
  #endif

  #ifdef  STEPPER_ENABLE_PIN
    power_off();
  #endif

  #ifdef DEBUG_LED_PIN
    SET_OUTPUT(DEBUG_LED_PIN);
    WRITE(DEBUG_LED_PIN, 0);
  #endif
}

void power_on() {

	if (ps_is_on == 0) {
		#ifdef	PS_ON_PIN
      SET_OUTPUT(PS_ON_PIN);
			WRITE(PS_ON_PIN, 0);
			delay_ms(500);
		#endif
    #ifdef PS_MOSFET_PIN
      WRITE(PS_MOSFET_PIN, 1);
      delay_ms(10);
    #endif
		ps_is_on = 1;
	}

	psu_timeout = 0;
}

void power_off() {

	stepper_disable();
	x_disable();
	y_disable();
	z_disable();
	e_disable();

	#ifdef	PS_ON_PIN
		SET_INPUT(PS_ON_PIN);
    PULLUP_OFF(PS_ON_PIN);
	#endif

  #ifdef PS_MOSFET_PIN
    WRITE(PS_MOSFET_PIN, 0);
  #endif

	ps_is_on = 0;
}
