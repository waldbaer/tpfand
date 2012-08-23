/*
 * ThinkPad Fan Regulator
 * Daemon which keeps the CPU on a constant temperature by regulating the fan speed.
 *
 * (c) 2012 Sebastian Waldvogel
 * s.waldvogel /at/ xeotec.de
 *
 * Version history:
 * 0.9.0  Initial version
 */

#include "stdincludes.h"
#include "TpTemperature.h"
#include "TpFan.h"
#include "TpTempRegulator.h"
#if defined(HAVE_CONFIG_H)
# include "config.h"
#endif

/*************************************************************************************************/
/* Constants                                                                                     */
/*************************************************************************************************/
#if !defined(HAVE_CONFIG_H)
# define APPLICATION_NAME      "tpfand"
# define APPLICATION_VERSION   "0.9.0"
#endif

/** Config file parameters **/
static const std::string CFG_FILE_PATH = "/etc/tpfand.conf";
/* Groups */
static const string CFG_GRP_DEVICE = "DeviceConfig";
static const string CFG_GRP_REGULATOR = "RegulatorConfig";

/* group 'DeviceConfig' */
static const string CFG_TEMPSENSORS = "temp_sensors";
static const string CFG_FANDEVICE = "fan_device";
/* group 'RegulatorConfig' */
static const string CFG_NOM_TEMP = "nominal_temperature";
static const string CFG_KP = "kP";
static const string CFG_KD = "kD";
static const string CFG_SAMPLE_TIME = "sample_time";
static const string CFG_REG_PRESCALE = "regulation_prescaler";

/** Config file parameters - Default values **/
static const string CFG_TEMPSENSORS_DEFAULT =
    "/sys/devices/platform/coretemp.0/temp2_input;/sys/devices/platform/coretemp.0/temp3_input";
static const string CFG_FANDEVICE_DEFAULT = "/proc/acpi/ibm/fan";

static const double CFG_NOM_TEMP_DEFAULT = 65.0F;
static const double CFG_KP_DEFAULT = -0.125;
static const double CFG_KD_DEFAULT = 0.25;
static const double CFG_SAMPLE_TIME_DEFAULT = 1;
static const double CFG_REG_PRESCALE_DEFAULT = 5;

/*************************************************************************************************/
/* Types                                                                                         */
/*************************************************************************************************/
typedef struct
{
  gchar *tempdevice;
  gchar *fandevice;

  gdouble nom_temp;
  gdouble kP;
  gdouble kD;
  guint sample_time;
  guint reg_prescaler;
} rccfgType;

typedef struct
{
  gboolean daemon;
  gboolean verbose;
  gboolean version;
} cloptionsType;

/*************************************************************************************************/
/* Local function prototypes                                                                     */
/*************************************************************************************************/
static void setupDaemon();
static rccfgType parseRcConfig();
static cloptionsType parseCommandLineOptions(int argc, char *argv[]);
static void process(TpTempRegulator *pTempRegulator);
static void log_handler(const gchar *log_domain, GLogLevelFlags log_level, const gchar *message, gpointer user_data);
static void dummy_log_handler(const gchar *log_domain, GLogLevelFlags log_level, const gchar *message,
                              gpointer user_data);

/*************************************************************************************************/
/* Global variables                                                                              */
/*************************************************************************************************/

/*************************************************************************************************/
/* Local functions                                                                               */
/*************************************************************************************************/
static void setupDaemon()
{
  /**** Fork off the parent process ****/
  g_message("Starting as daemon...");
  pid_t pid = fork();
  if(pid < 0)
  {
    cout << "failed." << endl;
    exit(EXIT_FAILURE);
  }
  /* Exit parent if PID request was successful */
  if(pid > 0)
  {
    exit(EXIT_SUCCESS);
  }

  /**** Change the file mode mask ****/
  umask(0);

  /**** Register session ID ****/
  pid_t sid = setsid();
  if(sid < 0)
  {
    exit(EXIT_FAILURE);
  }

  /**** Change working dir ****/
  if((chdir("/")) < 0)
  {
    exit(EXIT_FAILURE);
  }
}

