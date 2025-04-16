# Overview
Wrapper to the [AlphaVantage](https://www.alphavantage.co/) API for C and C++.

(still under construction)

# Usage
```C
// api instance handle
alphavantage_t avapi;

// initialize api instance
rc = alphavantage_init(&avapi, "your-api-key");

// request data
av_response_t response;

// configure request with the enumerated AV_ values in api.h
rc = alphavantage(&avapi, ..., &response);

// received `response.size` bytes in `response.data`

// delete api instance
alphavantage_delete(&avapi);
```
