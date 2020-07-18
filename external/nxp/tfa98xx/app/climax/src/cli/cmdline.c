/*
 *Copyright 2014 NXP Semiconductors
 *
 *Licensed under the Apache License, Version 2.0 (the "License");
 *you may not use this file except in compliance with the License.
 *You may obtain a copy of the License at
 *
 *http://www.apache.org/licenses/LICENSE-2.0
 *
 *Unless required by applicable law or agreed to in writing, software
 *distributed under the License is distributed on an "AS IS" BASIS,
 *WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *See the License for the specific language governing permissions and
 *limitations under the License.
 */



/* If we use autoconf.  */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dbgprint.h"

#ifndef FIX_UNUSED
#define FIX_UNUSED(X) (void) (X) /* avoid warnings for unused params */
#endif

#if defined(WIN32) || defined(_X64)
#include <cmdlineWin.h>
#else
#include <getopt.h>
#endif
#include "cmdline.h"

const char *gengetopt_args_info_purpose = "";

const char *gengetopt_args_info_usage = "Usage: climax [OPTIONS]...";

const char *gengetopt_args_info_versiontext = "";

const char *gengetopt_args_info_description = "Command Line Interface for MAXimus\nNXP SemiConductors Smart Amplifier: TFA98xx";

const char *gengetopt_args_info_full_help[] = {
  "  -h, --help                    Print help and exit",
  "      --full-help               Print help, including hidden options, and exit",
  "      --version                 Print version and exit",
  "\n Device operation options:",
  "      --start                   device power up and start",
  "      --stop                    device stop and power down",
  "  -v, --volume=step             set volume step",
  "  -P, --profile[=profilenr]     set the (new) profile for current operation",
  " Generic options:",
  "  -d, --device=/dev/i2c-x|/dev/ttyUSBx|host:port|dummy\n                                target name for the interface: i2c, serial,\n                                  socket, i2c dummy",
  "      --resetMtpEx              reset MtpEx register to do re-calibration",
  "  -R, --reset                   initialize I2C registers and set ACS. After the\n                                  command, the state is same power on reset.",
  " Speaker boost options:",
  "  -a, --calibrate[=once|always] do calibration with loaded speaker file,\n                                  --output returns updated speaker file\n                                  (default=`always')",
  "  -A, --calshow                 show calibration impedance value",
  "  -p, --params=parameter file name\n                                write the params file directly to the device;\n                                  depending on header type: patch, speaker,\n                                  preset, config, drc, eq ",
  "      --re0[=re0]               set specified re0 or read the current re0",
  "  -D, --dsp[=hex]               DSP get speakerboost params, use --count to set\n                                  bytecount  (default=`0x80')",
  "  -s, --save=filename.<type>.bin\n                                write settings to binary file without header.\n                                  the file type extension must be specified as\n                                  .eq.bin, .speaker.bin, etc",
  " Diagnostics and test options:",
  "      --currentprof=profilenr   set the currently (loaded) runing profile to\n                                  force transition to new profile. This options\n                                  should be used with profile option",
  "      --tone=on/off             Set the tone detection off or on",
  "  -V, --versions                print versions and chip rev",
  "  -r, --register=offset[,value] read tfa register, write if extra arg given",
  "  -w, --regwrite=hex            write value for register",
  "      --dump                    dump all defined registers",
  "      --pin=pin                 control devkit signal pin",
  "      --diag[=testnr]           run all tests, or single if testnr; optional\n                                  extra argument: [i2s|dsp|sb|pins]\n                                  (default=`0')",
  "  -x, --xmem=offset[,value]     access (read/write) tfa xmem",
  " Live data options:",
  "      --dumpmodel[=x|z]         dump current speakermodel impedance=z or\n                                  excursion=x  (default=`z')",
  "      --record[=msInterval]     record speaker state info via I2C and display\n                                  (default=`55')",
  "      --count=cycles            number of read cycles to execute, 0 means\n                                  forever",
  "  -o, --output=filename         specify the output file for binary speaker\n                                  state info records, default=stdout",
  "      --logger[=sInterval]      start datalogger, recording <count> state info\n                                  lines and binary Z/Xmodels  (default=`2')",
  " Container file handling options:",
  "      --ini2cnt=filename.ini    Generate the container file from an ini file\n                                  <this>.ini to <this>.cnt",
  "      --bin2hdr=file.<type>     Generate a file with header from input binary\n                                  file.<type> [customer] [application] [type].\n                                  Original file is backed up as file.<type>.old\n                                  file",
  "  -m, --maximus=maX_type        provide the maximus device type  (default=`1')",
  " Generic options:",
  "  -l, --load=filename.cnt       read parameter settings from container file",
  "      --splitparms              save parameters of the loaded container file to\n                                  seperate parameter files",
  "      --server[=port]           run as server (for Linux only, default=`9887')\n                                  (default=`9887')",
  "      --client[=port]           run as client (for Linux only, default=`9887')\n                                  (default=`9887')",
  "      --slave=i2c address       override hardcoded I2C slave address",
  "  -L, --loop=count              loop the operation [0=forever]",
  "  -b, --verbose[=mask]          Enable verbose\n                                  (mask=timing|i2cserver|socket|scribo)",
  "  -t, --trace[=filename]        Enable I2C transaction tracing to stdout/file",
  "  -q, --quiet                   Suppress printing to stdout",
    0
};

static void
init_help_array(void)
{
  gengetopt_args_info_help[0] = gengetopt_args_info_full_help[0];
  gengetopt_args_info_help[1] = gengetopt_args_info_full_help[1];
  gengetopt_args_info_help[2] = gengetopt_args_info_full_help[2];
  gengetopt_args_info_help[3] = gengetopt_args_info_full_help[3];
  gengetopt_args_info_help[4] = gengetopt_args_info_full_help[4];
  gengetopt_args_info_help[5] = gengetopt_args_info_full_help[5];
  gengetopt_args_info_help[6] = gengetopt_args_info_full_help[6];
  gengetopt_args_info_help[7] = gengetopt_args_info_full_help[7];
  gengetopt_args_info_help[8] = gengetopt_args_info_full_help[8];
  gengetopt_args_info_help[9] = gengetopt_args_info_full_help[9];
  gengetopt_args_info_help[10] = gengetopt_args_info_full_help[10];
  gengetopt_args_info_help[11] = gengetopt_args_info_full_help[11];
  gengetopt_args_info_help[12] = gengetopt_args_info_full_help[12];
  gengetopt_args_info_help[13] = gengetopt_args_info_full_help[13];
  gengetopt_args_info_help[14] = gengetopt_args_info_full_help[14];
  gengetopt_args_info_help[15] = gengetopt_args_info_full_help[15];
  gengetopt_args_info_help[16] = gengetopt_args_info_full_help[17];
  gengetopt_args_info_help[17] = gengetopt_args_info_full_help[18];
  gengetopt_args_info_help[18] = gengetopt_args_info_full_help[19];
  gengetopt_args_info_help[19] = gengetopt_args_info_full_help[20];
  gengetopt_args_info_help[20] = gengetopt_args_info_full_help[22];
  gengetopt_args_info_help[21] = gengetopt_args_info_full_help[23];
  gengetopt_args_info_help[22] = gengetopt_args_info_full_help[25];
  gengetopt_args_info_help[23] = gengetopt_args_info_full_help[27];
  gengetopt_args_info_help[24] = gengetopt_args_info_full_help[28];
  gengetopt_args_info_help[25] = gengetopt_args_info_full_help[29];
  gengetopt_args_info_help[26] = gengetopt_args_info_full_help[30];
  gengetopt_args_info_help[27] = gengetopt_args_info_full_help[31];
  gengetopt_args_info_help[28] = gengetopt_args_info_full_help[32];
  gengetopt_args_info_help[29] = gengetopt_args_info_full_help[33];
  gengetopt_args_info_help[30] = gengetopt_args_info_full_help[34];
  gengetopt_args_info_help[31] = gengetopt_args_info_full_help[35];
  gengetopt_args_info_help[32] = gengetopt_args_info_full_help[36];
  gengetopt_args_info_help[33] = gengetopt_args_info_full_help[37];
  gengetopt_args_info_help[34] = gengetopt_args_info_full_help[38];
  gengetopt_args_info_help[35] = gengetopt_args_info_full_help[39];
  gengetopt_args_info_help[36] = gengetopt_args_info_full_help[40];
  gengetopt_args_info_help[37] = gengetopt_args_info_full_help[41];
  gengetopt_args_info_help[38] = gengetopt_args_info_full_help[42];
  gengetopt_args_info_help[39] = gengetopt_args_info_full_help[43];
  gengetopt_args_info_help[40] = gengetopt_args_info_full_help[44];
  gengetopt_args_info_help[41] = gengetopt_args_info_full_help[45];
  gengetopt_args_info_help[42] = gengetopt_args_info_full_help[46];
  gengetopt_args_info_help[43] = gengetopt_args_info_full_help[47];
  gengetopt_args_info_help[44] = gengetopt_args_info_full_help[48];
  gengetopt_args_info_help[45] = 0;

}

