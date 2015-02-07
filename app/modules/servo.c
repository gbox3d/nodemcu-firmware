// Module for RC servo interfacing by Milan Spacek

//#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "platform.h"
#include "auxmods.h"
#include "lrotable.h"

#include "c_types.h"

#define max_servos   6

#define DIRECT_MODE_OUTPUT(pin)   platform_gpio_mode(pin,PLATFORM_GPIO_OUTPUT,PLATFORM_GPIO_PULLUP)
#define DIRECT_WRITE_LOW(pin)    (GPIO_OUTPUT_SET(GPIO_ID_PIN(pin_num[pin]), 0))
#define DIRECT_WRITE_HIGH(pin)   (GPIO_OUTPUT_SET(GPIO_ID_PIN(pin_num[pin]), 1))

static os_timer_t servo_timer;
unsigned char timer_running = 0;
unsigned char active_servos = 0;


static signed char servo_pin[max_servos] = {-1,-1,-1,-1,-1,-1};
static int servo_time[max_servos] = {0,0,0,0,0,0};

static int servo_tmp_time[max_servos] = {0,0,0,0,0,0};
static int servo_delay_time[max_servos] = {0,0,0,0,0,0};
static signed char servo_pin_sorted[max_servos] = {-1,-1,-1,-1,-1,-1};

unsigned char need_sort = 0;



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



int sort_pulse_time(void)
{
   int c,d,swap,pin_swap,n,delay_sum;

    // reset sorting flag
    need_sort = 0;

    // count active servos
	active_servos = 0;

    // fill variables for calculation
    for ( c = 0 ; c < max_servos ; c++ )
    {
		 // fill only active pins and servos
		 if (servo_pin[c] >= 0)
		 {
			servo_pin_sorted[active_servos] = servo_pin[c];
     		servo_tmp_time[active_servos] = servo_time[c];
     		active_servos++;
		 }
    }

    // if only 1 servo is active, simple copy tmp_time to delay_time
    if (active_servos == 1)
    {
    	servo_delay_time[0] = servo_tmp_time[0];
    }
    // if more than 1 servo is active, sorting is needed.
    else if (active_servos > 1)
    {
		// set max number of servos for sorting
		n = active_servos;

		// sort servos by servo pulse time and sort pin numbers.
		for (c = 0 ; c < ( n - 1 ); c++)
		{
			for (d = 0 ; d < n - c - 1; d++)
			{
			  if (servo_tmp_time[d] > servo_tmp_time[d+1])
			  {
				// sort servo time;
				swap                = servo_tmp_time[d];
				servo_tmp_time[d]   = servo_tmp_time[d+1];
				servo_tmp_time[d+1] = swap;

				// hold servo pins sorted same as time
				pin_swap              = servo_pin_sorted[d];
				servo_pin_sorted[d]   = servo_pin_sorted[d+1];
				servo_pin_sorted[d+1] = pin_swap;
			  }
			}
		}

		// calculate delay offset
		delay_sum = 0;
		for ( c = 0 ; c < n ; c++ )
		{
			servo_delay_time[c] = servo_tmp_time[c]-delay_sum;
			delay_sum += servo_delay_time[c];
		}

	}

/*
    for ( c = 0 ; c < max_servos ; c++ )
    {
       os_sprintf(debug_buffer,"pt:%d  delay:%d  pin:%d  active:%d\r\n", servo_tmp_time[c], servo_delay_time[c],servo_pin_sorted[c],active_servos);
       debug(debug_buffer);
    }
*/

    return 0;
}



