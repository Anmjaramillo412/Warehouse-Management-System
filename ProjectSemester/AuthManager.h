#pragma once

// ================================================================
// AUTH MANAGER (header-only)
// ================================================================
// Login, user accounts and role-based access for the web interface.
//
// Roles:
//   Viewer - read only (every GET; no changes of any kind)
//   Editor - day-to-day work (create / modify / delete / receive ...)
//   Admin  - everything an Editor can do, plus user management and
//            system-level actions (load data, exit server, reset
//            numbering, data / document-folder settings)
//
// Users live in "data/users.txt" (one line per user:
// username|role|salt|hash). data/ is git-ignored, so account data
// never ends up in the repository. Passwords are never stored - only
// a salted, iterated SHA-256 hash.
//
// Sessions are random tokens kept in memory (they end when the
// server stops) and travel in an HttpOnly cookie.
//
// Everything is enforced on the SERVER (see AuthMiddleware at the
// bottom): hiding buttons in the web page is only cosmetic.
// ================================================================

#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <chrono>
#include <random>
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <filesystem>
#include <cstdint>
#include <cstring>
#include <cctype>

#include <crow.h>

namespace auth
{

// ----------------------------------------------------------------
// ROLES
// ----------------------------------------------------------------

enum class Role
{
    Viewer,
    Editor,
    Admin
};

inline std::string roleToString(Role role)
{
    switch (role)
    {
    case Role::Admin:  return "admin";
    case Role::Editor: return "editor";
    default:           return "viewer";
    }
}

inline bool parseRole(const std::string& text, Role& role)
{
    if (text == "admin")  { role = Role::Admin;  return true; }
    if (text == "editor") { role = Role::Editor; return true; }
    if (text == "viewer") { role = Role::Viewer; return true; }

    return false;
}


// ----------------------------------------------------------------
// SHA-256 (public-domain style reference implementation)
// ----------------------------------------------------------------

class Sha256
{
public:

    static std::string raw(const std::string& data)
    {
        static const uint32_t K[64] = {
            0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,
            0x59f111f1,0x923f82a4,0xab1c5ed5,0xd807aa98,0x12835b01,
            0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,
            0xc19bf174,0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,
            0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,0x983e5152,
            0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,
            0x06ca6351,0x14292967,0x27b70a85,0x2e1b2138,0x4d2c6dfc,
            0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
            0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,
            0xd6990624,0xf40e3585,0x106aa070,0x19a4c116,0x1e376c08,
            0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,
            0x682e6ff3,0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,
            0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2 };

        uint32_t h[8] = {
            0x6a09e667,0xbb67ae85,0x3c6ef372,0xa54ff53a,
            0x510e527f,0x9b05688c,0x1f83d9ab,0x5be0cd19 };

        std::string msg = data;

        uint64_t bitLength =
            static_cast<uint64_t>(data.size()) * 8;

        msg.push_back(static_cast<char>(0x80));

        while (msg.size() % 64 != 56)
        {
            msg.push_back(0);
        }

        for (int i = 7; i >= 0; i--)
        {
            msg.push_back(
                static_cast<char>((bitLength >> (i * 8)) & 0xff));
        }

        for (size_t chunk = 0; chunk < msg.size(); chunk += 64)
        {
            uint32_t w[64];

            for (int i = 0; i < 16; i++)
            {
                w[i] =
                    (static_cast<uint32_t>(
                        static_cast<unsigned char>(msg[chunk + i * 4])) << 24) |
                    (static_cast<uint32_t>(
                        static_cast<unsigned char>(msg[chunk + i * 4 + 1])) << 16) |
                    (static_cast<uint32_t>(
                        static_cast<unsigned char>(msg[chunk + i * 4 + 2])) << 8) |
                    (static_cast<uint32_t>(
                        static_cast<unsigned char>(msg[chunk + i * 4 + 3])));
            }

            for (int i = 16; i < 64; i++)
            {
                uint32_t s0 =
                    rotr(w[i - 15], 7) ^ rotr(w[i - 15], 18) ^ (w[i - 15] >> 3);

                uint32_t s1 =
                    rotr(w[i - 2], 17) ^ rotr(w[i - 2], 19) ^ (w[i - 2] >> 10);

                w[i] = w[i - 16] + s0 + w[i - 7] + s1;
            }

            uint32_t a = h[0], b = h[1], c = h[2], d = h[3];
            uint32_t e = h[4], f = h[5], g = h[6], hh = h[7];

            for (int i = 0; i < 64; i++)
            {
                uint32_t S1 =
                    rotr(e, 6) ^ rotr(e, 11) ^ rotr(e, 25);

                uint32_t ch = (e & f) ^ (~e & g);

                uint32_t temp1 = hh + S1 + ch + K[i] + w[i];

                uint32_t S0 =
                    rotr(a, 2) ^ rotr(a, 13) ^ rotr(a, 22);

                uint32_t maj = (a & b) ^ (a & c) ^ (b & c);

                uint32_t temp2 = S0 + maj;

                hh = g;
                g = f;
                f = e;
                e = d + temp1;
                d = c;
                c = b;
                b = a;
                a = temp1 + temp2;
            }

            h[0] += a; h[1] += b; h[2] += c; h[3] += d;
            h[4] += e; h[5] += f; h[6] += g; h[7] += hh;
        }

        std::string out(32, '\0');

        for (int i = 0; i < 8; i++)
        {
            out[i * 4]     = static_cast<char>((h[i] >> 24) & 0xff);
            out[i * 4 + 1] = static_cast<char>((h[i] >> 16) & 0xff);
            out[i * 4 + 2] = static_cast<char>((h[i] >> 8) & 0xff);
            out[i * 4 + 3] = static_cast<char>(h[i] & 0xff);
        }

        return out;
    }

