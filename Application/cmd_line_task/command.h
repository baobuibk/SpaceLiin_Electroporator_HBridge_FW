#ifndef COMMAND_H_
#define COMMAND_H_

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Include ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
#include <stdint.h>

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Defines ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Enum ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Struct ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Class ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Types ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Variables ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Prototype ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* :::::::::: CMD Line Task Init :::::::: */
int CMD_LINE_TEST(int argc, char *argv[]);

int CMD_PULSE_COUNT(int argc, char *argv[]);
int CMD_PULSE_DELAY(int argc, char *argv[]);
int CMD_PULSE_HV(int argc, char *argv[]);
int CMD_PULSE_LV(int argc, char *argv[]);
int CMD_PULSE_CONTROL(int argc, char *argv[]);

int CMD_RELAY_SET(int argc, char *argv[]);
int CMD_RELAY_CONTROL(int argc, char *argv[]);

int CMD_CHANNEL_SET(int argc, char *argv[]);
int CMD_CHANNEL_CONTROL(int argc, char *argv[]);

void decode_ls_relay(uint8_t cuvette_code);
void decode_hs_relay(uint8_t cuvette_code);
int CMD_PRINT_SENSOR(int argc, char *argv[]);
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ End of the program ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#endif /* COMMAND_H_ */
