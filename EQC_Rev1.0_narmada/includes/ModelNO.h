#ifndef MODELNO_H_
#define MODELNO_H_

// This file contains all Model No defs.  
// No need to include this file seperately. It is included in Projects.h

//  Default factory model
#define DEFAULT_1V_1A_MODEL			255
#define RATED_VOLTAGE_1_1			1
#define RATED_CURRENT_1_1			1

// Max Current/Max voltage across all models in 3U family
#define MAX_RATED_VOLTAGE_FOR_EQC_MODEL_TIMES_100						60000	// 600V
#define MAX_RATED_CURRENT_FOR_EQC_MODEL_TIMES_100						70000	// 700A


//	Model_10_700
#define	MODEL_NO_10_700			1
#define	MIN_OVP_10_700				2

#define	RATED_VOLTAGE_10_700_TIMES100									1000
#define	RATED_CURRENT_10_700_TIMES100									70000
#define MODEL_10_700_CURRENT_RISE_TIME									500	// 500ms

//	Model_20_500
#define	MODEL_NO_20_500			2
#define	MIN_OVP_20_500				5

#define	RATED_VOLTAGE_20_500_TIMES100									2000
#define	RATED_CURRENT_20_500_TIMES100									50000
#define MODEL_20_500_CURRENT_RISE_TIME									500	// 500ms

//	Model_30_334
#define	MODEL_NO_30_334			3
#define	MIN_OVP_30_334				10

#define	RATED_VOLTAGE_30_334_TIMES100									3000
#define	RATED_CURRENT_30_334_TIMES100									33400
#define MODEL_30_334_CURRENT_RISE_TIME									500	// 500ms

//	Model_40_250
#define	MODEL_NO_40_250			4
#define	MIN_OVP_40_250				10

#define	RATED_VOLTAGE_40_250_TIMES100									4000
#define	RATED_CURRENT_40_250_TIMES100									25000
#define MODEL_40_250_CURRENT_RISE_TIME									500	// 500ms

//	Model_50_200
#define	MODEL_NO_50_200			5
#define	MIN_OVP_50_200			10

#define	RATED_VOLTAGE_50_200_TIMES100									5000
#define	RATED_CURRENT_50_200_TIMES100									20000
#define MODEL_50_200_CURRENT_RISE_TIME									500	// 500ms

//	Model_60_167
#define	MODEL_NO_60_167			6
#define	MIN_OVP_60_167			10

#define	RATED_VOLTAGE_60_167_TIMES100									6000
#define	RATED_CURRENT_60_167_TIMES100									16700
#define MODEL_60_167_CURRENT_RISE_TIME									500	// 500ms

//	Model_80_125
#define	MODEL_NO_80_125			7
#define	MIN_OVP_80_125			10

#define	RATED_VOLTAGE_80_125_TIMES100									8000
#define	RATED_CURRENT_80_125_TIMES100									12500
#define MODEL_80_125_CURRENT_RISE_TIME									500	// 500ms

//	Model_100_100
#define	MODEL_NO_100_100			8
#define	MIN_OVP_100_100				10

#define	RATED_VOLTAGE_100_100_TIMES100									10000
#define	RATED_CURRENT_100_100_TIMES100									10000
#define MODEL_100_100_CURRENT_RISE_TIME									500	// 500ms

//	Model_150_67
#define	MODEL_NO_150_67			9
#define	MIN_OVP_150_67				10

#define	RATED_VOLTAGE_150_67_TIMES100									15000
#define	RATED_CURRENT_150_67_TIMES100									6700
#define MODEL_150_67_CURRENT_RISE_TIME									500	// 500ms

//	Model_300_34
#define	MODEL_NO_300_34			10
#define	MIN_OVP_300_34				20

#define	RATED_VOLTAGE_300_34_TIMES100									30000
#define	RATED_CURRENT_300_34_TIMES100									3400
#define MODEL_300_34_CURRENT_RISE_TIME									500	// 500ms

//	Model_400_25
#define	MODEL_NO_400_25				11
#define	MIN_OVP_400_25				35

#define	RATED_VOLTAGE_400_25_TIMES100									40000
#define	RATED_CURRENT_400_25_TIMES100									3750
#define MODEL_400_25_CURRENT_RISE_TIME									500	// 500ms

//	Model_600_17
#define	MODEL_NO_600_17				12
#define	MIN_OVP_600_17				35

#define	RATED_VOLTAGE_600_17_TIMES100									60000
#define	RATED_CURRENT_600_17_TIMES100									1700
#define MODEL_600_17_CURRENT_RISE_TIME									500	// 500ms
/*-------------------------	CUSTOM MODELS -----------------------------------------*/
// Custom models will have model numbers incrementing from 13. (We can go upto only 31 because CAN Model IDs vary from 0-31)
// For creating a new custom Model we will allot the next available model number and enter details
// for it below this section. INCLUDE SENT DATE, CUSTOMER NAME AND SERIAL NUMBER IN THE COMMENTS TO KEEP TRACK OF THE UNITS.


// Date Shipped:			7/11/2014
// Customer:				SUNEDISON
// Serial No (Nos.):
//	Model_68_150
#define	MODEL_NO_450_23				13
#define	MIN_OVP_450_23				37.5

#define	RATED_VOLTAGE_450_23_TIMES100									45000
#define	RATED_CURRENT_450_23_TIMES100									2300
#define MODEL_450_23_CURRENT_RISE_TIME									500	// 500ms

//	Model_68_150
#define	MODEL_NO_68_150				14
#define	MIN_OVP_68_150				5

#define	RATED_VOLTAGE_68_150_TIMES100									6800
#define	RATED_CURRENT_68_150_TIMES100									15000
#define MODEL_68_150_CURRENT_RISE_TIME									500	// 500ms

// Model_48_27
#define	MODEL_NO_48_27				15
#define	MIN_OVP_48_27				5

#define	RATED_VOLTAGE_48_27_TIMES100									4800
#define	RATED_CURRENT_48_27_TIMES100									2700
#define MODEL_48_27_CURRENT_RISE_TIME									500	// 500ms


#endif /*MODELNO_H_*/