    static std::string hex(const std::string& rawBytes)
    {
        static const char digits[] = "0123456789abcdef";

        std::string out;

        out.reserve(rawBytes.size() * 2);

        for (unsigned char c : rawBytes)
        {
            out.push_back(digits[c >> 4]);
            out.push_back(digits[c & 0x0f]);
        }

        return out;
    }

private:

    static uint32_t rotr(uint32_t x, int n)
    {
        return (x >> n) | (x << (32 - n));
    }
};


// ----------------------------------------------------------------
// AUTH MANAGER
// ----------------------------------------------------------------

struct CurrentUser
{
    std::string username;
    Role role = Role::Viewer;
};


class AuthManager
{
public:

    static AuthManager& instance()
    {
        static AuthManager manager;

        return manager;
    }


    // Reads data/users.txt. If there are no users yet, creates the
    // first Admin account ("admin") with a random password that is
    // printed ONCE on the server console.

    void load()
    {
        std::lock_guard<std::mutex> lock(mutex);

        users.clear();

        std::ifstream file(usersFile());

        std::string line;

        while (file && std::getline(file, line))
        {
            if (!line.empty() && line.back() == '\r')
            {
                line.pop_back();
            }

            std::vector<std::string> parts = split(line, '|');

            if (parts.size() != 4)
            {
                continue;
            }

            Role role;

            if (!parseRole(parts[1], role))
            {
                continue;
            }

            users.push_back({ parts[0], role, parts[2], parts[3] });
        }

        if (users.empty())
        {
            std::string password = randomPassword(12);

            std::string salt = randomHex(16);

            users.push_back({
                "admin",
                Role::Admin,
                salt,
                hashPassword(password, salt) });

            saveLocked();

            std::cout << std::endl
                << "========================================" << std::endl
                << "FIRST START - Admin account created." << std::endl
                << "  Username: admin" << std::endl
                << "  Password: " << password << std::endl
                << "Log in and change this password. It is shown only now." << std::endl
                << "(Forgot it later? Stop the server, delete data/users.txt"
                << " and start again to get a new one.)" << std::endl
                << "========================================" << std::endl
                << std::endl;
        }
    }


    // ---------------- Login / sessions ----------------

    // Returns "" and fills token/user on success, otherwise a
    // message for the user.

    std::string login(
        const std::string& usernameInput,
        const std::string& password,
        std::string& token,
        CurrentUser& user)
    {
        std::string name = normalize(usernameInput);

        std::lock_guard<std::mutex> lock(mutex);

        auto now = std::chrono::steady_clock::now();

        auto& attempt = attempts[name];

        if (attempt.lockedUntil > now)
        {
            return "Too many failed attempts. Please wait a minute and try again.";
        }

        const UserRecord* record = findLocked(name);

        // Hash even for unknown users so both cases take equally long.

        std::string salt = record ? record->salt : "00000000000000000000000000000000";

        std::string computed = hashPassword(password, salt);

        bool ok =
            record != nullptr &&
            constantTimeEquals(computed, record->hash);

        if (!ok)
        {
            attempt.failures++;

            if (attempt.failures >= 5)
            {
                attempt.failures = 0;

                attempt.lockedUntil =
                    now + std::chrono::seconds(60);
            }

            return "Wrong username or password.";
        }

        attempts.erase(name);

        token = randomHex(32);

        sessions[token] = { record->username, now + sessionLifetime };

        user = { record->username, record->role };

        return "";
    }


