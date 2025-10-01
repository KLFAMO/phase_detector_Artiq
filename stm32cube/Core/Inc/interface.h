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
    value div;
    value al_sw;
} divider;

typedef struct {
    double version;
    value ver;
    value mode;
    divider osc;
    divider ref;
    value save;
    value load;
} parameters;

pointer getPointer(pointer,char * );
void initInterface(void);
void setParam(value*, double);


#endif /* INC_INTERFACE_H_ */
