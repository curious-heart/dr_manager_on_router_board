#ifndef _TOF_MEASURE_H_
#define _TOF_MEASURE_H_

#define TOF_CONTI_MEAS_MIN_COUNT 1
#define TOF_CONTI_MEAS_MAX_COUNT 5000 
#define TOF_CONTI_MEAS_MIN_INTERVAL 0.1
#define TOF_CONTI_MEAS_MAX_INTERVAL 10.0

int tof_open(const char* dev_name, unsigned char dev_addr);
int tof_close();
int tof_continuous_measure(unsigned short* result_buf, int count, float interval);

/*call tof_single_measure_prepare, then call tof_single_measure.*/
int tof_single_measure_prepare();
unsigned short tof_single_measure();

void tof_ref_register_test();

#endif