const char *gengetopt_args_info_help[46];

typedef enum {ARG_NO
  , ARG_STRING
  , ARG_INT
  , ARG_FLOAT
} cmdline_parser_arg_type;

static
void clear_given (struct gengetopt_args_info *args_info);
static
void clear_args (struct gengetopt_args_info *args_info);

static int
cmdline_parser_internal (int argc, char **argv, struct gengetopt_args_info *args_info,
                        struct cmdline_parser_params *params, const char *additional_error);

static int
cmdline_parser_required2 (struct gengetopt_args_info *args_info, const char *prog_name, const char *additional_error);

static char *
gengetopt_strdup (const char *s);

static
void clear_given (struct gengetopt_args_info *args_info)
{
  args_info->help_given = 0 ;
  args_info->full_help_given = 0 ;
  args_info->version_given = 0 ;
  args_info->start_given = 0 ;
  args_info->stop_given = 0 ;
  args_info->volume_given = 0 ;
  args_info->profile_given = 0 ;
  args_info->device_given = 0 ;
  args_info->resetMtpEx_given = 0 ;
  args_info->reset_given = 0 ;
  args_info->calibrate_given = 0 ;
  args_info->calshow_given = 0 ;
  args_info->params_given = 0 ;
  args_info->re0_given = 0 ;
  args_info->dsp_given = 0 ;
  args_info->save_given = 0 ;
  args_info->currentprof_given = 0 ;
  args_info->tone_given = 0 ;
  args_info->versions_given = 0 ;
  args_info->register_given = 0 ;
  args_info->regwrite_given = 0 ;
  args_info->dump_given = 0 ;
  args_info->pin_given = 0 ;
  args_info->diag_given = 0 ;
  args_info->xmem_given = 0 ;
  args_info->dumpmodel_given = 0 ;
  args_info->record_given = 0 ;
  args_info->count_given = 0 ;
  args_info->output_given = 0 ;
  args_info->logger_given = 0 ;
  args_info->ini2cnt_given = 0 ;
  args_info->bin2hdr_given = 0 ;
  args_info->maximus_given = 0 ;
  args_info->load_given = 0 ;
  args_info->splitparms_given = 0 ;
  args_info->server_given = 0 ;
  args_info->client_given = 0 ;
  args_info->slave_given = 0 ;
  args_info->loop_given = 0 ;
  args_info->verbose_given = 0 ;
  args_info->trace_given = 0 ;
  args_info->quiet_given = 0 ;
}

static
void clear_args (struct gengetopt_args_info *args_info)
{
  FIX_UNUSED (args_info);
  args_info->volume_arg = NULL;
  args_info->volume_orig = NULL;
  args_info->profile_orig = NULL;
  args_info->device_arg = NULL;
  args_info->device_orig = NULL;
  args_info->calibrate_arg = gengetopt_strdup ("always");
  args_info->calibrate_orig = NULL;
  args_info->params_arg = NULL;
  args_info->params_orig = NULL;
  args_info->re0_orig = NULL;
  args_info->dsp_arg = 0x80;
  args_info->dsp_orig = NULL;
  args_info->save_arg = NULL;
  args_info->save_orig = NULL;
  args_info->currentprof_orig = NULL;
  args_info->tone_orig = NULL;
  args_info->register_arg = NULL;
  args_info->register_orig = NULL;
  args_info->regwrite_arg = NULL;
  args_info->regwrite_orig = NULL;
  args_info->pin_orig = NULL;
  args_info->diag_arg = 0;
  args_info->diag_orig = NULL;
  args_info->xmem_arg = NULL;
  args_info->xmem_orig = NULL;
  args_info->dumpmodel_arg = gengetopt_strdup ("z");
  args_info->dumpmodel_orig = NULL;
  args_info->record_arg = 55;
  args_info->record_orig = NULL;
  args_info->count_orig = NULL;
  args_info->output_arg = NULL;
  args_info->output_orig = NULL;
  args_info->logger_arg = 2;
  args_info->logger_orig = NULL;
  args_info->ini2cnt_arg = NULL;
  args_info->ini2cnt_orig = NULL;
  args_info->bin2hdr_arg = NULL;
  args_info->bin2hdr_orig = NULL;
  args_info->maximus_arg = 1;
  args_info->maximus_orig = NULL;
  args_info->load_arg = NULL;
  args_info->load_orig = NULL;
  args_info->server_arg = gengetopt_strdup ("9887");
  args_info->server_orig = NULL;
  args_info->client_arg = gengetopt_strdup ("9887");
  args_info->client_orig = NULL;
  args_info->slave_orig = NULL;
  args_info->loop_orig = NULL;
  args_info->verbose_orig = NULL;
  args_info->trace_arg = NULL;
  args_info->trace_orig = NULL;

}

static
void init_args_info(struct gengetopt_args_info *args_info)
{

  init_help_array();
  args_info->help_help = gengetopt_args_info_full_help[0] ;
  args_info->full_help_help = gengetopt_args_info_full_help[1] ;
  args_info->version_help = gengetopt_args_info_full_help[2] ;
  args_info->start_help = gengetopt_args_info_full_help[4] ;
  args_info->stop_help = gengetopt_args_info_full_help[5] ;
  args_info->volume_help = gengetopt_args_info_full_help[6] ;
  args_info->volume_min = 0;
  args_info->volume_max = 0;
  args_info->profile_help = gengetopt_args_info_full_help[7] ;
  args_info->device_help = gengetopt_args_info_full_help[9] ;
  args_info->resetMtpEx_help = gengetopt_args_info_full_help[10] ;
  args_info->reset_help = gengetopt_args_info_full_help[11] ;
  args_info->calibrate_help = gengetopt_args_info_full_help[13] ;
  args_info->calshow_help = gengetopt_args_info_full_help[14] ;
  args_info->params_help = gengetopt_args_info_full_help[15] ;
  args_info->params_min = 0;
  args_info->params_max = 0;
  args_info->re0_help = gengetopt_args_info_full_help[16] ;
  args_info->dsp_help = gengetopt_args_info_full_help[17] ;
  args_info->save_help = gengetopt_args_info_full_help[18] ;
  args_info->currentprof_help = gengetopt_args_info_full_help[20] ;
  args_info->tone_help = gengetopt_args_info_full_help[21] ;
  args_info->versions_help = gengetopt_args_info_full_help[22] ;
  args_info->register_help = gengetopt_args_info_full_help[23] ;
  args_info->register_min = 0;
  args_info->register_max = 0;
  args_info->regwrite_help = gengetopt_args_info_full_help[24] ;
  args_info->regwrite_min = 0;
  args_info->regwrite_max = 0;
  args_info->dump_help = gengetopt_args_info_full_help[25] ;
  args_info->pin_help = gengetopt_args_info_full_help[26] ;
  args_info->diag_help = gengetopt_args_info_full_help[27] ;
  args_info->xmem_help = gengetopt_args_info_full_help[28] ;
  args_info->xmem_min = 0;
  args_info->xmem_max = 0;
  args_info->dumpmodel_help = gengetopt_args_info_full_help[30] ;
  args_info->record_help = gengetopt_args_info_full_help[31] ;
  args_info->count_help = gengetopt_args_info_full_help[32] ;
  args_info->output_help = gengetopt_args_info_full_help[33] ;
  args_info->logger_help = gengetopt_args_info_full_help[34] ;
  args_info->ini2cnt_help = gengetopt_args_info_full_help[36] ;
  args_info->bin2hdr_help = gengetopt_args_info_full_help[37] ;
  args_info->maximus_help = gengetopt_args_info_full_help[38] ;
  args_info->load_help = gengetopt_args_info_full_help[40] ;
  args_info->splitparms_help = gengetopt_args_info_full_help[41] ;
  args_info->server_help = gengetopt_args_info_full_help[42] ;
  args_info->client_help = gengetopt_args_info_full_help[43] ;
  args_info->slave_help = gengetopt_args_info_full_help[44] ;
  args_info->loop_help = gengetopt_args_info_full_help[45] ;
  args_info->verbose_help = gengetopt_args_info_full_help[46] ;
  args_info->trace_help = gengetopt_args_info_full_help[47] ;
  args_info->quiet_help = gengetopt_args_info_full_help[48] ;

}

