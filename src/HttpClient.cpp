#include <iostream>

#include "Log.h"
#include "HttpClient.h"

HttpClient::HttpClient()
{
	curl_global_init(CURL_GLOBAL_ALL);
	curl_handle = curl_easy_init();
	curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, callback);
	curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)this);

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

size_t HttpClient::callback(void *contents, size_t csize, size_t nmemb, void *data)
{
	HttpClient *hc = (HttpClient *)data;

	//char * p = new char[csize * nmemb];
	//memcopy(p, contents, csize*nmemb);
	std::string s;
	s.insert(0, (const char*)contents, csize * nmemb);
	*hc->responce += s;
	hc->downloadSize += csize * nmemb;

	return csize * nmemb;
}

std::string* HttpClient::get(const std::string &url)
{
	Log::gdb("get url: %s", url.c_str());

//	if(responce)
//		delete responce;

	responce = new std::string();

	curl_easy_setopt(curl_handle, CURLOPT_URL, url.c_str());
	res = curl_easy_perform(curl_handle);

	if(res != CURLE_OK)
	{
		Log::err("curl_easy_perform failed: %s after: %d", curl_easy_strerror(res),downloadSize);
		delete responce;
		return NULL;
	}

    Log::gdb("get url: ok, size: %d", responce->size());

	return responce;
}

std::string *HttpClient::get(const std::string &url, ssize_t sz)
{
	Log::gdb("get url: %s", url.c_str());

	responce = new std::string();

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
                delete responce;
                return nullptr;
            }
        }
	}

    Log::gdb("get url: ok, size: %d", responce->size());

	return responce;
}

std::string* HttpClient::post(const std::string &url, const std::string &params)
{
	Log::gdb("post url: &s", url.c_str());

//	char *parms = new char[1024];
//	parms = curl_easy_escape( curl_handle, params.c_str() , params.size() );

//	if(responce)
//		delete responce;

	responce = new std::string();
	curl_easy_setopt(curl_handle, CURLOPT_URL, url.c_str());
//	curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, (void*)parms);
	curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, (void*)params.c_str());
	res = curl_easy_perform(curl_handle);
//	delete parms;

	if(res != CURLE_OK)
	{
        Log::err("curl_easy_perform failed: %s", curl_easy_strerror(res));
		delete responce;
		return NULL;
	}

    Log::gdb("post url: ok, size: %d", responce->size());
	return responce;
}