    void logout(const std::string& token)
    {
        std::lock_guard<std::mutex> lock(mutex);

        sessions.erase(token);
    }


    // Finds the user behind a session token (sliding expiry). The
    // role is read from the user list each time, so a role change
    // or deletion applies to open sessions immediately.

    bool userForToken(const std::string& token, CurrentUser& user)
    {
        if (token.empty())
        {
            return false;
        }

        std::lock_guard<std::mutex> lock(mutex);

        auto it = sessions.find(token);

        if (it == sessions.end())
        {
            return false;
        }

        auto now = std::chrono::steady_clock::now();

        if (it->second.expiresAt < now)
        {
            sessions.erase(it);

            return false;
        }

        const UserRecord* record = findLocked(it->second.username);

        if (record == nullptr)
        {
            sessions.erase(it);

            return false;
        }

        it->second.expiresAt = now + sessionLifetime;

        user = { record->username, record->role };

        return true;
    }


    // ---------------- User management ----------------
    // All return "" on success, otherwise the reason.

    std::string createUser(
        const std::string& usernameInput,
        const std::string& password,
        const std::string& roleText)
    {
        std::string name = normalize(usernameInput);

        std::string problem = validateUsername(name);

        if (!problem.empty())
        {
            return problem;
        }

        problem = validatePassword(password);

        if (!problem.empty())
        {
            return problem;
        }

        Role role;

        if (!parseRole(roleText, role))
        {
            return "Role must be admin, editor or viewer.";
        }

        std::lock_guard<std::mutex> lock(mutex);

        if (findLocked(name) != nullptr)
        {
            return "A user with this name already exists.";
        }

        std::string salt = randomHex(16);

        users.push_back({ name, role, salt, hashPassword(password, salt) });

        saveLocked();

        return "";
    }


    std::string setRole(
        const std::string& usernameInput,
        const std::string& roleText)
    {
        std::string name = normalize(usernameInput);

        Role role;

        if (!parseRole(roleText, role))
        {
            return "Role must be admin, editor or viewer.";
        }

        std::lock_guard<std::mutex> lock(mutex);

        UserRecord* record = findLocked(name);

        if (record == nullptr)
        {
            return "User not found.";
        }

        if (record->role == Role::Admin &&
            role != Role::Admin &&
            adminCountLocked() <= 1)
        {
            return "There must always be at least one Admin.";
        }

        record->role = role;

        saveLocked();

        return "";
    }


    std::string setPassword(
        const std::string& usernameInput,
        const std::string& newPassword)
    {
        std::string name = normalize(usernameInput);

        std::string problem = validatePassword(newPassword);

        if (!problem.empty())
        {
            return problem;
        }

        std::lock_guard<std::mutex> lock(mutex);

        UserRecord* record = findLocked(name);

        if (record == nullptr)
        {
            return "User not found.";
        }

        record->salt = randomHex(16);

        record->hash = hashPassword(newPassword, record->salt);

        // Existing sessions of this user are closed.

        for (auto it = sessions.begin(); it != sessions.end();)
        {
            if (it->second.username == record->username)
            {
                it = sessions.erase(it);
            }
            else
            {
                ++it;
            }
        }

        saveLocked();

        return "";
    }


    std::string changeOwnPassword(
        const std::string& username,
        const std::string& currentPassword,
        const std::string& newPassword)
    {
        {
            std::lock_guard<std::mutex> lock(mutex);

            const UserRecord* record = findLocked(normalize(username));

            if (record == nullptr ||
                !constantTimeEquals(
                    hashPassword(currentPassword, record->salt),
                    record->hash))
            {
                return "Current password is wrong.";
            }
        }

        return setPassword(username, newPassword);
    }


    std::string deleteUser(
        const std::string& usernameInput,
        const std::string& actingUsername)
    {
        std::string name = normalize(usernameInput);

        if (name == normalize(actingUsername))
        {
            return "You cannot delete your own account.";
        }

        std::lock_guard<std::mutex> lock(mutex);

        for (auto it = users.begin(); it != users.end(); ++it)
        {
            if (it->username != name)
            {
                continue;
            }

            if (it->role == Role::Admin && adminCountLocked() <= 1)
            {
                return "There must always be at least one Admin.";
            }

            users.erase(it);

            for (auto s = sessions.begin(); s != sessions.end();)
            {
                if (s->second.username == name)
                {
                    s = sessions.erase(s);
                }
                else
                {
                    ++s;
                }
            }

            saveLocked();

            return "";
        }

        return "User not found.";
    }


