#define UTIL_IMPL
#include "util.h"

#include "api.h"

const char apikey[] = "your-api-key";

int
main(int argc, char *argv[])
{
    if (argc != 2) {
        ulog(UNONE, "Usage: %s <ticker>", argv[0]);
        return 1;
    }

    int rc;
    alphavantage_t av;
    av_response_t response;

    rc = alphavantage_init(&av, apikey);

    if (rc < 0)
        return rc;

    rc = alphavantage(av,
            argv[1],
            NULL,
            AV_TIME_SERIES_INTRADAY,
            AV_INTERVAL_60MIN,
            AV_ADJUSTED_TRUE,
            AV_EXTENDED_HOURS_TRUE,
            AV_OUTPUTSIZE_COMPACT,
            AV_DATATYPE_CSV,
            &response);

    ulog(UINFO, "%s", response.data);
    ulog(UINFO, "got %zu bytes", response.size);

    alphavantage_delete(&av);
    free(response.data);

    return 0;
}
