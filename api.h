#ifndef API_H
#define API_H

#include <stdbool.h>

#define AV_TIMESERIES_INTRADAY (0)
#define AV_TIMESERIES_DAILY (1)
#define AV_TIMESERIES_DAILY_ADJUSTED (2)
#define AV_TIMESERIES_MONTHLY (3)
#define AV_TIMESERIES_MONTHLY_ADJUSTED (4)

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

void alphavantage_init(alphavantage_t *av, const char *apikey);
void alphavantage_delete(alphavantage_t *av);

int alphavantage_stock(
    alphavantage_t av,
    const char *symbol,
    int function,
    int interval,
    int outputsize,
    int datatype
);


#endif // API_H
