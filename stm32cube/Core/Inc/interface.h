/*
 * interface.h
 *
 *  Created on: Apr 10, 2022
 *      Author: UMK
 */

#ifndef INC_INTERFACE_H_
#define INC_INTERFACE_H_

#define CMD_SEP ';'

typedef struct{
    void* p;
    char* type;
} pointer;

typedef struct{
    int is;
} ison;

typedef struct{
    int tabsize;
    int tabcount;
    int tabpos;
    double* ptab[2];
}mestab;

typedef struct {
    double min;
    double max;
    double val;
    char* cmdset;
    ison tabon;
    mestab mes;
} value;

typedef struct {
	value raw;
    value volt;
    value avr;
    value coron;
    value corfactor;
}sadcchannel;

typedef struct{
    sadcchannel ch1;
    sadcchannel ch2;
} sadc;

typedef struct {
	value raw;
    value volt;
}sdacchannel;

typedef struct{
	sdacchannel ch1;
} sdac;



typedef struct {
    sadc adc;
    sdac dac;
} parameters;

pointer getPointer(pointer,char * );
void initInterface(void);
void setParam(value*, double);


#endif /* INC_INTERFACE_H_ */