    std::vector<CurrentUser> listUsers()
    {
        std::lock_guard<std::mutex> lock(mutex);

        std::vector<CurrentUser> result;

        for (const auto& user : users)
        {
            result.push_back({ user.username, user.role });
        }

        std::sort(result.begin(), result.end(),
            [](const CurrentUser& a, const CurrentUser& b)
            {
                return a.username < b.username;
            });

        return result;
    }


    // ---------------- Permission rules ----------------
    // Returns 0 = allowed, 401 = not logged in, 403 = not allowed.
    // "path" is the request path, "isRead" is true for GET / HEAD.

    static int decide(
        const std::string& path,
        bool isRead,
        bool loggedIn,
        Role role,
        std::string& message)
    {
        bool isApi = startsWith(path, "/api/");

        bool isImage = startsWith(path, "/web/images/");

        // Page shell (index.html / app.js / style.css) is public so
        // the login screen can be shown; all data is under /api/.

        if (!isApi && !isImage)
        {
            return 0;
        }

        if (path == "/api/auth/login" || path == "/api/status")
        {
            return 0;
        }

        if (!loggedIn)
        {
            message = "Please log in.";

            return 401;
        }

        // Own account actions (logout, who am I, change password).

        if (startsWith(path, "/api/auth/"))
        {
            return 0;
        }

        // Admin only.

        bool adminOnly =
            startsWith(path, "/api/users") ||
            path == "/api/data/exit" ||
            path == "/api/data/load" ||
            path == "/api/data/reset-numbering" ||
            (!isRead && path == "/api/data/config") ||
            (!isRead && path == "/api/data/logging") ||
            (!isRead && startsWith(path, "/api/settings/"));

        if (adminOnly && role != Role::Admin)
        {
            message = "This action is only available to Admin users.";

            return 403;
        }

        // Viewer: read only. POST routes that only look something up
        // are listed here.

        if (role == Role::Viewer && !isRead)
        {
            if (path == "/api/suppliers/lookup")
            {
                return 0;
            }

            message = "Viewer accounts are read-only.";

            return 403;
        }

        return 0;
    }


    static std::string cookieValue(
        const std::string& cookieHeader,
        const std::string& name)
    {
        std::string search = name + "=";

        size_t pos = 0;

        while (pos < cookieHeader.size())
        {
            while (pos < cookieHeader.size() &&
                (cookieHeader[pos] == ' ' || cookieHeader[pos] == ';'))
            {
                pos++;
            }

            if (cookieHeader.compare(pos, search.size(), search) == 0)
            {
                size_t start = pos + search.size();

                size_t end = cookieHeader.find(';', start);

                return cookieHeader.substr(
                    start,
                    end == std::string::npos ? std::string::npos : end - start);
            }

            size_t next = cookieHeader.find(';', pos);

            if (next == std::string::npos)
            {
                break;
            }

            pos = next + 1;
        }

        return "";
    }


    static std::string normalize(const std::string& name)
    {
        std::string out;

        for (char c : name)
        {
            out.push_back(static_cast<char>(
                std::tolower(static_cast<unsigned char>(c))));
        }

        size_t first = out.find_first_not_of(" \t");

        if (first == std::string::npos)
        {
            return "";
        }

        size_t last = out.find_last_not_of(" \t");

        return out.substr(first, last - first + 1);
    }


private:

    struct UserRecord
    {
        std::string username;
        Role role;
        std::string salt;
        std::string hash;
    };

    struct Session
    {
        std::string username;
        std::chrono::steady_clock::time_point expiresAt;
    };

    struct Attempt
    {
        int failures = 0;
        std::chrono::steady_clock::time_point lockedUntil{};
    };

    std::mutex mutex;
    std::vector<UserRecord> users;
    std::map<std::string, Session> sessions;
    std::map<std::string, Attempt> attempts;

    const std::chrono::hours sessionLifetime{ 12 };

    AuthManager() = default;

    static std::string usersFile()
    {
        return "data/users.txt";
    }

    static bool startsWith(
        const std::string& text,
        const std::string& prefix)
    {
        return text.compare(0, prefix.size(), prefix) == 0;
    }

    static std::vector<std::string> split(
        const std::string& text,
        char separator)
    {
        std::vector<std::string> parts;

        std::string current;

        std::stringstream stream(text);

        while (std::getline(stream, current, separator))
        {
            parts.push_back(current);
        }

        return parts;
    }

