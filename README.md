<img src="/img/anynome-logo.png" align="right" style="float:right;">

# Anynome

## Overview
Anynome is a generalized metronome, which displays audible or visual stimuli according to an arbitrary schedule.  Additionally, keypress times are logged during the presentation, which enables data to be collected on reaction times or similar statistics.

Scheduling is accomplished via plugins which generate inter-stimulus interval time series. For example, for a standard metronome, this series would be made up of a single number repeated some number of times.

The plugin framework, however, allows more complicated stimulus schedules, such as random, chaotic, drifting, or sinusoidal.  Any array of numbers generatable by a program written in C will work!

## Plugins

Making new plugins is easy.  Each plugin must implement the following functions:
```C
void set_arg(void *config, dict_entry_t *entry, dict_entry_value_t value);
void get_delay_series(void *config, uint32_t *delay_data, int num_events);
```

and define its data dictionary:

```C
/*
 * Create names for your configuration keywords.
 * If they are duplicates of main keywords, they
 * will be ignored.
 */
enum {
    CONF_EXAMPLE,
    CONF_END
} config_keywords;

/*
 * Create the dictionary of keywords.
 * For each keyword, define its actual string value, type, and default.
 */
dict_entry_t plugin_dict_entries[] = {
    {CONF_EXAMPLE,           "example",           DICT_TYPE_INT, {.int_val=1}}
};
```

See plugin\_template.c for details.

**set\_arg()**
Each plugin may have its own configuration file keywords. As the main program parses the configuration file, it passes these keywords to the plugin for processing via `set_arg()`.

**get\_delay\_series()**
This function is responsible for filling in the `delay_data` array with `num_events` number of inter-stimulus delay times. The array is expected to be a series of *microseconds*.

## Changelog

0.10.0
------
- Arbitrary number of concurrent stimuli
- Plugin specification by configuration file, rather than command line
- Support for different colors
Bugs:
- This version includes its own audio mixer, which needs work to
  remove some crackling noises.

0.9.2
-----
- Add plugin api version checking
- Update event loop to increase timing resolution

0.9.1
-----
- Add ability to test timing resolution (-t option)

0.9.0
-----
- Add ability to load plugins
- First released version

