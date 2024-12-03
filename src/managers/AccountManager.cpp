#include <managers/AccountManager.hpp>

#include <managers/WebManager.hpp>
#include <Geode/utils/web.hpp>
#include <utils/CryptoHelper.hpp>

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
    
    std::string m_loginToken;

    Callback m_requestCallback;

    EventListener<web::WebTask> m_authListener;

    Result<> refreshCredentials();

    bool errorCallback(web::WebResponse* response);
    void serverTimeCallback(web::WebTask::Event* event);
    void loginCallback(web::WebTask::Event* event);
    void startChallengeCallback(web::WebTask::Event* event);
    void completeChallengeCallback(web::WebTask::Event* event);

    Result<> authenticate(Callback&& callback);
    Result<> startChallenge(Callback&& callback);

    std::string getLoginToken() const;

    std::string getAuthToken() const;
    void setAuthToken(std::string_view const token);

    bool isAuthenticated() const;
    bool isLoggedIn() const;
};

$on_mod(Loaded) {
    log::debug("found token {}", Mod::get()->getSavedValue<std::string>("auth-token"));
    AccountManager::get()->setAuthToken(Mod::get()->getSavedValue<std::string>("auth-token"));
}

std::string AccountManager::Impl::getLoginToken() const {
    return m_loginToken;
}

std::string AccountManager::Impl::getAuthToken() const {
    return m_authToken;
}

void AccountManager::Impl::setAuthToken(std::string_view const token) {
    m_authToken = token;
}

bool AccountManager::Impl::isAuthenticated() const {
    return !m_authToken.empty();
}

