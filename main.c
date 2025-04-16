#define UTIL_IMPL
#include "util.h"

#include "api.h"

const char apikey[] = "your-api-key";

int
main(int argc, char *argv[])
{
    alphavantage_t av;
    alphavantage_init(&av, apikey);

    int rc;
    rc = alphavantage_stock(av, "AAPL", 0, 0, 0, 0);
    rc = alphavantage_stock(av, "NVDA", 0, 0, 0, 0);
    rc = alphavantage_stock(av, "QQQ", 0, 0, 0, 0);
    rc = alphavantage_stock(av, "TSM", 0, 0, 0, 0);
    rc = alphavantage_stock(av, "SPY", 0, 0, 0, 0);

    alphavantage_delete(&av);

    return 0;
}
