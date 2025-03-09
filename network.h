
#include <iostream>
#include <sstream>
#include <random>
#include <algorithm>
#include <string>
#include <vector>
#include "dataset.h"

namespace network
{
	//std::string http_get_headers(const std::string& url, const std::string& authToken);
	std::string get_auth_token(const std::string& username, const std::string& password);
	std::vector<Product> fetch_products(const std::string& authToken);
	std::string try_redeem_key(const std::string& authToken, const std::string& key);
    std::string random_hex(const std::string& input);
}
