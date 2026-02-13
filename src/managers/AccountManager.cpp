#include <managers/AccountManager.hpp>

#include <managers/WebManager.hpp>
#include <Geode/utils/web.hpp>
#include <Geode/utils/async.hpp>
#include <utils/CryptoHelper.hpp>

using namespace tulip::editor;
using namespace geode::prelude;

class AccountManager::Impl {
public:
    Impl() = default;
    ~Impl() = default;
    
    arc::Future<Result<std::string>> login(argon::AccountData accountData);
    Result<> logout();
    arc::Future<Result<uint32_t>> claimKey(std::string_view key);
};

arc::Future<Result<std::string>> AccountManager::Impl::login(argon::AccountData accountData) {  
    auto accountId = accountData.accountId;
    auto userId = accountData.userId;
    auto username = accountData.username; 

    auto res = co_await argon::startAuth(std::move(accountData));
    if (!res) {
        log::warn("Auth failed: {}", res.unwrapErr());
        co_return Err(std::move(res.unwrapErr()));
    }

    auto token = std::move(res).unwrap();

    auto req = WebManager::get()->createRequest();
    req.bodyString(
        fmt::format("account_id={}&user_id={}&account_name={}&auth_token={}",
            accountId, userId, username, token
        )
    );
    // req.param("account_id", m_accountId);
    // req.param("user_id", m_userId);
    // req.param("account_name", m_username);
    // req.param("auth_token", token);

    auto response = co_await req.post(WebManager::get()->getServerURL("auth/login"));
    if (!response.ok()) {
        if (response.code() == 403) {
            argon::clearToken();
            log::warn("Auth token invalid, cleared saved token");
        }
    }
    GEODE_CO_UNWRAP(WebManager::get()->errorCallback(&response));

    auto genToken = GEODE_CO_UNWRAP(response.string());

    auto header = fmt::format("{}.{}.{}", accountId, userId, genToken);
    std::vector<uint8_t> headerBytes(header.begin(), header.end());
    auto base64Token = crypto::base64Encode(headerBytes, crypto::Base64Flags::UrlSafe);

    co_return Ok(base64Token);
}

Result<> AccountManager::Impl::logout() {
    return Ok();
}

arc::Future<Result<uint32_t>> AccountManager::Impl::claimKey(std::string_view key) {
    auto req = WebManager::get()->createAuthenticatedRequest();
    req.param("activation_key", std::string(key));

    auto response = co_await req.post(WebManager::get()->getServerURL("auth/claim_key"));
    GEODE_CO_UNWRAP(WebManager::get()->errorCallback(&response));

    co_return Ok(numFromString<uint32_t>(response.string().unwrapOrDefault()).unwrapOrDefault());
}

AccountManager* AccountManager::get() {
    static AccountManager instance;
    return &instance;
}

AccountManager::AccountManager() : impl(std::make_unique<Impl>()) {}
AccountManager::~AccountManager() = default;

arc::Future<Result<std::string>> AccountManager::login(argon::AccountData accountData) {
    return impl->login(std::move(accountData));
}
Result<> AccountManager::logout() {
    return impl->logout();
}

arc::Future<Result<uint32_t>> AccountManager::claimKey(std::string_view key) {
    return impl->claimKey(key);
}
