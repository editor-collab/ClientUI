#include <managers/AccountManager.hpp>

#include <managers/WebManager.hpp>
#include <Geode/utils/web.hpp>

using namespace tulip::editor;
using namespace geode::prelude;

class AccountManager::Impl {
public:
    Impl() = default;
    ~Impl() = default;

    int m_accountId = 0;
    int m_userId = 0;
    std::string m_username;

    std::mutex m_authMutex;
    std::string m_authToken;

    Callback m_requestCallback;

    EventListener<web::WebTask> m_authListener;

    Result<> refreshCredentials();

    bool errorCallback(web::WebResponse* response);
    void authenticateCallback(web::WebTask::Event* event);
    Result<> authenticate(Callback&& callback);
    Result<> startChallenge(Callback&& callback);
    Result<> completeChallenge(std::string const& challenge);
};

Result<> AccountManager::Impl::refreshCredentials() {
    if (GJAccountManager::get()->m_accountID == 0) {
        return Err("Account not logged in");
    }

    m_accountId = GJAccountManager::get()->m_accountID;
    m_userId = GameManager::get()->m_playerUserID.value();
    m_username = GJAccountManager::get()->m_username;

    return Ok();
}

bool AccountManager::Impl::errorCallback(web::WebResponse* response) {
    if (response->ok()) {
        auto const res = response->json();
        if (res.isErr()) {
            m_requestCallback(Err("Invalid json response"));
            return true;
        }
    }
    else {
        auto const res = response->string();
        if (res.isErr()) {
            m_requestCallback(Err("Invalid string response"));
            return true;
        }

        auto const code = response->code();
        if (code == 401) {
            m_requestCallback(Err("Invalid credentials"));
        }
        else {
            m_requestCallback(Err(fmt::format("HTTP error: {}", code)));
        }
        return true;
    }
    return false;
}

void AccountManager::Impl::authenticateCallback(web::WebTask::Event* event) {
    if (event->getValue() == nullptr) return;

    auto response = event->getValue();
    if (this->errorCallback(response)) return;

    if (response->ok()) {
        auto const res = response->json().unwrap();
        if (res.contains("token")) {
            m_authToken = res.get<std::string>("token");
            m_requestCallback(Ok());
        }
        else {
            m_requestCallback(Err("Invalid json response"));
        }
    }
    else {
        auto const reason = response->string().unwrap();
        m_requestCallback(Err(reason));
    }
}

Result<> AccountManager::Impl::authenticate(Callback&& callback) {
    GEODE_UNWRAP(this->refreshCredentials());

    auto req = WebManager::get()->createRequest();

    req.param("accid", m_accountId);
    req.param("userid", m_userId);
    req.param("username", m_username);

    auto task = req.post(fmt::format("{}/auth", WebManager::get()->getServerURL()));

    m_authListener.bind(this, &AccountManager::Impl::authenticateCallback);
    m_authListener.setFilter(task);

    m_requestCallback = std::move(callback);

    return Ok();
}

Result<> AccountManager::Impl::startChallenge(Callback&& callback) {
    return Ok();
}

Result<> AccountManager::Impl::completeChallenge(std::string const& challenge) {
    return Ok();
}

AccountManager* AccountManager::get() {
    static AccountManager instance;
    return &instance;
}

AccountManager::AccountManager() : impl(std::make_unique<Impl>()) {}
AccountManager::~AccountManager() = default;

void AccountManager::authenticate(Callback&& callback) {
    auto res = impl->authenticate(callback);
    if (res.isErr()) {
        callback(res);
        return;
    }
}
void AccountManager::startChallenge(Callback&& callback) {
    auto res = impl->startChallenge(callback);
    if (res.isErr()) {
        callback(res);
        return;
    }
}