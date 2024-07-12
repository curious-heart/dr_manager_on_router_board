/*
 * Please refer to the comments in version_def.c for the format of version number definition.
 * */
const char* g_gpio_processor_APP_NAME = "gpio_key_monitor";

/*
 * Please refer to version_def.c for the version string format rule.
 * Note 1: do not change the order of the two definitions under macro.
 * Note 2: when update version number, please update the definition in both case so that both version can be of the latest.
*/
#ifdef MANAGE_LCD_AND_TOF_HERE
const char * g_gpio_processor_APP_VER_STR = "023";
#else
const char * g_gpio_processor_APP_VER_STR = "223";
#endif