servo_timer_tick(void) // servo timer function
{
    unsigned char i;
    os_timer_disarm(&servo_timer); // dis_arm the timer
    os_timer_setfn(&servo_timer, (os_timer_func_t *)servo_timer_tick, NULL); // set the timer function, dot get os_timer_func_t to force function convert
    os_timer_arm(&servo_timer, 20, 1); // arm the timer every 20ms and repeat

    if (need_sort != 0) { sort_pulse_time(); }


	// if only 1 servo is active
    if (active_servos == 1)
    {
    	DIRECT_WRITE_HIGH(servo_pin_sorted[0]);
    	os_delay_us(servo_delay_time[0]);
    	DIRECT_WRITE_LOW(servo_pin_sorted[0]);
    }
    // if more than 1 servo is active, loop for all active servo is needed
    else if (active_servos > 1)
    {
		for ( i = 0 ; i < active_servos ; i++ )
		{
			if (servo_pin_sorted[i] >= 0) {DIRECT_WRITE_HIGH(servo_pin_sorted[i]);}
		}

		for ( i = 0 ; i < active_servos ; i++ )
		{
			if (servo_delay_time[i] > 0) {os_delay_us(servo_delay_time[i]);}
			if (servo_pin_sorted[i] >= 0) {DIRECT_WRITE_LOW(servo_pin_sorted[i]);}
		}
    }

}




// Lua: setup( id, pin, default_pulse_lenght )
static int servo_setup( lua_State* L )
{
  unsigned id, pin, default_pulse;

  id = luaL_checkinteger( L, 1 );
  if (id >= max_servos)
        return luaL_error( L, "wrong id, must be 0-5" );

  pin = luaL_checkinteger( L, 2 );
  MOD_CHECK_ID( gpio, pin );

  default_pulse = luaL_checkinteger( L, 3 );
  if (default_pulse < 500 || default_pulse > 2500 )
        return luaL_error( L, "wrong pulse length, must be 500-2500" );

  //os_sprintf(debug_buffer, "id=%d    pin=%d    time=%d",id,pin,default_pulse);
  //debug(debug_buffer);

  if (id >= 0 && id < max_servos)
  {
     servo_pin[id] = pin;
     servo_time[id] = default_pulse;

     if (servo_pin[id] >= 0)
     {
        DIRECT_WRITE_LOW(pin);
        DIRECT_MODE_OUTPUT(pin);
     }

     need_sort = 1;

     if (timer_running != 1)
     {
      os_timer_disarm(&servo_timer); // dis_arm the timer
      os_timer_setfn(&servo_timer, (os_timer_func_t *)servo_timer_tick, NULL); // set the timer function, dot get os_timer_func_t to force function convert
      os_timer_arm(&servo_timer, 20, 1); // arm the timer every 5ms and repeat
      timer_running = 1;
     }
  }

  return 1;
}



// Lua: position( id, pulse_lenght )
static int servo_position( lua_State* L )
{
  unsigned id, pulse_lenght;

  id = luaL_checkinteger( L, 1 );
  if (id >= max_servos)
        return luaL_error( L, "wrong id" );

  pulse_lenght = luaL_checkinteger( L, 2 );
  if (pulse_lenght < 500 || pulse_lenght > 2500 )
        return luaL_error( L, "wrong pulse length" );

  //os_sprintf(debug_buffer, "id=%d   time=%d",id,pulse_lenght);
  //debug(debug_buffer);

  if (id >= 0 && id < max_servos)
  {
     servo_time[id] = pulse_lenght;
     need_sort = 1;
  }

  return 1;
}




// Lua: stop(id)
static int servo_stop( lua_State* L )
{
   unsigned id,i,tmp_active;

   id = luaL_checkinteger( L, 1 );
   if (id >= max_servos)
      return luaL_error( L, "wrong id" );

   if (id >= 0 && id < max_servos)
   {
     servo_pin[id] = -1;
     servo_time[id] = 0;
     need_sort = 1;
   }

   // check if any servo is active, if no stop the timer
   tmp_active = 0;
   for ( i = 0 ; i < max_servos ; i++ )
   {
      if (servo_pin[i] >= 0) {tmp_active += 1;}
   }

   // if no active servo found, stop the timer
   if (tmp_active == 0)
   {
      os_timer_disarm(&servo_timer); // dis_arm the timer
      timer_running = 0;
   }

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