static rccfgType parseRcConfig()
{
  rccfgType config;

  GKeyFile *key_file = g_key_file_new();
  GError *error = NULL;
  /* Create new config if not yet existing */
  if(!g_file_test(CFG_FILE_PATH.c_str(), G_FILE_TEST_EXISTS))
  {
    g_message("Creating config file '%s'.", CFG_FILE_PATH.c_str());
    g_key_file_set_string(key_file, CFG_GRP_DEVICE.c_str(), CFG_TEMPSENSORS.c_str(), CFG_TEMPSENSORS_DEFAULT.c_str());
    g_key_file_set_string(key_file, CFG_GRP_DEVICE.c_str(), CFG_FANDEVICE.c_str(), CFG_FANDEVICE_DEFAULT.c_str());

    g_key_file_set_double(key_file, CFG_GRP_REGULATOR.c_str(), CFG_NOM_TEMP.c_str(), CFG_NOM_TEMP_DEFAULT);
    g_key_file_set_double(key_file, CFG_GRP_REGULATOR.c_str(), CFG_KP.c_str(), CFG_KP_DEFAULT);
    g_key_file_set_double(key_file, CFG_GRP_REGULATOR.c_str(), CFG_KD.c_str(), CFG_KD_DEFAULT);
    g_key_file_set_double(key_file, CFG_GRP_REGULATOR.c_str(), CFG_SAMPLE_TIME.c_str(), CFG_SAMPLE_TIME_DEFAULT);
    g_key_file_set_double(key_file, CFG_GRP_REGULATOR.c_str(), CFG_REG_PRESCALE.c_str(), CFG_REG_PRESCALE_DEFAULT);
    gsize rclen;
    gchar* rcontent = g_key_file_to_data(key_file, &rclen, &error);

    ofstream rc;
    rc.open(CFG_FILE_PATH.c_str());
    if(rc.good())
    {
      rc.write(rcontent, rclen);
      rc.close();
    }
    else
    {
      rc.close();
      stringstream msg;
      msg << "Failed to create config file " << CFG_FILE_PATH.c_str() << ". Maybe root privileges required.";
      throw runtime_error(msg.str());
    }
  }

  /* Parse config file */
  if(g_key_file_load_from_file(key_file, CFG_FILE_PATH.c_str(), G_KEY_FILE_KEEP_COMMENTS, &error) == TRUE)
  {
    /* Device configuration */
    config.tempdevice = g_key_file_get_string(key_file, CFG_GRP_DEVICE.c_str(), CFG_TEMPSENSORS.c_str(), &error);
    if(error != NULL)
    {
      g_error(
          "Failed to get configuration of temperature devices. Please specify via option '%s'.", CFG_TEMPSENSORS.c_str());
    }

    config.fandevice = g_key_file_get_string(key_file, CFG_GRP_DEVICE.c_str(), CFG_FANDEVICE.c_str(), &error);
    if(error != NULL)
    {
      g_error("Failed to get configuration of fan device. Please specify via option '%s'.", CFG_FANDEVICE.c_str());
    }

    config.nom_temp = g_key_file_get_double(key_file, CFG_GRP_REGULATOR.c_str(), CFG_NOM_TEMP.c_str(), &error);
    if(error != NULL)
    {
      g_error(
          "Failed to get configuration of nominal temperature. Please specify via option '%s'.", CFG_NOM_TEMP.c_str());
    }

    config.kP = g_key_file_get_double(key_file, CFG_GRP_REGULATOR.c_str(), CFG_KP.c_str(), &error);
    if(error != NULL)
    {
      g_error("Failed to get configuration of kP gain. Please specify via option '%s'.", CFG_KP.c_str());
    }

    config.kD = g_key_file_get_double(key_file, CFG_GRP_REGULATOR.c_str(), CFG_KD.c_str(), &error);
    if(error != NULL)
    {
      g_error("Failed to get configuration of kD gain. Please specify via option '%s'.", CFG_KD.c_str());
    }

    config.sample_time = g_key_file_get_integer(key_file, CFG_GRP_REGULATOR.c_str(), CFG_SAMPLE_TIME.c_str(), &error);
    if(error != NULL)
    {
      g_error("Failed to get configuration of sample time. Please specify via option '%s'.", CFG_SAMPLE_TIME.c_str());
    }

    config.reg_prescaler = g_key_file_get_integer(key_file, CFG_GRP_REGULATOR.c_str(), CFG_REG_PRESCALE.c_str(),
                                                  &error);
    if(error != NULL)
    {
      g_error(
          "Failed to get configuration of regulator prescaler. Please specify via option '%s'.", CFG_REG_PRESCALE.c_str());
    }
  }
  else
  {
    g_error("Failed to load configuration from %s.", CFG_FILE_PATH.c_str());
  }
  return config;
}