void
cmdline_parser_print_version (void)
{
  PRINT ("%s %s\n",
     (strlen(CMDLINE_PARSER_PACKAGE_NAME) ? CMDLINE_PARSER_PACKAGE_NAME : CMDLINE_PARSER_PACKAGE),
     CMDLINE_PARSER_VERSION);

  if (strlen(gengetopt_args_info_versiontext) > 0)
    PRINT("\n%s\n", gengetopt_args_info_versiontext);
}

static void print_help_common(void) {
  cmdline_parser_print_version ();

  if (strlen(gengetopt_args_info_purpose) > 0)
    PRINT("\n%s\n", gengetopt_args_info_purpose);

  if (strlen(gengetopt_args_info_usage) > 0)
    PRINT("\n%s\n", gengetopt_args_info_usage);

  PRINT("\n");

  if (strlen(gengetopt_args_info_description) > 0)
    PRINT("%s\n\n", gengetopt_args_info_description);
}

void
cmdline_parser_print_help (void)
{
  int i = 0;
  print_help_common();
  while (gengetopt_args_info_help[i])
    PRINT("%s\n", gengetopt_args_info_help[i++]);
}

void
cmdline_parser_print_full_help (void)
{
  int i = 0;
  print_help_common();
  while (gengetopt_args_info_full_help[i])
    PRINT("%s\n", gengetopt_args_info_full_help[i++]);
}

void
cmdline_parser_init (struct gengetopt_args_info *args_info)
{
  clear_given (args_info);
  clear_args (args_info);
  init_args_info (args_info);
}

void
cmdline_parser_params_init(struct cmdline_parser_params *params)
{
  if (params)
    {
      params->override = 0;
      params->initialize = 1;
      params->check_required = 1;
      params->check_ambiguity = 0;
      params->print_errors = 1;
    }
}

struct cmdline_parser_params *
cmdline_parser_params_create(void)
{
  struct cmdline_parser_params *params =
    (struct cmdline_parser_params *)malloc(sizeof(struct cmdline_parser_params));
  cmdline_parser_params_init(params);
  return params;
}

static void
free_string_field (char **s)
{
  if (*s)
    {
      free (*s);
      *s = 0;
    }
}

/** @brief generic value variable */
union generic_value {
    int int_arg;
    float float_arg;
    char *string_arg;
    const char *default_string_arg;
};

/** @brief holds temporary values for multiple options */
struct generic_list
{
  union generic_value arg;
  char *orig;
  struct generic_list *next;
};

/**
 * @brief add a node at the head of the list
 */
static void add_node(struct generic_list **list) {
  struct generic_list *new_node = (struct generic_list *) malloc (sizeof (struct generic_list));
  new_node->next = *list;
  *list = new_node;
  new_node->arg.string_arg = 0;
  new_node->orig = 0;
}

/**
 * The passed arg parameter is NOT set to 0 from this function
 */
static void
free_multiple_field(unsigned int len, void *arg, char ***orig)
{
  unsigned int i;
  if (arg) {
    for (i = 0; i < len; ++i)
      {
        free_string_field(&((*orig)[i]));
      }

    free (arg);
    free (*orig);
    *orig = 0;
  }
}

static void
free_multiple_string_field(unsigned int len, char ***arg, char ***orig)
{
  unsigned int i;
  if (*arg) {
    for (i = 0; i < len; ++i)
      {
        free_string_field(&((*arg)[i]));
        free_string_field(&((*orig)[i]));
      }
    free_string_field(&((*arg)[0])); /* free default string */

    free (*arg);
    *arg = 0;
    free (*orig);
    *orig = 0;
  }
}

static void
cmdline_parser_release (struct gengetopt_args_info *args_info)
{

  free_multiple_field (args_info->volume_given, (void *)(args_info->volume_arg), &(args_info->volume_orig));
  args_info->volume_arg = 0;
  free_string_field (&(args_info->profile_orig));
  free_string_field (&(args_info->device_arg));
  free_string_field (&(args_info->device_orig));
  free_string_field (&(args_info->calibrate_arg));
  free_string_field (&(args_info->calibrate_orig));
  free_multiple_string_field (args_info->params_given, &(args_info->params_arg), &(args_info->params_orig));
  free_string_field (&(args_info->re0_orig));
  free_string_field (&(args_info->dsp_orig));
  free_string_field (&(args_info->save_arg));
  free_string_field (&(args_info->save_orig));
  free_string_field (&(args_info->currentprof_orig));
  free_string_field (&(args_info->tone_orig));
  free_multiple_field (args_info->register_given, (void *)(args_info->register_arg), &(args_info->register_orig));
  args_info->register_arg = 0;
  free_multiple_field (args_info->regwrite_given, (void *)(args_info->regwrite_arg), &(args_info->regwrite_orig));
  args_info->regwrite_arg = 0;
  free_string_field (&(args_info->pin_orig));
  free_string_field (&(args_info->diag_orig));
  free_multiple_field (args_info->xmem_given, (void *)(args_info->xmem_arg), &(args_info->xmem_orig));
  args_info->xmem_arg = 0;
  free_string_field (&(args_info->dumpmodel_arg));
  free_string_field (&(args_info->dumpmodel_orig));
  free_string_field (&(args_info->record_orig));
  free_string_field (&(args_info->count_orig));
  free_string_field (&(args_info->output_arg));
  free_string_field (&(args_info->output_orig));
  free_string_field (&(args_info->logger_orig));
  free_string_field (&(args_info->ini2cnt_arg));
  free_string_field (&(args_info->ini2cnt_orig));
  free_string_field (&(args_info->bin2hdr_arg));
  free_string_field (&(args_info->bin2hdr_orig));
  free_string_field (&(args_info->maximus_orig));
  free_string_field (&(args_info->load_arg));
  free_string_field (&(args_info->load_orig));
  free_string_field (&(args_info->server_arg));
  free_string_field (&(args_info->server_orig));
  free_string_field (&(args_info->client_arg));
  free_string_field (&(args_info->client_orig));
  free_string_field (&(args_info->slave_orig));
  free_string_field (&(args_info->loop_orig));
  free_string_field (&(args_info->verbose_orig));
  free_string_field (&(args_info->trace_arg));
  free_string_field (&(args_info->trace_orig));



  clear_given (args_info);
}


static void
write_into_file(FILE *outfile, const char *opt, const char *arg, const char *values[])
{
  FIX_UNUSED (values);
  if (arg) {
    PRINT_FILE(outfile, "%s=\"%s\"\n", opt, arg);
  } else {
    PRINT_FILE(outfile, "%s\n", opt);
  }
}

static void
write_multiple_into_file(FILE *outfile, int len, const char *opt, char **arg, const char *values[])
{
  int i;

  for (i = 0; i < len; ++i)
    write_into_file(outfile, opt, (arg ? arg[i] : 0), values);
}