bool AccountManager::Impl::isLoggedIn() const {
    return !m_loginToken.empty();
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

bool AccountManager::Impl::errorCallback(web::WebResponse* response) {
    auto const res = response->string();
    if (res.isErr()) {
        m_requestCallback(Err("Invalid string response"));
        return true;
    }
    if (!response->ok()) {
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

void AccountManager::Impl::completeChallengeCallback(web::WebTask::Event* event) {
    if (event->getValue() == nullptr) return;

    auto response = event->getValue();
    if (this->errorCallback(response)) return;

    auto const res = response->string().unwrapOrDefault();

    if (res.find(":") == std::string::npos) {
        m_requestCallback(Err("Invalid challenge response"));
        return;
    }

    GEODE_UNWRAP_OR_ELSE(messageId, numFromString<int>(res.substr(0, res.find(":")))) {
        m_requestCallback(Err("Invalid message response"));
        return;
    }

    if (messageId) {
        auto message = GJUserMessage::create();
        message->m_messageID = messageId;
        GameLevelManager::sharedState()->deleteUserMessages(message, nullptr, true);
    }

    auto const encodedToken = res.substr(res.find(":") + 1);
    m_authToken = encodedToken;
    Mod::get()->setSavedValue("auth-token", m_authToken);

    m_requestCallback(Ok());
}

void AccountManager::Impl::startChallengeCallback(web::WebTask::Event* event) {
    if (event->getValue() == nullptr) return;

    auto response = event->getValue();
    if (this->errorCallback(response)) return;

    auto const res = response->string().unwrapOrDefault();

    if (res.find(":") == std::string::npos) {
        m_requestCallback(Err("Invalid challenge response"));
        return;
    }

    GEODE_UNWRAP_OR_ELSE(toSendId, numFromString<int>(res.substr(0, res.find(":")))) {
        m_requestCallback(Err("Invalid account response"));
        return;
    }

    auto const challenge = res.substr(res.find(":") + 1);
    auto const challengeDecoded = crypto::base64Decode(challenge, crypto::Base64Flags::UrlSafe);

    constexpr auto NONCE_INDEX = 0;
    constexpr auto NONCE_LENGTH = 12;
    constexpr auto KEY_INDEX = NONCE_INDEX + NONCE_LENGTH;
    constexpr auto KEY_LENGTH = 32;
    constexpr auto TAG_INDEX = KEY_INDEX + KEY_LENGTH;
    constexpr auto TAG_LENGTH = 16;
    constexpr auto CIPHER_INDEX = TAG_INDEX + TAG_LENGTH;
    constexpr auto CIPHER_LENGTH = 16;

    auto const challengeNonce = std::vector<uint8_t>(
        challengeDecoded.begin() + NONCE_INDEX, challengeDecoded.begin() + NONCE_INDEX + NONCE_LENGTH
    );
    auto const challengeKey = std::vector<uint8_t>(
        challengeDecoded.begin() + KEY_INDEX, challengeDecoded.begin() + KEY_INDEX + KEY_LENGTH
    );
    auto const challengeTag = std::vector<uint8_t>(
        challengeDecoded.begin() + TAG_INDEX, challengeDecoded.begin() + TAG_INDEX + TAG_LENGTH
    );
    auto const challengeCipher = std::vector<uint8_t>(
        challengeDecoded.begin() + CIPHER_INDEX, challengeDecoded.begin() + CIPHER_INDEX + CIPHER_LENGTH
    );

    auto const [decryptedAnswer, decryptedTag] = crypto::chacha20Poly1305Decrypt(challengeCipher, challengeKey, challengeNonce);
    
    if (decryptedTag != challengeTag) {
        m_requestCallback(Err("Invalid challenge response"));
        return;
    }

    auto const answer = std::string(decryptedAnswer.begin(), decryptedAnswer.end());

    if (toSendId) {
        log::debug("Sending message to: {}", toSendId);
        GameLevelManager::sharedState()->uploadUserMessage(
            toSendId, fmt::format("###: {}", answer), 
            "Editor collab verification challenge, can be safely deleted."
        );
    }
    
    auto req = WebManager::get()->createRequest();

    req.param("account_id", m_accountId);
    req.param("user_id", m_userId);
    req.param("account_name", m_username);
    req.param("challenge", answer);
    
    auto task = req.post(WebManager::get()->getServerURL("auth/verify"));

    m_authListener.bind(this, &AccountManager::Impl::completeChallengeCallback);
    m_authListener.setFilter(task);
}

void AccountManager::Impl::loginCallback(web::WebTask::Event* event) {
    if (event->getValue() == nullptr) return;

    auto response = event->getValue();
    if (this->errorCallback(response)) return;

    auto token = response->string().unwrapOrDefault();

    auto header = fmt::format("{}.{}.{}", m_accountId, m_userId, token);
    std::vector<uint8_t> headerBytes(header.begin(), header.end());
    auto base64Token = crypto::base64Encode(headerBytes, crypto::Base64Flags::UrlSafe);

    m_loginToken = base64Token;

    m_requestCallback(Ok());
}

void AccountManager::Impl::serverTimeCallback(web::WebTask::Event* event) {
    if (event->getValue() == nullptr) return;

    auto const response = event->getValue();
    if (this->errorCallback(response)) return;

    auto const res = response->string().unwrapOrDefault();

    GEODE_UNWRAP_OR_ELSE(time, numFromString<uint64_t>(res)) {
        m_requestCallback(Err("Invalid time response"));
        return;
    }
    auto const roundedTime = time - time % 30;
    auto const timedToken = fmt::format("{}:{}", m_authToken, roundedTime);
    log::debug("timedToken: {}", timedToken);
    auto const hashedToken = crypto::blake2bEncrypt(std::vector<uint8_t>(timedToken.begin(), timedToken.end()));
    auto const base64Token = crypto::base64Encode(hashedToken, crypto::Base64Flags::UrlSafe);

    auto req = WebManager::get()->createRequest();

    req.param("account_id", m_accountId);
    req.param("user_id", m_userId);
    req.param("account_name", m_username);
    req.param("auth_token", base64Token);

    auto task = req.post(WebManager::get()->getServerURL("auth/login"));

    m_authListener.bind(this, &AccountManager::Impl::loginCallback);
    m_authListener.setFilter(task);
}

Result<> AccountManager::Impl::authenticate(Callback&& callback) {    
    GEODE_UNWRAP(this->refreshCredentials());

    m_requestCallback = std::move(callback);

    auto req = WebManager::get()->createRequest();

    auto task = req.get(WebManager::get()->getServerURL("auth/server_time"));

    m_authListener.bind(this, &AccountManager::Impl::serverTimeCallback);
    m_authListener.setFilter(task);

    return Ok();
}

Result<> AccountManager::Impl::startChallenge(Callback&& callback) {
    GEODE_UNWRAP(this->refreshCredentials());

    m_requestCallback = std::move(callback);

    auto req = WebManager::get()->createRequest();

    req.param("account_id", m_accountId);
    req.param("user_id", m_userId);
    req.param("account_name", m_username);

    auto task = req.post(WebManager::get()->getServerURL("auth/challenge"));

    m_authListener.bind(this, &AccountManager::Impl::startChallengeCallback);
    m_authListener.setFilter(task);

    return Ok();
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
void AccountManager::startChallenge(Callback&& callback) {
    auto res = impl->startChallenge(std::move(callback));
    if (res.isErr()) {
        callback(res);
        return;
    }
}

std::string AccountManager::getLoginToken() const {
    return impl->getLoginToken();
}

std::string AccountManager::getAuthToken() const {
    return impl->getAuthToken();
}

void AccountManager::setAuthToken(std::string_view const token) {
    return impl->setAuthToken(token);
}

bool AccountManager::isAuthenticated() const {
    return impl->isAuthenticated();
}
bool AccountManager::isLoggedIn() const {
    return impl->isLoggedIn();
}