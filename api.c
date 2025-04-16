#include <curl/curl.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#include "api.h"

#include "util.h"

#define USTR_IMPL
#include "ustr.h"

struct avctx_t {
    CURL *curl;
    char apikey[17];
};

static const char *baseurl = "https://www.alphavantage.co/query";

static size_t
writecb(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t totalsize = size*nmemb;
    av_response_t *mem = (av_response_t *)userp;

    char *p = realloc(mem->data, mem->size + totalsize + 1);
    if (!p) {
        ulog(UPERR, "%s: realloc failed", __func__);
        return 0;
    }

    mem->data = p;
    memcpy(&(mem->data[mem->size]), contents, totalsize);
    mem->size += totalsize;
    mem->data[mem->size] = '\0';

    return totalsize;
}

int
alphavantage_init(alphavantage_t *av, const char *apikey)
{
    UASSERT(av);
    UASSERT(*av == NULL);
    UASSERT(apikey);
    UASSERT(strlen(apikey) == 16);

    CURL *curl;
    CURLcode rc;

    rc = curl_global_init(CURL_GLOBAL_DEFAULT);
    if (rc) {
        ulog(UFATL, "curl_global_init -> %s", curl_easy_strerror(rc));
        return -1;
    }

    curl = curl_easy_init();
    if (!curl) {
        ulog(UFATL, "curl_easy_init -> %s", curl_easy_strerror(rc));
        return -1;
    }

    rc = curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, (long)CURL_HTTP_VERSION_3);
    if (rc) {
        ulog(UFATL, "curl_easy_setopt, CURLOPT_HTTP_VERSION -> %s", curl_easy_strerror(rc));
        return -1;
    }

    rc = curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
    if (rc) {
        ulog(UFATL, "curl_easy_setopt, CURLOPT_USERAGENT -> %s", curl_easy_strerror(rc));
        return -1;
    }

    rc = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writecb);
    if (rc) {
        ulog(UFATL, "curl_easy_setopt, CURLOPT_WRITEFUNCTION -> %s", curl_easy_strerror(rc));
        return -1;
    }

    struct avctx_t *ctx = malloc(sizeof(*ctx));
    UASSERT(ctx);

    ctx->curl = curl;

    strncpy(ctx->apikey, apikey, 16);
    ctx->apikey[16] = '\0';

    *av = (alphavantage_t)ctx;

    return 0;
}

void
alphavantage_delete(alphavantage_t *av)
{
    UASSERT(av);
    UASSERT(*av);
    struct avctx_t *ctx = (struct avctx_t *)*av;
    curl_easy_cleanup(ctx->curl);
    curl_global_cleanup();
    memset(ctx, 0, sizeof(*ctx));
    free(ctx);
    *av = NULL;
}

static const char *param_function(int function) {
    switch (function) {
    case AV_TIME_SERIES_INTRADAY: return "function=TIME_SERIES_INTRADAY";
    case AV_TIME_SERIES_DAILY: return "function=TIME_SERIES_DAILY";
    case AV_TIME_SERIES_DAILY_ADJUSTED: return "function=TIME_SERIES_DAILY_ADJUSTED";
    case AV_TIME_SERIES_WEEKLY: return "function=TIME_SERIES_WEEKLY";
    case AV_TIME_SERIES_WEEKLY_ADJUSTED: return "function=TIME_SERIES_WEEKLY_ADJUSTED";
    case AV_TIME_SERIES_MONTHLY: return "function=TIME_SERIES_MONTHLY";
    case AV_TIME_SERIES_MONTHLY_ADJUSTED: return "function=TIME_SERIES_MONTHLY_ADJUSTED";
    default: return "function=TIME_SERIES_INTRADAY";
    }
}

static const char *param_interval(int interval) {
    switch (interval) {
    case AV_INTERVAL_1MIN: return "interval=1min";
    case AV_INTERVAL_5MIN: return "interval=5min";
    case AV_INTERVAL_15MIN: return "interval=15min";
    case AV_INTERVAL_30MIN: return "interval=30min";
    case AV_INTERVAL_60MIN: return "interval=60min";
    default: return "interval=60min";
    }
}

static const char *param_outputsize(int outputsize) {
    switch (outputsize) {
    case AV_OUTPUTSIZE_COMPACT: return "outputsize=compact";
    case AV_OUTPUTSIZE_FULL: return "outputsize=full";
    default: return "outputsize=compact";
    }
}

static const char *param_datatype(int datatype) {
    switch (datatype) {
    case AV_DATATYPE_JSON: return "datatype=json";
    case AV_DATATYPE_CSV: return "datatype=csv";
    default: return "datatype=json";
    }
}

static const char *param_adjusted(int adjusted) {
    switch (adjusted) {
    case AV_ADJUSTED_TRUE: return "adjusted=true";
    case AV_ADJUSTED_FALSE: return "adjusted=false";
    default: return "adjusted=true";
    }
}

static const char *param_extended_hours(int extended_hours) {
    switch (extended_hours) {
    case AV_EXTENDED_HOURS_TRUE: return "extended_hours=true";
    case AV_EXTENDED_HOURS_FALSE: return "extended_hours=false";
    default: return "extended_hours=true";
    }
}

