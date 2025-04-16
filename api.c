#include <curl/curl.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#include "api.h"

#include "util.h"

#define USTR_IMPL
#include "ustr.h"

struct curlmem_t {
    size_t size;
    char *data;
};

struct avctx_t {
    CURL *curl;
    char apikey[17];
    struct curlmem_t curlmem;
};

static const char *baseurl = "https://www.alphavantage.co/query";

static size_t
writecb(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t totalsize = size*nmemb;
    struct curlmem_t *mem = (struct curlmem_t *)userp;

    char *p = realloc(mem->data, mem->size + totalsize + 1);
    if (!p) {
        ulog(UPERR, "writecb: realloc failed");
        return 0;
    }

    mem->data = p;
    memcpy(&(mem->data[mem->size]), contents, totalsize);
    mem->size += totalsize;
    mem->data[mem->size] = '\0';

    return totalsize;
}

void
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
        return;
    }

    curl = curl_easy_init();
    if (!curl) {
        ulog(UFATL, "curl_easy_init -> %s", curl_easy_strerror(rc));
        return;
    }

    rc = curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, (long)CURL_HTTP_VERSION_3);
    if (rc) {
        ulog(UFATL, "curl_easy_setopt, CURLOPT_HTTP_VERSION -> %s", curl_easy_strerror(rc));
        return;
    }

    rc = curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
    if (rc) {
        ulog(UFATL, "curl_easy_setopt, CURLOPT_USERAGENT -> %s", curl_easy_strerror(rc));
        return;
    }

    rc = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writecb);
    if (rc) {
        ulog(UFATL, "curl_easy_setopt, CURLOPT_WRITEFUNCTION -> %s", curl_easy_strerror(rc));
        return;
    }

    struct avctx_t *ctx = malloc(sizeof(*ctx));
    UASSERT(ctx);

    ctx->curl = curl;
    UASSERT(ctx->curlmem.data);

    strncpy(ctx->apikey, apikey, 16);
    ctx->apikey[16] = '\0';

    *av = (alphavantage_t)ctx;
}

void
alphavantage_delete(alphavantage_t *av)
{
    UASSERT(av);
    UASSERT(*av);

    struct avctx_t *ctx = (struct avctx_t *)*av;

    curl_easy_cleanup(ctx->curl);
    curl_global_cleanup();
    if (ctx->curlmem.data)
        free(ctx->curlmem.data);

    memset(ctx, 0, sizeof(*ctx));
    free(ctx);
    *av = NULL;
}

static const char *param_function(int function) {
    switch (function) {
    case AV_TIMESERIES_INTRADAY: return "function=TIMESERIES_INTRADAY";
    case AV_TIMESERIES_DAILY: return "function=TIMESERIES_DAILY";
    case AV_TIMESERIES_DAILY_ADJUSTED: return "function=TIMESERIES_DAILY_ADJUSTED";
    case AV_TIMESERIES_MONTHLY: return "function=TIMESERIES_MONTHLY";
    case AV_TIMESERIES_MONTHLY_ADJUSTED: return "function=TIMESERIES_MONTHLY_ADJUSTED";
    default: return "function=TIMESERIES_INTRADAY";
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

int alphavantage_stock(
    alphavantage_t av,
    const char *symbol,
    int function,
    int interval,
    int outputsize,
    int datatype
)
{
    struct avctx_t *ctx = (struct avctx_t *)av;
    CURLcode rc;

    ustr_builder_t b;
    ustr_builder_alloc(&b);

    char *url = ustr_builder_puts(&b, baseurl);
    ustr_builder_putc(&b, '?');
    ustr_builder_puts(&b, param_function(function));
    ustr_builder_putc(&b, '&');
    ustr_builder_puts(&b, param_interval(interval));
    ustr_builder_putc(&b, '&');
    ustr_builder_puts(&b, param_outputsize(outputsize));
    ustr_builder_putc(&b, '&');
    ustr_builder_puts(&b, param_datatype(datatype));
    ustr_builder_putc(&b, '&');
    ustr_builder_printf(&b, "symbol=%s", symbol);
    ustr_builder_putc(&b, '&');
    ustr_builder_printf(&b, "apikey=%s", ctx->apikey);
    ustr_builder_putc(&b, '\0');

    // register url
    rc = curl_easy_setopt(ctx->curl, CURLOPT_URL, url);
    if (rc) {
        ulog(UFATL, "curl_easy_setopt, CURLOPT_URL -> %d", rc);
        return -1;
    }

    // create memory for response
    ctx->curlmem.size = 0;
    ctx->curlmem.data = malloc(1);
    rc = curl_easy_setopt(ctx->curl, CURLOPT_WRITEDATA, (void *)&ctx->curlmem);
    if (rc) {
        free(ctx->curlmem.data);
        ctx->curlmem.data = NULL;

        ulog(UFATL, "curl_easy_setopt, CURLOPT_WRITEDATA -> %s", curl_easy_strerror(rc));
        return -1;
    }

    rc = curl_easy_perform(ctx->curl);
    if (rc) {
        ulog(UFATL, "curl_easy_perform -> %s", curl_easy_strerror(rc));
        return -1;
    }

    ulog(UINFO, "url: %s", url);
    ulog(UINFO, "got %zu bytes", ctx->curlmem.size);

    // free response memory
    free(ctx->curlmem.data);
    ctx->curlmem.data = NULL;
    ustr_builder_free(&b);

    return 0;
}

