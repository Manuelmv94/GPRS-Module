#include <hidef.h> /* for EnableInterrupts macro */
#include "derivative.h" /* include peripheral declarations */
#include "GPRSManager.h"
#include "types.h"





void main(void) {
  EnableInterrupts;
  /* include your code here */

  GPRSManager_Init();
  //GPRSManager_sendMessage("6673903075","Hola sinsel:)",13);

  for(;;) {
    __RESET_WATCHDOG();	/* feeds the dog */
    bmessageready();
	  
  } /* loop forever */
  /* please make sure that you never leave main */
}
