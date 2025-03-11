//#pragma comment(lib, "wininet.lib")
//
//#include <iostream>
//#include <vector>
//#include <string>
//#include <windows.h>
//#include <wininet.h>
//#include "network.h"

//std::string network::http_get_headers(const std::string& url, const std::string& authToken) {
//    HINTERNET hInternet = InternetOpenA("Demonized/1.0 (compatible)", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
//    if (!hInternet) return "";
//
//    HINTERNET hConnect = InternetOpenUrlA(hInternet, url.c_str(),
//        ("Authorization: Bearer " + authToken).c_str(),
//        -1, INTERNET_FLAG_SECURE | INTERNET_FLAG_RELOAD, 0);
//    if (!hConnect) {
//        InternetCloseHandle(hInternet);
//        return "";
//    }
//
//    char buffer[4096];
//    DWORD bytesRead;
//    std::string response;
//
//    while (InternetReadFile(hConnect, buffer, sizeof(buffer) - 1, &bytesRead) && bytesRead > 0) {
//        buffer[bytesRead] = 0;
//        response += buffer;
//    }
//
//    InternetCloseHandle(hConnect);
//    InternetCloseHandle(hInternet);
//
//    return response;
//}

//#pragma comment(lib, "Ws2_32.lib")
//#define WIN32_LEAN_AND_MEAN
//#include <WinSock2.h>
//#include <Windows.h>
//#include <ws2tcpip.h>

#include <string>
#include <iostream>
#include "nlohmann/json.hpp"
#include "network.h"
#include "dataset.h"
#include "xorstr.h"
#include <cpr/cpr.h>


std::string network::get_auth_token(const std::string& username, const std::string& password) {
    cpr::Response r = cpr::Post(cpr::Url{ xorstr_("http://127.0.0.1:5000/login") },
        cpr::Header{ {xorstr_("Content-Type"), xorstr_("application/json")} },
        cpr::Body{ xorstr_(R"({"username":")") + username + xorstr_(R"(","password":")") + password + xorstr_(R"("})") });

    if (r.status_code == 200) {
        auto jsonData = nlohmann::json::parse(r.text);
        return jsonData[xorstr_("access_token")];
    }
    else {
        //std::cerr << xorstr_("Failed to authenticate: ") << r.text << std::endl;
        return "";
    }
}
std::vector<Product> network::fetch_products(const std::string& authToken) {
    std::vector<Product> products;
    try {
        cpr::Response response = cpr::Get(
            cpr::Url{ xorstr_("http://127.0.0.1:5000/products") },
            cpr::Header{ {xorstr_("Authorization"), xorstr_("Bearer ") + authToken} },
            cpr::VerifySsl(false)
        );

        if (response.status_code != 200) {
            //std::cerr << "Request failed with status code: " << response.status_code << std::endl;
            products.push_back({ "error", "0", "0"});
            return products;
        }

        // Parse JSON response
        nlohmann::json jsonData = nlohmann::json::parse(response.text);
        for (const auto& item : jsonData) {
            products.push_back({ item[xorstr_("name")], item[xorstr_("status")], item[xorstr_("expiration_date")] });
        }
    }
    catch (const std::exception& e) {
        //std::cerr << "Request error: " << e.what() << '\n';
        products.push_back({ "error", "0", "1" });
    }

    return products;
}

std::string network::try_redeem_key(const std::string& authToken, const std::string& key) {
    cpr::Response r = cpr::Post(cpr::Url{ xorstr_("http://127.0.0.1:5000/claim_key") },
        cpr::Header{ { xorstr_("Authorization"), xorstr_("Bearer ") + authToken }, { xorstr_("Content-Type"), xorstr_("application/json") } },
        cpr::Body{ xorstr_(R"({"key":")") + key + xorstr_(R"("})") },
        cpr::VerifySsl(false));

    if (r.status_code == 200 || r.status_code == 400) {
        auto jsonData = nlohmann::json::parse(r.text);
        return jsonData[xorstr_("msg")];
    }
    else {
        //std::cerr << "Failed to authenticate: " << r.text << std::endl;
        return "";
    }
}

std::string network::random_hex(const std::string& input) {
    std::vector<std::string> hexParts;
    std::stringstream ss;

    // Convert each character to hex
    for (unsigned char c : input) {
        ss.str("");  // Clear stream
        ss << std::hex << static_cast<int>(c);
        hexParts.push_back(ss.str());
    }

    // Shuffle hex values randomly
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(hexParts.begin(), hexParts.end(), g);

    // Join shuffled hex values into a final string
    std::string result;
    for (const auto& part : hexParts) {
        result += part;
    }

    return result;
}