int
cmdline_parser_dump(FILE *outfile, struct gengetopt_args_info *args_info)
{
  int i = 0;

  if (!outfile)
    {
      fprintf (stderr, "%s: cannot dump options to stream\n", CMDLINE_PARSER_PACKAGE);
      return EXIT_FAILURE;
    }

  if (args_info->help_given)
    write_into_file(outfile, "help", 0, 0 );
  if (args_info->full_help_given)
    write_into_file(outfile, "full-help", 0, 0 );
  if (args_info->version_given)
    write_into_file(outfile, "version", 0, 0 );
  if (args_info->start_given)
    write_into_file(outfile, "start", 0, 0 );
  if (args_info->stop_given)
    write_into_file(outfile, "stop", 0, 0 );
  write_multiple_into_file(outfile, args_info->volume_given, "volume", args_info->volume_orig, 0);
  if (args_info->profile_given)
    write_into_file(outfile, "profile", args_info->profile_orig, 0);
  if (args_info->device_given)
    write_into_file(outfile, "device", args_info->device_orig, 0);
  if (args_info->resetMtpEx_given)
    write_into_file(outfile, "resetMtpEx", 0, 0 );
  if (args_info->reset_given)
    write_into_file(outfile, "reset", 0, 0 );
  if (args_info->calibrate_given)
    write_into_file(outfile, "calibrate", args_info->calibrate_orig, 0);
  if (args_info->calshow_given)
    write_into_file(outfile, "calshow", 0, 0 );
  write_multiple_into_file(outfile, args_info->params_given, "params", args_info->params_orig, 0);
  if (args_info->re0_given)
    write_into_file(outfile, "re0", args_info->re0_orig, 0);
  if (args_info->dsp_given)
    write_into_file(outfile, "dsp", args_info->dsp_orig, 0);
  if (args_info->save_given)
    write_into_file(outfile, "save", args_info->save_orig, 0);
  if (args_info->currentprof_given)
    write_into_file(outfile, "currentprof", args_info->currentprof_orig, 0);
  if (args_info->tone_given)
    write_into_file(outfile, "tone", args_info->tone_orig, 0);
  if (args_info->versions_given)
    write_into_file(outfile, "versions", 0, 0 );
  write_multiple_into_file(outfile, args_info->register_given, "register", args_info->register_orig, 0);
  write_multiple_into_file(outfile, args_info->regwrite_given, "regwrite", args_info->regwrite_orig, 0);
  if (args_info->dump_given)
    write_into_file(outfile, "dump", 0, 0 );
  if (args_info->pin_given)
    write_into_file(outfile, "pin", args_info->pin_orig, 0);
  if (args_info->diag_given)
    write_into_file(outfile, "diag", args_info->diag_orig, 0);
  write_multiple_into_file(outfile, args_info->xmem_given, "xmem", args_info->xmem_orig, 0);
  if (args_info->dumpmodel_given)
    write_into_file(outfile, "dumpmodel", args_info->dumpmodel_orig, 0);
  if (args_info->record_given)
    write_into_file(outfile, "record", args_info->record_orig, 0);
  if (args_info->count_given)
    write_into_file(outfile, "count", args_info->count_orig, 0);
  if (args_info->output_given)
    write_into_file(outfile, "output", args_info->output_orig, 0);
  if (args_info->logger_given)
    write_into_file(outfile, "logger", args_info->logger_orig, 0);
  if (args_info->ini2cnt_given)
    write_into_file(outfile, "ini2cnt", args_info->ini2cnt_orig, 0);
  if (args_info->bin2hdr_given)
    write_into_file(outfile, "bin2hdr", args_info->bin2hdr_orig, 0);
  if (args_info->maximus_given)
    write_into_file(outfile, "maximus", args_info->maximus_orig, 0);
  if (args_info->load_given)
    write_into_file(outfile, "load", args_info->load_orig, 0);
  if (args_info->splitparms_given)
    write_into_file(outfile, "splitparms", 0, 0 );
  if (args_info->server_given)
    write_into_file(outfile, "server", args_info->server_orig, 0);
  if (args_info->client_given)
    write_into_file(outfile, "client", args_info->client_orig, 0);
  if (args_info->slave_given)
    write_into_file(outfile, "slave", args_info->slave_orig, 0);
  if (args_info->loop_given)
    write_into_file(outfile, "loop", args_info->loop_orig, 0);
  if (args_info->verbose_given)
    write_into_file(outfile, "verbose", args_info->verbose_orig, 0);
  if (args_info->trace_given)
    write_into_file(outfile, "trace", args_info->trace_orig, 0);
  if (args_info->quiet_given)
    write_into_file(outfile, "quiet", 0, 0 );


  i = EXIT_SUCCESS;
  return i;
}

int
cmdline_parser_file_save(const char *filename, struct gengetopt_args_info *args_info)
{
  FILE *outfile;
  int i = 0;

  outfile = fopen(filename, "w");

  if (!outfile)
    {
      PRINT_ERROR("%s: cannot open file for writing: %s\n", CMDLINE_PARSER_PACKAGE, filename);
      return EXIT_FAILURE;
    }

  i = cmdline_parser_dump(outfile, args_info);
  fclose (outfile);

  return i;
}

void
cmdline_parser_free (struct gengetopt_args_info *args_info)
{
  cmdline_parser_release (args_info);
}

/** @brief replacement of strdup, which is not standard */
char *
gengetopt_strdup (const char *s)
{
  char *result = 0;
  if (!s)
    return result;

  result = (char*)malloc(strlen(s) + 1);
  if (result == (char*)0)
    return (char*)0;
  strcpy(result, s);
  return result;
}

static char *
get_multiple_arg_token(const char *arg)
{
  const char *tok;
  char *ret;
  size_t len, num_of_escape, i, j;

  if (!arg)
    return 0;

  tok = strchr (arg, ',');
  num_of_escape = 0;

  /* make sure it is not escaped */
  while (tok)
    {
      if (*(tok-1) == '\\')
        {
          /* find the next one */
          tok = strchr (tok+1, ',');
          ++num_of_escape;
        }
      else
        break;
    }

  if (tok)
    len = (size_t)(tok - arg + 1);
  else
    len = strlen (arg) + 1;

  len -= num_of_escape;

  ret = (char *) malloc (len);

  i = 0;
  j = 0;
  while (arg[i] && (j < len-1))
    {
      if (arg[i] == '\\' &&
      arg[ i + 1 ] &&
      arg[ i + 1 ] == ',')
        ++i;

      ret[j++] = arg[i++];
    }

  ret[len-1] = '\0';

  return ret;
}

static const char *
get_multiple_arg_token_next(const char *arg)
{
  const char *tok;

  if (!arg)
    return 0;

  tok = strchr (arg, ',');

  /* make sure it is not escaped */
  while (tok)
    {
      if (*(tok-1) == '\\')
        {
          /* find the next one */
          tok = strchr (tok+1, ',');
        }
      else
        break;
    }

  if (! tok || strlen(tok) == 1)
    return 0;

  return tok+1;
}

static int
check_multiple_option_occurrences(const char *prog_name, unsigned int option_given, unsigned int min, unsigned int max, const char *option_desc);

int
check_multiple_option_occurrences(const char *prog_name, unsigned int option_given, unsigned int min, unsigned int max, const char *option_desc)
{
  int error = 0;

  if (option_given && (min > 0 || max > 0))
    {
      if (min > 0 && max > 0)
        {
          if (min == max)
            {
              /* specific occurrences */
              if (option_given != (unsigned int) min)
                {
                  PRINT_ERROR("%s: %s option occurrences must be %d\n",
                    prog_name, option_desc, min);
                  error = 1;
                }
            }
          else if (option_given < (unsigned int) min
                || option_given > (unsigned int) max)
            {
              /* range occurrences */
              PRINT_ERROR("%s: %s option occurrences must be between %d and %d\n",
                prog_name, option_desc, min, max);
              error = 1;
            }
        }
      else if (min > 0)
        {
          /* at least check */
          if (option_given < min)
            {
              PRINT_ERROR("%s: %s option occurrences must be at least %d\n",
                prog_name, option_desc, min);
              error = 1;
            }
        }
      else if (max > 0)
        {
          /* at most check */
          if (option_given > max)
            {
              PRINT_ERROR("%s: %s option occurrences must be at most %d\n",
                prog_name, option_desc, max);
              error = 1;
            }
        }
    }

  return error;
}
int
cmdline_parser (int argc, char **argv, struct gengetopt_args_info *args_info)
{
  return cmdline_parser2 (argc, argv, args_info, 0, 1, 1);
}

