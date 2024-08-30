const char* g_APP_NAME = "dr_manager";
/*version format: 3~4 alpha-num chars
 * abc[x]
 * for normal version, "a" and "b" and "c" are all single dec-digit, i.e. 0~9. no "x"
 * for test/debug version, "x" is used, alpha recommended.
 *
 * example:
 * A nomral released version: 033
 * A debug version: 033a
 * 
 * */
/* the 1st version digit 0 and 1 means the version manage LCD and tof by itself, and 2 and 3 mean lcd and tof are managed
 * by external entity.
 *
 * Note 1: do not change the order of the two definitions under macro.
 * Note 2: when update version number, please update the definition in both case so that both version can be of the latest.
 * */
#ifdef MANAGE_LCD_AND_TOF_HERE
const char * g_APP_VER_STR = "041a"; 
#else
const char * g_APP_VER_STR = "240b"; 
#endif

const unsigned char g_SW_VER_NUMBER = 1;
