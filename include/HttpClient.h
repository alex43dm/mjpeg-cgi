#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H

#include <string>

#include <curl/curl.h>

#define USERAGENT "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.31 (KHTML,like Gecko) Chrome/26.0.1410.43 Safari/537.31"

class HttpClient
{
	public:
		HttpClient();
		virtual ~HttpClient();
		std::string get(const std::string &url);
		std::string get(const std::string &url, ssize_t sz);
		std::string post(const std::string &url, const std::string &params);

	protected:
	private:
		CURL *curl_handle;
		CURLcode res;
		std::string *responce;
		ssize_t downloadSize;

		static size_t callback(void *contents, size_t size, size_t nmemb, void *userp);
};

#endif // HTTPCLIENT_H