int
cmdline_parser_ext (int argc, char **argv, struct gengetopt_args_info *args_info,
                   struct cmdline_parser_params *params)
{
  int result;
  result = cmdline_parser_internal (argc, argv, args_info, params, 0);

  if (result == EXIT_FAILURE)
    {
      cmdline_parser_free (args_info);
      exit (EXIT_FAILURE);
    }

  return result;
}

int
cmdline_parser2 (int argc, char **argv, struct gengetopt_args_info *args_info, int override, int initialize, int check_required)
{
  int result;
  struct cmdline_parser_params params;

  params.override = override;
  params.initialize = initialize;
  params.check_required = check_required;
  params.check_ambiguity = 0;
  params.print_errors = 1;

  result = cmdline_parser_internal (argc, argv, args_info, &params, 0);

  if (result == EXIT_FAILURE)
    {
      cmdline_parser_free (args_info);
      exit (EXIT_FAILURE);
    }

  return result;
}

int
cmdline_parser_required (struct gengetopt_args_info *args_info, const char *prog_name)
{
  int result = EXIT_SUCCESS;

  if (cmdline_parser_required2(args_info, prog_name, 0) > 0)
    result = EXIT_FAILURE;

  if (result == EXIT_FAILURE)
    {
      cmdline_parser_free (args_info);
      exit (EXIT_FAILURE);
    }

  return result;
}

int
cmdline_parser_required2 (struct gengetopt_args_info *args_info, const char *prog_name, const char *additional_error)
{
  int error = 0;
  FIX_UNUSED (additional_error);

  /* checks for required options */
  if (check_multiple_option_occurrences(prog_name, args_info->volume_given, args_info->volume_min, args_info->volume_max, "'--volume' ('-v')"))
     error = 1;

  if (check_multiple_option_occurrences(prog_name, args_info->params_given, args_info->params_min, args_info->params_max, "'--params' ('-p')"))
     error = 1;

  if (check_multiple_option_occurrences(prog_name, args_info->register_given, args_info->register_min, args_info->register_max, "'--register' ('-r')"))
     error = 1;

  if (check_multiple_option_occurrences(prog_name, args_info->regwrite_given, args_info->regwrite_min, args_info->regwrite_max, "'--regwrite' ('-w')"))
     error = 1;

  if (check_multiple_option_occurrences(prog_name, args_info->xmem_given, args_info->xmem_min, args_info->xmem_max, "'--xmem' ('-x')"))
     error = 1;


  /* checks for dependences among options */
  if (args_info->currentprof_given && ! args_info->profile_given)
    {
      PRINT_ERROR("%s: '--currentprof' option depends on option 'profile'%s\n", prog_name, (additional_error ? additional_error : ""));
      error = 1;
    }
  if (args_info->splitparms_given && ! args_info->load_given)
    {
      PRINT_ERROR("%s: '--splitparms' option depends on option 'load'%s\n", prog_name, (additional_error ? additional_error : ""));
      error = 1;
    }

  return error;
}


static char *package_name = 0;

/**
 * @brief updates an option
 * @param field the generic pointer to the field to update
 * @param orig_field the pointer to the orig field
 * @param field_given the pointer to the number of occurrence of this option
 * @param prev_given the pointer to the number of occurrence already seen
 * @param value the argument for this option (if null no arg was specified)
 * @param possible_values the possible values for this option (if specified)
 * @param default_value the default value (in case the option only accepts fixed values)
 * @param arg_type the type of this option
 * @param check_ambiguity @see cmdline_parser_params.check_ambiguity
 * @param override @see cmdline_parser_params.override
 * @param no_free whether to free a possible previous value
 * @param multiple_option whether this is a multiple option
 * @param long_opt the corresponding long option
 * @param short_opt the corresponding short option (or '-' if none)
 * @param additional_error possible further error specification
 */
static
int update_arg(void *field, char **orig_field,
               unsigned int *field_given, unsigned int *prev_given,
               char *value, const char *possible_values[],
               const char *default_value,
               cmdline_parser_arg_type arg_type,
               int check_ambiguity, int override,
               int no_free, int multiple_option,
               const char *long_opt, char short_opt,
               const char *additional_error)
{
  char *stop_char = 0;
  const char *val = value;
  int found;
  char **string_field;
  FIX_UNUSED (field);

  stop_char = 0;
  found = 0;

  if (!multiple_option && prev_given && (*prev_given || (check_ambiguity && *field_given)))
    {
      if (short_opt != '-')
        PRINT_ERROR("%s: `--%s' (`-%c') option given more than once%s\n",
               package_name, long_opt, short_opt,
               (additional_error ? additional_error : ""));
      else
        PRINT_ERROR("%s: `--%s' option given more than once%s\n",
               package_name, long_opt,
               (additional_error ? additional_error : ""));
      return 1; /* failure */
    }

  FIX_UNUSED (default_value);

  if (field_given && *field_given && ! override)
    return 0;
  if (prev_given)
    (*prev_given)++;
  if (field_given)
    (*field_given)++;
  if (possible_values)
    val = possible_values[found];

  switch(arg_type) {
  case ARG_INT:
    if (val) *((int *)field) = strtol (val, &stop_char, 0);
    break;
  case ARG_FLOAT:
    if (val) *((float *)field) = (float)strtod (val, &stop_char);
    break;
  case ARG_STRING:
    if (val) {
      string_field = (char **)field;
      if (!no_free && *string_field)
        free (*string_field); /* free previous string */
      *string_field = gengetopt_strdup (val);
    }
    break;
  default:
    break;
  };

  /* check numeric conversion */
  switch(arg_type) {
  case ARG_INT:
  case ARG_FLOAT:
    if (val && !(stop_char && *stop_char == '\0')) {
      PRINT_ERROR("%s: invalid numeric value: %s\n", package_name, val);
      return 1; /* failure */
    }
    break;
  default:
    ;
  };

  /* store the original value */
  switch(arg_type) {
  case ARG_NO:
    break;
  default:
    if (value && orig_field) {
      if (no_free) {
        *orig_field = value;
      } else {
        if (*orig_field)
          free (*orig_field); /* free previous string */
        *orig_field = gengetopt_strdup (value);
      }
    }
  };

  return 0; /* OK */
}

/**
 * @brief store information about a multiple option in a temporary list
 * @param list where to (temporarily) store multiple options
 */
static
int update_multiple_arg_temp(struct generic_list **list,
               unsigned int *prev_given, const char *val,
               const char *possible_values[], const char *default_value,
               cmdline_parser_arg_type arg_type,
               const char *long_opt, char short_opt,
               const char *additional_error)
{
  /* store single arguments */
  char *multi_token;
  const char *multi_next;

  if (arg_type == ARG_NO) {
    (*prev_given)++;
    return 0; /* OK */
  }

  multi_token = get_multiple_arg_token(val);
  multi_next = get_multiple_arg_token_next (val);

  while (1)
    {
      add_node (list);
      if (update_arg((void *)&((*list)->arg), &((*list)->orig), 0,
          prev_given, multi_token, possible_values, default_value,
          arg_type, 0, 1, 1, 1, long_opt, short_opt, additional_error)) {
        if (multi_token) free(multi_token);
        return 1; /* failure */
      }

      if (multi_next)
        {
          multi_token = get_multiple_arg_token(multi_next);
          multi_next = get_multiple_arg_token_next (multi_next);
        }
      else
        break;
    }

  return 0; /* OK */
}

/**
 * @brief free the passed list (including possible string argument)
 */
static
void free_list(struct generic_list *list, short string_arg)
{
  if (list) {
    struct generic_list *tmp;
    while (list)
      {
        tmp = list;
        if (string_arg && list->arg.string_arg)
          free (list->arg.string_arg);
        if (list->orig)
          free (list->orig);
        list = list->next;
        free (tmp);
      }
  }
}

/**
 * @brief updates a multiple option starting from the passed list
 */
static
void update_multiple_arg(void *field, char ***orig_field,
               unsigned int field_given, unsigned int prev_given, union generic_value *default_value,
               cmdline_parser_arg_type arg_type,
               struct generic_list *list)
{
  int i;
  struct generic_list *tmp;