static cloptionsType parseCommandLineOptions(int argc, char *argv[])
{
  /* Default command line options */
  cloptionsType cloptions;
  cloptions.daemon = false;
  cloptions.verbose = false;
  cloptions.version = false;

  GOptionEntry entries[] =
  {
  {"daemon", 'd', 0, G_OPTION_ARG_NONE, &(cloptions.daemon), "start as daemon", NULL},
   {"verbose", 'v', 0, G_OPTION_ARG_NONE, &(cloptions.verbose), "be verbose", NULL},
   {"version", 0, 0, G_OPTION_ARG_NONE, &(cloptions.version), "print application version and exit", NULL},
   {NULL}};

  /* Command line options */
  GOptionContext *context;
  GError *error = NULL;
  context = g_option_context_new(NULL);
  g_assert(context != NULL);
  g_option_context_set_help_enabled(context, TRUE);
  g_option_context_set_summary(context, "Fan regulation to hold constant CPU temperature.");
  g_option_context_add_main_entries(context, entries, NULL);

  /* adds GTK+ options */
  if(g_option_context_parse(context, &argc, &argv, &error) == FALSE)
  {
    if(error != NULL)
      g_error("Failed to parse command line options. %s.", error->message);
    else
      g_error("Unknown command line options. See --help.");
    error = NULL;
  }
  return cloptions;
}

static void process(TpTempRegulator *pTempRegulator)
{
  if(pTempRegulator != NULL)
  {
    pTempRegulator->process();
  }
}

static void log_handler(const gchar *log_domain, GLogLevelFlags log_level, const gchar *message, gpointer user_data)
{
  cout << message << endl;
}
static void dummy_log_handler(const gchar *log_domain, GLogLevelFlags log_level, const gchar *message,
                              gpointer user_data)
{
  /* Suppress any output */
}

/*************************************************************************************************/
/* MAIN function                                                                                */
/*************************************************************************************************/

/************************
 * Tutorial: http://www.netzmafia.de/skripten/unix/linux-daemon-howto.html
 ***********************/
int main(int argc, char *argv[])
{
  try
  {
    /* Configure logging handler */
    g_log_set_handler(G_LOG_DOMAIN, G_LOG_LEVEL_MESSAGE, log_handler, NULL);

    /* Parse command line / config file options */
    cloptionsType cloptions = parseCommandLineOptions(argc, argv);
    rccfgType rcoptions = parseRcConfig();

    /* Print version info */
    if(cloptions.version)
    {
      g_message("%s version: %s", APPLICATION_NAME, APPLICATION_VERSION);
      exit(EXIT_SUCCESS);
    }

    /* Configure logging/outputs */
    if(!cloptions.verbose)
    {
      g_log_set_handler(G_LOG_DOMAIN, G_LOG_LEVEL_MESSAGE, dummy_log_handler, NULL);
    }

    /* Print some options */
    g_message("Config options:");
    g_message(
        "Nom.Temp: %.2f°C   kP=%.4f   kD=%.4f   Sampling time: %ds   Regulation prescaler: %ds", rcoptions.nom_temp, rcoptions.kP, rcoptions.kD, rcoptions.sample_time, rcoptions.reg_prescaler);

    /* Start as daemon if configured */
    if(cloptions.daemon)
    {
      g_message("Starting as daemon...");
      setupDaemon();
    }

    vector<string> tempSensorsCfg;
    stringstream ss(rcoptions.tempdevice);
    string tempsensor;
    while(std::getline(ss, tempsensor, ';'))
    {
      tempSensorsCfg.push_back(tempsensor);
    }

    /* Init  Temperature Sensor */
    TpTemperature *pTempSensor = new TpTemperature(&tempSensorsCfg, 3);
    /* Init Fan access */
    TpFan *pFan = new TpFan(rcoptions.fandevice);
    /* Init Regulator */
    TpTempRegulator *pTempRegulator = new TpTempRegulator(pTempSensor, pFan, rcoptions.sample_time,
                                                          rcoptions.reg_prescaler, rcoptions.nom_temp, rcoptions.kP,
                                                          rcoptions.kD);

    /* The Big Loop */
    while(1)
    {
      process(pTempRegulator);
      sleep(rcoptions.sample_time);
    }
  }
  catch(invalid_argument &iae)
  {
    g_error("Invalid argument: %s", iae.what());
  }
  catch(runtime_error &rte)
  {
    g_error("Runtime error: %s", rte.what());
  }
  catch(...)
  {
    g_error("Unknown exception has occured.");
  }
  exit(EXIT_SUCCESS);
}

