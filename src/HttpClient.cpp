#include <iostream>
#include <string.h>
#include "Log.h"
#include "HttpClient.h"

HttpClient::HttpClient()
{
    curl_global_init(CURL_GLOBAL_ALL);
    curl_handle = curl_easy_init();
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, callback);

    //curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, "Accept: image/gif, image/x-bitmap, image/jpeg, image/pjpeg");
    //curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, "Connection: Keep-Alive");
    //curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, "Content-type: application/x-www-form-urlencoded;charset=UTF-8");
    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, USERAGENT);
    curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);

    curl_easy_setopt(curl_handle, CURLOPT_NOSIGNAL, 0);
    curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT_MS, 120000 );

//    curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, 1L);
//    curl_easy_setopt(curl_handle, CURLOPT_NOPROXY, "192.168.127.252");
    downloadSize = 0;
}

HttpClient::~HttpClient()
{
    curl_global_cleanup();
}


struct MemoryStruct
{
    char *memory;
    size_t size;
};

size_t HttpClient::callback(void *contents, size_t csize, size_t nmemb, void *data)
{
    size_t realsize = csize * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)data;

    mem->memory = (char*)realloc(mem->memory, mem->size + realsize + 1);
    if(mem->memory == NULL)
    {
        Log::err("not enough memory (realloc returned NULL)\n");
        return 0;
    }

    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

std::string HttpClient::get(const std::string &url)
{
    Log::gdb("get url: %s", url.c_str());

    struct MemoryStruct chunk;

    chunk.memory = (char*)malloc(1);
    chunk.size = 0;

    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(curl_handle, CURLOPT_URL, url.c_str());
    res = curl_easy_perform(curl_handle);

    if(res != CURLE_OK)
    {
        Log::err("curl_easy_perform failed: %s after: %d", curl_easy_strerror(res),downloadSize);
        free(chunk.memory);
        return "";
    }

    std::string ret = std::string(chunk.memory,chunk.size);
    free(chunk.memory);
    return ret;
}

std::string HttpClient::get(const std::string &url, ssize_t sz)
{
    struct MemoryStruct chunk;

    chunk.memory = (char*)malloc(1);
    chunk.size = 0;

    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(curl_handle, CURLOPT_URL, url.c_str());
    res = curl_easy_perform(curl_handle);

    if(res != CURLE_OK)
    {
        Log::err("curl_easy_perform failed: %s", curl_easy_strerror(res));

        if(downloadSize < sz)
        {
            curl_easy_setopt(curl_handle, CURLOPT_RESUME_FROM , downloadSize);
            res = curl_easy_perform(curl_handle);
            if(res != CURLE_OK)
            {
                Log::err("curl_easy_perform failed(1): %s", curl_easy_strerror(res));
                free(chunk.memory);
                return "";
            }
        }
    }

    std::string ret = std::string(chunk.memory,chunk.size);
    free(chunk.memory);
    return ret;
}

std::string HttpClient::post(const std::string &url, const std::string &params)
{
    Log::gdb("post url: &s", url.c_str());

    struct MemoryStruct chunk;

    chunk.memory = (char*)malloc(1);
    chunk.size = 0;

    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(curl_handle, CURLOPT_URL, url.c_str());
//	curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, (void*)parms);
    curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, (void*)params.c_str());
    res = curl_easy_perform(curl_handle);
//	delete parms;

    if(res != CURLE_OK)
    {
        Log::err("curl_easy_perform failed: %s", curl_easy_strerror(res));
        free(chunk.memory);
        return "";
    }

    std::string ret = std::string(chunk.memory,chunk.size);
    free(chunk.memory);
    return ret;
}