  if (prev_given && list) {
    *orig_field = (char **) realloc (*orig_field, (field_given + prev_given) * sizeof (char *));

    switch(arg_type) {
    case ARG_INT:
      *((int **)field) = (int *)realloc (*((int **)field), (field_given + prev_given) * sizeof (int)); break;
    case ARG_FLOAT:
      *((float **)field) = (float *)realloc (*((float **)field), (field_given + prev_given) * sizeof (float)); break;
    case ARG_STRING:
      *((char ***)field) = (char **)realloc (*((char ***)field), (field_given + prev_given) * sizeof (char *)); break;
    default:
      break;
    };

    for (i = (prev_given - 1); i >= 0; --i)
      {
        tmp = list;

        switch(arg_type) {
        case ARG_INT:
          (*((int **)field))[i + field_given] = tmp->arg.int_arg; break;
        case ARG_FLOAT:
          (*((float **)field))[i + field_given] = tmp->arg.float_arg; break;
        case ARG_STRING:
          (*((char ***)field))[i + field_given] = tmp->arg.string_arg; break;
        default:
          break;
        }
        (*orig_field) [i + field_given] = list->orig;
        list = list->next;
        free (tmp);
      }
  } else { /* set the default value */
    if (default_value && ! field_given) {
      switch(arg_type) {
      case ARG_INT:
        if (! *((int **)field)) {
          *((int **)field) = (int *)malloc (sizeof (int));
          (*((int **)field))[0] = default_value->int_arg;
        }
        break;
      case ARG_FLOAT:
        if (! *((float **)field)) {
          *((float **)field) = (float *)malloc (sizeof (float));
          (*((float **)field))[0] = default_value->float_arg;
        }
        break;
      case ARG_STRING:
        if (! *((char ***)field)) {
          *((char ***)field) = (char **)malloc (sizeof (char *));
          (*((char ***)field))[0] = gengetopt_strdup(default_value->string_arg);
        }
        break;
      default: break;
      }
      if (!(*orig_field)) {
        *orig_field = (char **) malloc (sizeof (char *));
        (*orig_field)[0] = 0;
      }
    }
  }
}

#if (defined(WIN32) || defined(_X64))
  char *optarg;
  int optind;
  int opterr;
  int optopt;
#endif
int
cmdline_parser_internal (
  int argc, char **argv, struct gengetopt_args_info *args_info,
                        struct cmdline_parser_params *params, const char *additional_error)
{
  int c;    /* Character of the parsed option.  */

  struct generic_list * volume_list = NULL;
  struct generic_list * params_list = NULL;
  struct generic_list * register_list = NULL;
  struct generic_list * regwrite_list = NULL;
  struct generic_list * xmem_list = NULL;
  int error = 0;
  struct gengetopt_args_info local_args_info;

  int override;
  int initialize;
  int check_required;
  int check_ambiguity;

  package_name = argv[0];

  override = params->override;
  initialize = params->initialize;
  check_required = params->check_required;
  check_ambiguity = params->check_ambiguity;

  if (initialize)
    cmdline_parser_init (args_info);

  cmdline_parser_init (&local_args_info);

  optarg = 0;
  optind = 0;
  opterr = params->print_errors;
  optopt = '?';