    UserRecord* findLocked(const std::string& name)
    {
        for (auto& user : users)
        {
            if (user.username == name)
            {
                return &user;
            }
        }

        return nullptr;
    }

    int adminCountLocked() const
    {
        int count = 0;

        for (const auto& user : users)
        {
            if (user.role == Role::Admin)
            {
                count++;
            }
        }

        return count;
    }

    void saveLocked()
    {
        std::filesystem::create_directories("data");

        std::string tempName = usersFile() + ".tmp";

        {
            std::ofstream file(tempName, std::ios::trunc);

            for (const auto& user : users)
            {
                file << user.username << '|'
                    << roleToString(user.role) << '|'
                    << user.salt << '|'
                    << user.hash << '\n';
            }
        }

        std::error_code error;

        std::filesystem::rename(tempName, usersFile(), error);

        if (error)
        {
            // rename over an existing file can fail on some systems.

            std::filesystem::remove(usersFile(), error);

            std::filesystem::rename(tempName, usersFile(), error);
        }
    }

    static std::string validateUsername(const std::string& name)
    {
        if (name.size() < 3 || name.size() > 32)
        {
            return "Username must be 3 to 32 characters long.";
        }

        for (char c : name)
        {
            bool ok =
                std::isalnum(static_cast<unsigned char>(c)) ||
                c == '.' || c == '_' || c == '-';

            if (!ok)
            {
                return "Username may only contain letters, digits, "
                    "'.', '_' and '-'.";
            }
        }

        return "";
    }

    static std::string validatePassword(const std::string& password)
    {
        if (password.size() < 8)
        {
            return "Password must be at least 8 characters long.";
        }

        if (password.size() > 128)
        {
            return "Password is too long (max 128 characters).";
        }

        return "";
    }

    // Salted, iterated SHA-256: slows down guessing if the file is
    // ever copied. Not a replacement for a vetted KDF (bcrypt /
    // Argon2) in a production system - fine for this internal tool.

    static std::string hashPassword(
        const std::string& password,
        const std::string& salt)
    {
        std::string digest = Sha256::raw(salt + password);

        for (int i = 0; i < 20000; i++)
        {
            digest = Sha256::raw(digest + salt + password);
        }

        return Sha256::hex(digest);
    }

    static bool constantTimeEquals(
        const std::string& a,
        const std::string& b)
    {
        if (a.size() != b.size())
        {
            return false;
        }

        unsigned char diff = 0;

        for (size_t i = 0; i < a.size(); i++)
        {
            diff |= static_cast<unsigned char>(a[i] ^ b[i]);
        }

        return diff == 0;
    }

    static std::string randomHex(size_t bytes)
    {
        std::random_device device;

        std::string raw(bytes, '\0');

        for (size_t i = 0; i < bytes; i++)
        {
            raw[i] = static_cast<char>(device() & 0xff);
        }

        return Sha256::hex(raw);
    }

    static std::string randomPassword(size_t length)
    {
        // No look-alike characters (0/O, 1/l/I).

        static const std::string alphabet =
            "abcdefghijkmnpqrstuvwxyzABCDEFGHJKLMNPQRSTUVWXYZ23456789";

        std::random_device device;

        std::string out;

        for (size_t i = 0; i < length; i++)
        {
            out.push_back(alphabet[device() % alphabet.size()]);
        }

        return out;
    }
};

} // namespace auth


// ================================================================
// CROW MIDDLEWARE
// ================================================================
// Runs before every route (see crow::App<AuthMiddleware> in
// WebServer.cpp).

struct AuthMiddleware
{
    struct context
    {
    };

    void before_handle(
        crow::request& req,
        crow::response& res,
        context&)
    {
        auth::CurrentUser user;

        std::string token =
            auth::AuthManager::cookieValue(
                req.get_header_value("Cookie"),
                "wms_session");

        bool loggedIn =
            auth::AuthManager::instance().userForToken(token, user);

        bool isRead =
            req.method == crow::HTTPMethod::GET ||
            req.method == crow::HTTPMethod::HEAD;

        std::string message;

        int status =
            auth::AuthManager::decide(
                req.url,
                isRead,
                loggedIn,
                user.role,
                message);

        if (status != 0)
        {
            res.code = status;

            res.set_header("Content-Type", "text/plain");

            res.write(message);

            res.end();
        }
    }

    void after_handle(
        crow::request&,
        crow::response&,
        context&)
    {
    }
};
