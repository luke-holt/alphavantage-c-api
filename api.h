#ifndef API_H
#define API_H

#include <stdbool.h>
#include <stddef.h>

typedef enum {
    AV_TIME_SERIES_INTRADAY,
    AV_TIME_SERIES_DAILY,
    AV_TIME_SERIES_DAILY_ADJUSTED,
    AV_TIME_SERIES_WEEKLY_ADJUSTED,
    AV_TIME_SERIES_WEEKLY,
    AV_TIME_SERIES_MONTHLY,
    AV_TIME_SERIES_MONTHLY_ADJUSTED,
} av_function_t;

typedef enum {
    AV_INTERVAL_1MIN,
    AV_INTERVAL_5MIN,
    AV_INTERVAL_15MIN,
    AV_INTERVAL_30MIN,
    AV_INTERVAL_60MIN,
} av_interval_t;

typedef enum {
    AV_OUTPUTSIZE_COMPACT,
    AV_OUTPUTSIZE_FULL,
} av_outputsize_t;

typedef enum {
    AV_DATATYPE_JSON,
    AV_DATATYPE_CSV,
} av_datatype_t;

typedef enum {
    AV_ADJUSTED_TRUE,
    AV_ADJUSTED_FALSE,
} av_adjusted_t;

typedef enum {
    AV_EXTENDED_HOURS_TRUE,
    AV_EXTENDED_HOURS_FALSE,
} av_extended_hours_t;

typedef void *alphavantage_t;

typedef struct {
    size_t size;
    char *data;
} av_response_t;

// returns 0 on success and -1 on failure
int alphavantage_init(alphavantage_t *av, const char *apikey);

void alphavantage_delete(alphavantage_t *av);

// returns 0 on success and -1 on failure
// `response` is allocated by this function and must be freed by the caller
int alphavantage(
    alphavantage_t av,
    const char *symbol,
    const char *month,
    av_function_t function,
    av_interval_t interval,
    av_adjusted_t adjusted,
    av_extended_hours_t extended_hours,
    av_outputsize_t outputsize,
    av_datatype_t datatype,
    av_response_t *response
);

#endif // API_H