  while (1)
    {
      int option_index = 0;

      static struct option long_options[] = {
        { "help",    0, NULL, 'h' },
        { "full-help",    0, NULL, 0 },
        { "version",    0, NULL, 0 },
        { "start",    0, NULL, 0 },
        { "stop",    0, NULL, 0 },
        { "volume",    1, NULL, 'v' },
        { "profile",    2, NULL, 'P' },
        { "device",    1, NULL, 'd' },
        { "resetMtpEx",    0, NULL, 0 },
        { "reset",    0, NULL, 'R' },
        { "calibrate",    2, NULL, 'a' },
        { "calshow",    0, NULL, 'A' },
        { "params",    1, NULL, 'p' },
        { "re0",    2, NULL, 0 },
        { "dsp",    2, NULL, 'D' },
        { "save",    1, NULL, 's' },
        { "currentprof",    1, NULL, 0 },
        { "tone",    1, NULL, 0 },
        { "versions",    0, NULL, 'V' },
        { "register",    1, NULL, 'r' },
        { "regwrite",    1, NULL, 'w' },
        { "dump",    0, NULL, 0 },
        { "pin",    1, NULL, 0 },
        { "diag",    2, NULL, 0 },
        { "xmem",    1, NULL, 'x' },
        { "dumpmodel",    2, NULL, 0 },
        { "record",    2, NULL, 0 },
        { "count",    1, NULL, 0 },
        { "output",    1, NULL, 'o' },
        { "logger",    2, NULL, 0 },
        { "ini2cnt",    1, NULL, 0 },
        { "bin2hdr",    1, NULL, 0 },
        { "maximus",    1, NULL, 'm' },
        { "load",    1, NULL, 'l' },
        { "splitparms",    0, NULL, 0 },
        { "server",    2, NULL, 0 },
        { "client",    2, NULL, 0 },
        { "slave",    1, NULL, 0 },
        { "loop",    1, NULL, 'L' },
        { "verbose",    2, NULL, 'b' },
        { "trace",    2, NULL, 't' },
        { "quiet",    0, NULL, 'q' },
        { 0,  0, 0, 0 }
      };

#if (defined(WIN32) || defined(_X64))
      custom_optarg = optarg;
      custom_optind = optind;
      custom_opterr = opterr;
      custom_optopt = optopt;
      c = custom_getopt_long (argc, argv, "hv:P::d:Ra::Ap:D::s:Vr:w:x:o:m:l:L:b::t::q", long_options, &option_index);

      optarg = custom_optarg;
      optind = custom_optind;
      opterr = custom_opterr;
      optopt = custom_optopt;
#else
      c = getopt_long (argc, argv, "hv:P::d:Ra::Ap:D::s:Vr:w:x:o:m:l:L:b::t::q", long_options, &option_index);
#endif

      if (c == -1) break;    /* Exit from `while (1)' loop.  */

      switch (c)
        {
        case 'h':    /* Print help and exit.  */
          cmdline_parser_print_help ();
          cmdline_parser_free (&local_args_info);
          exit (EXIT_SUCCESS);

        case 'v':    /* set volume step.  */

          if (update_multiple_arg_temp(&volume_list,
              &(local_args_info.volume_given), optarg, 0, 0, ARG_INT,
              "volume", 'v',
              additional_error))
            goto failure;

          break;
        case 'P':    /* set the (new) profile for current operation.  */


          if (update_arg( (void *)&(args_info->profile_arg),
               &(args_info->profile_orig), &(args_info->profile_given),
              &(local_args_info.profile_given), optarg, 0, 0, ARG_INT,
              check_ambiguity, override, 0, 0,
              "profile", 'P',
              additional_error))
            goto failure;

          break;
        case 'd':    /* target name for the interface: i2c, serial, socket, i2c dummy.  */


          if (update_arg( (void *)&(args_info->device_arg),
               &(args_info->device_orig), &(args_info->device_given),
              &(local_args_info.device_given), optarg, 0, 0, ARG_STRING,
              check_ambiguity, override, 0, 0,
              "device", 'd',
              additional_error))
            goto failure;

          break;
        case 'R':    /* initialize I2C registers and set ACS. After the command, the state is same power on reset..  */


          if (update_arg( 0 ,
               0 , &(args_info->reset_given),
              &(local_args_info.reset_given), optarg, 0, 0, ARG_NO,
              check_ambiguity, override, 0, 0,
              "reset", 'R',
              additional_error))
            goto failure;

          break;
        case 'a':    /* do calibration with loaded speaker file, --output returns updated speaker file.  */


          if (update_arg( (void *)&(args_info->calibrate_arg),
               &(args_info->calibrate_orig), &(args_info->calibrate_given),
              &(local_args_info.calibrate_given), optarg, 0, "always", ARG_STRING,
              check_ambiguity, override, 0, 0,
              "calibrate", 'a',
              additional_error))
            goto failure;

          break;
        case 'A':    /* show calibration impedance value.  */


          if (update_arg( 0 ,
               0 , &(args_info->calshow_given),
              &(local_args_info.calshow_given), optarg, 0, 0, ARG_NO,
              check_ambiguity, override, 0, 0,
              "calshow", 'A',
              additional_error))
            goto failure;

          break;
        case 'p':    /* write the params file directly to the device; depending on header type: patch, speaker, preset, config, drc, eq .  */

          if (update_multiple_arg_temp(&params_list,
              &(local_args_info.params_given), optarg, 0, 0, ARG_STRING,
              "params", 'p',
              additional_error))
            goto failure;

          break;
        case 'D':    /* DSP get speakerboost params, use --count to set bytecount.  */


          if (update_arg( (void *)&(args_info->dsp_arg),
               &(args_info->dsp_orig), &(args_info->dsp_given),
              &(local_args_info.dsp_given), optarg, 0, "0x80", ARG_INT,
              check_ambiguity, override, 0, 0,
              "dsp", 'D',
              additional_error))
            goto failure;

          break;
        case 's':    /* write settings to binary file without header. the file type extension must be specified as .eq.bin, .speaker.bin, etc.  */


          if (update_arg( (void *)&(args_info->save_arg),
               &(args_info->save_orig), &(args_info->save_given),
              &(local_args_info.save_given), optarg, 0, 0, ARG_STRING,
              check_ambiguity, override, 0, 0,
              "save", 's',
              additional_error))
            goto failure;

          break;
        case 'V':    /* print versions and chip rev.  */


          if (update_arg( 0 ,
               0 , &(args_info->versions_given),
              &(local_args_info.versions_given), optarg, 0, 0, ARG_NO,
              check_ambiguity, override, 0, 0,
              "versions", 'V',
              additional_error))
            goto failure;

          break;
        case 'r':    /* read tfa register, write if extra arg given.  */

          if (update_multiple_arg_temp(&register_list,
              &(local_args_info.register_given), optarg, 0, 0, ARG_INT,
              "register", 'r',
              additional_error))
            goto failure;

          break;
        case 'w':    /* write value for register.  */

          if (update_multiple_arg_temp(&regwrite_list,
              &(local_args_info.regwrite_given), optarg, 0, 0, ARG_INT,
              "regwrite", 'w',
              additional_error))
            goto failure;

          break;
        case 'x':    /* access (read/write) tfa xmem.  */

          if (update_multiple_arg_temp(&xmem_list,
              &(local_args_info.xmem_given), optarg, 0, 0, ARG_INT,
              "xmem", 'x',
              additional_error))
            goto failure;

          break;
        case 'o':    /* specify the output file for binary speaker state info records, default=stdout.  */


          if (update_arg( (void *)&(args_info->output_arg),
               &(args_info->output_orig), &(args_info->output_given),
              &(local_args_info.output_given), optarg, 0, 0, ARG_STRING,
              check_ambiguity, override, 0, 0,
              "output", 'o',
              additional_error))
            goto failure;

          break;
        case 'm':    /* provide the maximus device type.  */


          if (update_arg( (void *)&(args_info->maximus_arg),
               &(args_info->maximus_orig), &(args_info->maximus_given),
              &(local_args_info.maximus_given), optarg, 0, "1", ARG_INT,
              check_ambiguity, override, 0, 0,
              "maximus", 'm',
              additional_error))
            goto failure;

          break;
        case 'l':    /* read parameter settings from container file.  */


          if (update_arg( (void *)&(args_info->load_arg),
               &(args_info->load_orig), &(args_info->load_given),
              &(local_args_info.load_given), optarg, 0, 0, ARG_STRING,
              check_ambiguity, override, 0, 0,
              "load", 'l',
              additional_error))
            goto failure;

          break;
        case 'L':    /* loop the operation [0=forever].  */


          if (update_arg( (void *)&(args_info->loop_arg),
               &(args_info->loop_orig), &(args_info->loop_given),
              &(local_args_info.loop_given), optarg, 0, 0, ARG_INT,
              check_ambiguity, override, 0, 0,
              "loop", 'L',
              additional_error))
            goto failure;

          break;
        case 'b':    /* Enable verbose (mask=timing|i2cserver|socket|scribo).  */


          if (update_arg( (void *)&(args_info->verbose_arg),
               &(args_info->verbose_orig), &(args_info->verbose_given),
              &(local_args_info.verbose_given), optarg, 0, 0, ARG_INT,
              check_ambiguity, override, 0, 0,
              "verbose", 'b',
              additional_error))
            goto failure;

          break;
        case 't':    /* Enable I2C transaction tracing to stdout/file.  */


          if (update_arg( (void *)&(args_info->trace_arg),
               &(args_info->trace_orig), &(args_info->trace_given),
              &(local_args_info.trace_given), optarg, 0, 0, ARG_STRING,
              check_ambiguity, override, 0, 0,
              "trace", 't',
              additional_error))
            goto failure;

          break;
        case 'q':    /* Suppress printing to stdout.  */


          if (update_arg( 0 ,
               0 , &(args_info->quiet_given),
              &(local_args_info.quiet_given), optarg, 0, 0, ARG_NO,
              check_ambiguity, override, 0, 0,
              "quiet", 'q',
              additional_error))
            goto failure;

          break;

        case 0:    /* Long option with no short option */
          if (strcmp (long_options[option_index].name, "full-help") == 0) {
            cmdline_parser_print_full_help ();
            cmdline_parser_free (&local_args_info);
            exit (EXIT_SUCCESS);
          }

          if (strcmp (long_options[option_index].name, "version") == 0) {
            cmdline_parser_print_version ();
            cmdline_parser_free (&local_args_info);
            exit (EXIT_SUCCESS);
          }

          /* device power up and start.  */
          if (strcmp (long_options[option_index].name, "start") == 0)
          {


            if (update_arg( 0 ,
                 0 , &(args_info->start_given),
                &(local_args_info.start_given), optarg, 0, 0, ARG_NO,
                check_ambiguity, override, 0, 0,
                "start", '-',
                additional_error))
              goto failure;

          }
          /* device stop and power down.  */
          else if (strcmp (long_options[option_index].name, "stop") == 0)
          {


            if (update_arg( 0 ,
                 0 , &(args_info->stop_given),
                &(local_args_info.stop_given), optarg, 0, 0, ARG_NO,
                check_ambiguity, override, 0, 0,
                "stop", '-',
                additional_error))
              goto failure;

          }
          /* reset MtpEx register to do re-calibration.  */
          else if (strcmp (long_options[option_index].name, "resetMtpEx") == 0)
          {


            if (update_arg( 0 ,
                 0 , &(args_info->resetMtpEx_given),
                &(local_args_info.resetMtpEx_given), optarg, 0, 0, ARG_NO,
                check_ambiguity, override, 0, 0,
                "resetMtpEx", '-',
                additional_error))
              goto failure;

          }
          /* set specified re0 or read the current re0.  */
          else if (strcmp (long_options[option_index].name, "re0") == 0)
          {


            if (update_arg( (void *)&(args_info->re0_arg),
                 &(args_info->re0_orig), &(args_info->re0_given),
                &(local_args_info.re0_given), optarg, 0, 0, ARG_FLOAT,
                check_ambiguity, override, 0, 0,
                "re0", '-',
                additional_error))
              goto failure;

          }
          /* set the currently (loaded) runing profile to force transition to new profile. This options should be used with profile option.  */
          else if (strcmp (long_options[option_index].name, "currentprof") == 0)
          {


            if (update_arg( (void *)&(args_info->currentprof_arg),
                 &(args_info->currentprof_orig), &(args_info->currentprof_given),
                &(local_args_info.currentprof_given), optarg, 0, 0, ARG_INT,
                check_ambiguity, override, 0, 0,
                "currentprof", '-',
                additional_error))
              goto failure;

          }
          /* Set the tone detection off or on.  */
          else if (strcmp (long_options[option_index].name, "tone") == 0)
          {


            if (update_arg( (void *)&(args_info->tone_arg),
                 &(args_info->tone_orig), &(args_info->tone_given),
                &(local_args_info.tone_given), optarg, 0, 0, ARG_INT,
                check_ambiguity, override, 0, 0,
                "tone", '-',
                additional_error))
              goto failure;

          }
          /* dump all defined registers.  */
          else if (strcmp (long_options[option_index].name, "dump") == 0)
          {


            if (update_arg( 0 ,
                 0 , &(args_info->dump_given),
                &(local_args_info.dump_given), optarg, 0, 0, ARG_NO,
                check_ambiguity, override, 0, 0,
                "dump", '-',
                additional_error))
              goto failure;

          }
          /* control devkit signal pin.  */
          else if (strcmp (long_options[option_index].name, "pin") == 0)
          {


            if (update_arg( (void *)&(args_info->pin_arg),
                 &(args_info->pin_orig), &(args_info->pin_given),
                &(local_args_info.pin_given), optarg, 0, 0, ARG_INT,
                check_ambiguity, override, 0, 0,
                "pin", '-',
                additional_error))
              goto failure;

          }
          /* run all tests, or single if testnr; optional extra argument: [i2s|dsp|sb|pins].  */
          else if (strcmp (long_options[option_index].name, "diag") == 0)
          {


            if (update_arg( (void *)&(args_info->diag_arg),
                 &(args_info->diag_orig), &(args_info->diag_given),
                &(local_args_info.diag_given), optarg, 0, "0", ARG_INT,
                check_ambiguity, override, 0, 0,
                "diag", '-',
                additional_error))
              goto failure;

          }
          /* dump current speakermodel impedance=z or excursion=x.  */
          else if (strcmp (long_options[option_index].name, "dumpmodel") == 0)
          {


            if (update_arg( (void *)&(args_info->dumpmodel_arg),
                 &(args_info->dumpmodel_orig), &(args_info->dumpmodel_given),
                &(local_args_info.dumpmodel_given), optarg, 0, "z", ARG_STRING,
                check_ambiguity, override, 0, 0,
                "dumpmodel", '-',
                additional_error))
              goto failure;

          }
          /* record speaker state info via I2C and display.  */
          else if (strcmp (long_options[option_index].name, "record") == 0)
          {


            if (update_arg( (void *)&(args_info->record_arg),
                 &(args_info->record_orig), &(args_info->record_given),
                &(local_args_info.record_given), optarg, 0, "55", ARG_INT,
                check_ambiguity, override, 0, 0,
                "record", '-',
                additional_error))
              goto failure;

          }
          /* number of read cycles to execute, 0 means forever.  */
          else if (strcmp (long_options[option_index].name, "count") == 0)
          {


            if (update_arg( (void *)&(args_info->count_arg),
                 &(args_info->count_orig), &(args_info->count_given),
                &(local_args_info.count_given), optarg, 0, 0, ARG_INT,
                check_ambiguity, override, 0, 0,
                "count", '-',
                additional_error))
              goto failure;

          }
          /* start datalogger, recording <count> state info lines and binary Z/Xmodels.  */
          else if (strcmp (long_options[option_index].name, "logger") == 0)
          {


            if (update_arg( (void *)&(args_info->logger_arg),
                 &(args_info->logger_orig), &(args_info->logger_given),
                &(local_args_info.logger_given), optarg, 0, "2", ARG_INT,
                check_ambiguity, override, 0, 0,
                "logger", '-',
                additional_error))
              goto failure;

          }
          /* Generate the container file from an ini file <this>.ini to <this>.cnt.  */
          else if (strcmp (long_options[option_index].name, "ini2cnt") == 0)
          {


            if (update_arg( (void *)&(args_info->ini2cnt_arg),
                 &(args_info->ini2cnt_orig), &(args_info->ini2cnt_given),
                &(local_args_info.ini2cnt_given), optarg, 0, 0, ARG_STRING,
                check_ambiguity, override, 0, 0,
                "ini2cnt", '-',
                additional_error))
              goto failure;

          }
          /* Generate a file with header from input binary file.<type> [customer] [application] [type]. Original file is backed up as file.<type>.old file.  */
          else if (strcmp (long_options[option_index].name, "bin2hdr") == 0)
          {


            if (update_arg( (void *)&(args_info->bin2hdr_arg),
                 &(args_info->bin2hdr_orig), &(args_info->bin2hdr_given),
                &(local_args_info.bin2hdr_given), optarg, 0, 0, ARG_STRING,
                check_ambiguity, override, 0, 0,
                "bin2hdr", '-',
                additional_error))
              goto failure;

          }
          /* save parameters of the loaded container file to seperate parameter files.  */
          else if (strcmp (long_options[option_index].name, "splitparms") == 0)
          {


            if (update_arg( 0 ,
                 0 , &(args_info->splitparms_given),
                &(local_args_info.splitparms_given), optarg, 0, 0, ARG_NO,
                check_ambiguity, override, 0, 0,
                "splitparms", '-',
                additional_error))
              goto failure;

          }
          /* run as server (for Linux only, default=`9887').  */
          else if (strcmp (long_options[option_index].name, "server") == 0)
          {


            if (update_arg( (void *)&(args_info->server_arg),
                 &(args_info->server_orig), &(args_info->server_given),
                &(local_args_info.server_given), optarg, 0, "9887", ARG_STRING,
                check_ambiguity, override, 0, 0,
                "server", '-',
                additional_error))
              goto failure;

          }
          /* run as client (for Linux only, default=`9887').  */
          else if (strcmp (long_options[option_index].name, "client") == 0)
          {


            if (update_arg( (void *)&(args_info->client_arg),
                 &(args_info->client_orig), &(args_info->client_given),
                &(local_args_info.client_given), optarg, 0, "9887", ARG_STRING,
                check_ambiguity, override, 0, 0,
                "client", '-',
                additional_error))
              goto failure;

          }
          /* override hardcoded I2C slave address.  */
          else if (strcmp (long_options[option_index].name, "slave") == 0)
          {


            if (update_arg( (void *)&(args_info->slave_arg),
                 &(args_info->slave_orig), &(args_info->slave_given),
                &(local_args_info.slave_given), optarg, 0, 0, ARG_INT,
                check_ambiguity, override, 0, 0,
                "slave", '-',
                additional_error))
              goto failure;

          }

          break;
        case '?':    /* Invalid option.  */
          /* `getopt_long' already printed an error message.  */
          goto failure;

        default:    /* bug: option not considered.  */
          PRINT_ERROR("%s: option unknown: %c%s\n", CMDLINE_PARSER_PACKAGE, c, (additional_error ? additional_error : ""));
          abort ();
        } /* switch */
    } /* while */


