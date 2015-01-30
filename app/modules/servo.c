// Module for RC servo interfacing by Milan Spacek

//#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "platform.h"
#include "auxmods.h"
#include "lrotable.h"

#include "c_types.h"

#define DIRECT_MODE_OUTPUT(pin)	 platform_gpio_mode(pin,PLATFORM_GPIO_OUTPUT,PLATFORM_GPIO_PULLUP)
#define DIRECT_WRITE_LOW(pin)    (GPIO_OUTPUT_SET(GPIO_ID_PIN(pin_num[pin]), 0))
#define DIRECT_WRITE_HIGH(pin)   (GPIO_OUTPUT_SET(GPIO_ID_PIN(pin_num[pin]), 1))




static os_timer_t servo_timer;

static unsigned servo_pins[4] = {0,0,0,0};
static unsigned servo_pulse_lenght[4] = {1500,1500,1500,1500};
static unsigned servo_loop_counter = 0;



/*
char debug_buffer [128];
void debug(char *dataPtr)
{
    while ( *dataPtr )
    {
    	platform_uart_send( 0, *dataPtr++ );
    }
}
*/




servo_timer_tick(void) // servo timer function, called every 5ms for one from 4 servos
{
	if (servo_pins[servo_loop_counter] != 0)
	{
		DIRECT_WRITE_HIGH(servo_pins[servo_loop_counter]);	// drive output high
		os_delay_us(servo_pulse_lenght[servo_loop_counter]);
		DIRECT_WRITE_LOW(servo_pins[servo_loop_counter]);
	}

	servo_loop_counter++;
	if(servo_loop_counter >= 4)
		servo_loop_counter = 0;
}



// Lua: setup( id, pin, default_pulse_lenght )
static int servo_setup( lua_State* L )
{
  unsigned id, pin, default_pulse;

  id = luaL_checkinteger( L, 1 );
  if (id > 3)
        return luaL_error( L, "wrong id, must be 0-3" );

  pin = luaL_checkinteger( L, 2 );
  MOD_CHECK_ID( gpio, pin );

  default_pulse = luaL_checkinteger( L, 3 );
  if (default_pulse < 500 || default_pulse > 2500 )
        return luaL_error( L, "wrong pulse length, must be 500-2500" );

  //os_sprintf(debug_buffer, "id=%d    pin=%d    time=%d",id,pin,default_pulse);
  //debug(debug_buffer);

  if (id >= 0 && id <= 3)
  {
	  servo_pins[id] = pin;
	  servo_pulse_lenght[id] = default_pulse;

	  if (servo_pins[id] != 0)
	  {
		  DIRECT_WRITE_LOW(pin);
		  DIRECT_MODE_OUTPUT(pin);
	  }

	  os_timer_disarm(&servo_timer); // dis_arm the timer
	  os_timer_setfn(&servo_timer, (os_timer_func_t *)servo_timer_tick, NULL); // set the timer function, dot get os_timer_func_t to force function convert
	  os_timer_arm(&servo_timer, 5, 1); // arm the timer every 5ms and repeat
  }

  return 1;
}




// Lua: position( id, pulse_lenght )
static int servo_position( lua_State* L )
{
  unsigned id, pulse_lenght;

  id = luaL_checkinteger( L, 1 );
  if (id > 3)
        return luaL_error( L, "wrong id, must be 0-3" );

  pulse_lenght = luaL_checkinteger( L, 2 );
  if (pulse_lenght < 500 || pulse_lenght > 2500 )
        return luaL_error( L, "wrong pulse length, must be 500-2500" );

  //os_sprintf(debug_buffer, "id=%d   time=%d",id,pulse_lenght);
  //debug(debug_buffer);

  if (id >= 0 && id <= 3)
  {
	  servo_pulse_lenght[id] = pulse_lenght;
  }

  return 1;
}




// Lua: stop()
static int servo_stop( lua_State* L )
{
  os_timer_disarm(&servo_timer); // dis_arm the timer
  return 1;
}






// Module function map
#define MIN_OPT_LEVEL 2
#include "lrodefs.h"
const LUA_REG_TYPE servo_map[] =
{
  { LSTRKEY( "setup" ),  LFUNCVAL( servo_setup ) },
  { LSTRKEY( "position" ), LFUNCVAL( servo_position ) },
  { LSTRKEY( "stop" ), LFUNCVAL( servo_stop ) },
#if LUA_OPTIMIZE_MEMORY > 0

#endif
  { LNILKEY, LNILVAL }
};

LUALIB_API int luaopen_servo( lua_State *L )
{
#if LUA_OPTIMIZE_MEMORY > 0
  return 0;
#else // #if LUA_OPTIMIZE_MEMORY > 0
  luaL_register( L, AUXLIB_SERVO, servo_map );
  // Add constants

  return 1;
#endif // #if LUA_OPTIMIZE_MEMORY > 0  
}
