#include <managers/AccountManager.hpp>

#include <managers/WebManager.hpp>
#include <Geode/utils/web.hpp>
#include <argon/argon.hpp>
#include <utils/CryptoHelper.hpp>

using namespace tulip::editor;
using namespace geode::prelude;

static inline std::string s_savedLoginToken = "";

class AccountManager::Impl {
public:
    Impl() = default;
    ~Impl() = default;

    int m_accountId = 0;
    int m_userId = 0;
    std::string m_username;
    
    std::string m_loginToken;

    EventListener<web::WebTask> m_authListener;

    Result<> refreshCredentials();

    bool errorCallback(web::WebResponse* response, Callback& callback);
    void loginCallback(web::WebTask::Event* event, Callback callback);

    Result<> authenticate(Callback&& callback);
    Result<> logout(Callback&& callback);

    std::string getLoginToken() const;

    Task<Result<uint32_t>, WebProgress> claimKey(std::string_view key);

    bool isLoggedIn() const;
};

$on_mod(Loaded) {
    if (Mod::get()->getSettingValue<bool>("auto-connect")) {
        s_savedLoginToken = Mod::get()->getSavedValue<std::string>("saved-login-token", "");
    }
}

std::string AccountManager::Impl::getLoginToken() const {
    if (!m_loginToken.empty()) {
        return m_loginToken;
    }
    return s_savedLoginToken;
}

bool AccountManager::Impl::isLoggedIn() const {
    if (!m_loginToken.empty()) {
        return true;
    }
    return !s_savedLoginToken.empty();
}

Result<> AccountManager::Impl::refreshCredentials() {
    if (GJAccountManager::get()->m_accountID == 0) {
        return Err("Account not logged in");
    }

    m_accountId = GJAccountManager::get()->m_accountID;
    m_userId = GameManager::get()->m_playerUserID.value();
    m_username = GJAccountManager::get()->m_username;

    return Ok();
}

bool AccountManager::Impl::errorCallback(web::WebResponse* response, Callback& callback) {
    auto const res = response->string();
    if (res.isErr()) {
        callback(Err("Invalid string response"));
        return true;
    }
    if (!response->ok()) {
        auto const code = response->code();
        if (code == 401) {
            callback(Err("Invalid credentials"));
        }
        else {
            callback(Err(fmt::format("HTTP error: {}", code)));
        }
        return true;
    }
    return false;
}


void AccountManager::Impl::loginCallback(web::WebTask::Event* event, Callback callback) {
    if (event->getValue() == nullptr) return;

    auto response = event->getValue();
    if (this->errorCallback(response, callback)) return;

    auto token = response->string().unwrapOrDefault();

    auto header = fmt::format("{}.{}.{}", m_accountId, m_userId, token);
    std::vector<uint8_t> headerBytes(header.begin(), header.end());
    auto base64Token = crypto::base64Encode(headerBytes, crypto::Base64Flags::UrlSafe);

    Mod::get()->setSavedValue("saved-login-token", base64Token);
    m_loginToken = base64Token;
    s_savedLoginToken.clear();

    callback(Ok());
}

Result<> AccountManager::Impl::authenticate(Callback&& callback) {   
    GEODE_UNWRAP(this->refreshCredentials());

    auto res = argon::startAuth([this, callback = std::move(callback)](Result<std::string> res) {
        if (!res) {
            log::warn("Auth failed: {}", res.unwrapErr());
            callback(Err(std::move(res.unwrapErr())));
            return;
        }

        auto token = std::move(res).unwrap();

        auto req = WebManager::get()->createRequest();
        req.bodyString(fmt::format("account_id={}&user_id={}&account_name={}&auth_token={}",
            m_accountId, m_userId, m_username, token));
        // req.param("account_id", m_accountId);
        // req.param("user_id", m_userId);
        // req.param("account_name", m_username);
        // req.param("auth_token", token);

        auto task = req.post(WebManager::get()->getServerURL("auth/login"));

        m_authListener.bind([this, callback = std::move(callback)](web::WebTask::Event* event) {
            this->loginCallback(event, callback);
        });
        m_authListener.setFilter(task);
    }, [](argon::AuthProgress progress) {
        log::info("Auth progress: {}", argon::authProgressToString(progress));
    }); 

    return Ok();
}

Result<> AccountManager::Impl::logout(Callback&& callback) {
    GEODE_UNWRAP(this->refreshCredentials());

    m_loginToken.clear();
    s_savedLoginToken.clear();

    callback(Ok());

    return Ok();
}

Task<Result<uint32_t>, WebProgress> AccountManager::Impl::claimKey(std::string_view key) {
    auto req = WebManager::get()->createAuthenticatedRequest();

    req.param("activation_key", key);

    auto task = req.post(WebManager::get()->getServerURL("auth/claim_key"));
    auto ret = task.map([=, this](auto response) -> Result<uint32_t> {
        if (!response->ok()) return Err(fmt::format("HTTP error: {}", response->code()));
        auto count = response->string().map([](auto str) {
            return numFromString<uint32_t>(str).unwrapOrDefault();
        });
        return count;
    });
    return ret;
}

AccountManager* AccountManager::get() {
    static AccountManager instance;
    return &instance;
}

AccountManager::AccountManager() : impl(std::make_unique<Impl>()) {}
AccountManager::~AccountManager() = default;

void AccountManager::authenticate(Callback&& callback) {
    auto res = impl->authenticate(std::move(callback));
    if (res.isErr()) {
        callback(res);
        return;
    }
}
void AccountManager::logout(Callback&& callback) {
    auto res = impl->logout(std::move(callback));
    if (res.isErr()) {
        callback(res);
        return;
    }
}

std::string AccountManager::getLoginToken() const {
    return impl->getLoginToken();
}

Task<Result<uint32_t>, WebProgress> AccountManager::claimKey(std::string_view key) {
    return impl->claimKey(key);
}

bool AccountManager::isLoggedIn() const {
    return impl->isLoggedIn();
}