  update_multiple_arg((void *)&(args_info->volume_arg),
    &(args_info->volume_orig), args_info->volume_given,
    local_args_info.volume_given, 0,
    ARG_INT, volume_list);
  update_multiple_arg((void *)&(args_info->params_arg),
    &(args_info->params_orig), args_info->params_given,
    local_args_info.params_given, 0,
    ARG_STRING, params_list);
  update_multiple_arg((void *)&(args_info->register_arg),
    &(args_info->register_orig), args_info->register_given,
    local_args_info.register_given, 0,
    ARG_INT, register_list);
  update_multiple_arg((void *)&(args_info->regwrite_arg),
    &(args_info->regwrite_orig), args_info->regwrite_given,
    local_args_info.regwrite_given, 0,
    ARG_INT, regwrite_list);
  update_multiple_arg((void *)&(args_info->xmem_arg),
    &(args_info->xmem_orig), args_info->xmem_given,
    local_args_info.xmem_given, 0,
    ARG_INT, xmem_list);

  args_info->volume_given += local_args_info.volume_given;
  local_args_info.volume_given = 0;
  args_info->params_given += local_args_info.params_given;
  local_args_info.params_given = 0;
  args_info->register_given += local_args_info.register_given;
  local_args_info.register_given = 0;
  args_info->regwrite_given += local_args_info.regwrite_given;
  local_args_info.regwrite_given = 0;
  args_info->xmem_given += local_args_info.xmem_given;
  local_args_info.xmem_given = 0;

  if (check_required)
    {
      error += cmdline_parser_required2 (args_info, argv[0], additional_error);
    }

  cmdline_parser_release (&local_args_info);

  if ( error )
    return (EXIT_FAILURE);

  return 0;

failure:
  free_list (volume_list, 0 );
  free_list (params_list, 1 );
  free_list (register_list, 0 );
  free_list (regwrite_list, 0 );
  free_list (xmem_list, 0 );

  cmdline_parser_release (&local_args_info);
  return (EXIT_FAILURE);
}