static bool
valid_month_format(const char *month)
{
    UASSERT(month);
    return (strlen(month) == 7) && (isnumber(month[0])) &&
           (isnumber(month[1])) && (isnumber(month[2])) &&
           (isnumber(month[3])) && (month[4] == '-') &&
           (isnumber(month[5])) && (isnumber(month[6]));
}

static const char *
build_query(
    ustr_builder_t *b,
    const char *apikey,
    const char *symbol,
    const char *month,
    int function,
    int interval,
    int adjusted,
    int extended_hours,
    int outputsize,
    int datatype)
{
    UASSERT(apikey);
    UASSERT(symbol);

    const char *url;

    // begin query with base url
    url = ustr_builder_puts(b, baseurl);

    switch (function) {
    case AV_TIME_SERIES_INTRADAY:
        ustr_builder_putc(b, '?');
        ustr_builder_puts(b, param_function(function));
        ustr_builder_puts(b, "&apikey=");
        ustr_builder_puts(b, apikey);
        ustr_builder_puts(b, "&symbol=");
        ustr_builder_puts(b, symbol);
        if (interval) {
            ustr_builder_putc(b, '&');
            ustr_builder_puts(b, param_interval(interval));
        }
        if (adjusted) {
            ustr_builder_putc(b, '&');
            ustr_builder_puts(b, param_adjusted(adjusted));
        }
        if (outputsize) {
            ustr_builder_putc(b, '&');
            ustr_builder_puts(b, param_outputsize(outputsize));
        }
        if (datatype) {
            ustr_builder_putc(b, '&');
            ustr_builder_puts(b, param_datatype(datatype));
        }
        if (month && valid_month_format(month)) {
            ustr_builder_puts(b, "&month=");
            ustr_builder_puts(b, month);
        }
        ustr_builder_putc(b, '\0');
        break;

    case AV_TIME_SERIES_DAILY:
    case AV_TIME_SERIES_DAILY_ADJUSTED:
        ustr_builder_putc(b, '?');
        ustr_builder_puts(b, param_function(function));
        ustr_builder_puts(b, "&apikey=");
        ustr_builder_puts(b, apikey);
        ustr_builder_puts(b, "&symbol=");
        ustr_builder_puts(b, symbol);
        if (outputsize) {
            ustr_builder_putc(b, '&');
            ustr_builder_puts(b, param_outputsize(outputsize));
        }
        if (datatype) {
            ustr_builder_putc(b, '&');
            ustr_builder_puts(b, param_datatype(datatype));
        }
        ustr_builder_putc(b, '\0');
        break;

    case AV_TIME_SERIES_WEEKLY:
    case AV_TIME_SERIES_WEEKLY_ADJUSTED:
    case AV_TIME_SERIES_MONTHLY:
    case AV_TIME_SERIES_MONTHLY_ADJUSTED:
        ustr_builder_putc(b, '?');
        ustr_builder_puts(b, param_function(function));
        ustr_builder_puts(b, "&apikey=");
        ustr_builder_puts(b, apikey);
        ustr_builder_puts(b, "&symbol=");
        ustr_builder_puts(b, symbol);
        if (datatype) {
            ustr_builder_putc(b, '&');
            ustr_builder_puts(b, param_datatype(datatype));
        }
        ustr_builder_putc(b, '\0');
        break;

    default:
        ulog(UWARN, "unimplemented function %d", function);
        break;
    }

    return url;
}

int
alphavantage(
    alphavantage_t av,
    const char *symbol,
    const char *month,
    int function,
    int interval,
    int adjusted,
    int extended_hours,
    int outputsize,
    int datatype,
    av_response_t *response)
{
    UASSERT(symbol);

    struct avctx_t *ctx = (struct avctx_t *)av;
    ustr_builder_t b;
    CURLcode rc;
    const char *url;

    ustr_builder_alloc(&b);

    url = build_query(&b, ctx->apikey, symbol, month, function,
                      interval, adjusted, extended_hours,
                      outputsize, datatype);
  
    ulog(UINFO, "url: %s", url);

    // register url
    rc = curl_easy_setopt(ctx->curl, CURLOPT_URL, url);
    if (rc) {
        ulog(UFATL, "curl_easy_setopt, CURLOPT_URL -> %d", rc);
        return -1;
    }

    // create memory for response
    response->size = 0;
    response->data = malloc(1);
    UASSERT(response->data);
    rc = curl_easy_setopt(ctx->curl, CURLOPT_WRITEDATA, (void *)response);
    if (rc) {
        free(response->data);
        response->data = NULL;
        ulog(UFATL, "curl_easy_setopt, CURLOPT_WRITEDATA -> %s", curl_easy_strerror(rc));
        return -1;
    }

    rc = curl_easy_perform(ctx->curl);
    if (rc) {
        ulog(UFATL, "curl_easy_perform -> %s", curl_easy_strerror(rc));
        return -1;
    }

    ustr_builder_free(&b);
    return 0;
}

