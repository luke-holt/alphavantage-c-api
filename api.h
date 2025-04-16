#ifndef API_H
#define API_H

#include <stdbool.h>
#include <stddef.h>

#define AV_TIME_SERIES_INTRADAY (0)
#define AV_TIME_SERIES_DAILY (1)
#define AV_TIME_SERIES_DAILY_ADJUSTED (2)
#define AV_TIME_SERIES_WEEKLY_ADJUSTED (3)
#define AV_TIME_SERIES_WEEKLY (4)
#define AV_TIME_SERIES_MONTHLY (5)
#define AV_TIME_SERIES_MONTHLY_ADJUSTED (6)

#define AV_INTERVAL_1MIN (0)
#define AV_INTERVAL_5MIN (1)
#define AV_INTERVAL_15MIN (2)
#define AV_INTERVAL_30MIN (3)
#define AV_INTERVAL_60MIN (5)

#define AV_OUTPUTSIZE_COMPACT (0)
#define AV_OUTPUTSIZE_FULL (1)

#define AV_DATATYPE_JSON (0)
#define AV_DATATYPE_CSV (1)

#define AV_ADJUSTED_TRUE (0)
#define AV_ADJUSTED_FALSE (1)

#define AV_EXTENDED_HOURS_TRUE (0)
#define AV_EXTENDED_HOURS_FALSE (1)

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
    int function,
    int interval,
    int adjusted,
    int extended_hours,
    int outputsize,
    int datatype,
    av_response_t *response
);

#endif // API_H
