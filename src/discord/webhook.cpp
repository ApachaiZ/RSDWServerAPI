#include "webhook.h"
#include "../utils/json.h"
#include "../utils/logger.h"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <deque>
#include <mutex>
#include <thread>

namespace Discord {

namespace {

bool UrlIsSafe(const std::string& url, bool allowHttp) {
    if (url.rfind("https://", 0) != 0 && !(allowHttp && url.rfind("http://", 0) == 0)) {
        return false;
    }
    for (char c : url) {
        if (c == '\'' || c == '"' || c == '`' || c == '$' || c == ';' || c == '|' ||
            c == '&' || c == '<' || c == '>' || c == '\\' || c == '\n' || c == '\r') {
            return false;
        }
    }
    return true;
}

bool PathIsSafe(const std::string& path) {
    for (char c : path) {
        if (c == '\'' || c == '"' || c == '`' || c == '$' || c == ';' || c == '|' ||
            c == '&' || c == '<' || c == '>' || c == '\\' || c == '\n' || c == '\r') {
            return false;
        }
    }
    return true;
}

std::string ShellQuote(const std::string& value) {
    std::string quoted = "'";
    for (char c : value) {
        if (c == '\'') {
            quoted += "'\\''";
        } else {
            quoted += c;
        }
    }
    quoted += "'";
    return quoted;
}

void RunCurl(const std::string& command) {
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) {
        LogMessage("Discord: could not run curl");
        return;
    }
    pclose(pipe);
}

struct Item {
    std::string content;
    std::string gifPath;
    std::string payload;  // JSON Discord complet pré-formaté (embeds) — prioritaire
};

struct Sender {
    explicit Sender(const char* senderName) : name(senderName) {}

    std::string name;
    std::string url;
    std::string username = "Dragonwilds";
    bool discordMode = true;
    std::atomic<bool> enabled{false};
    std::atomic<bool> running{false};
    std::thread* worker = nullptr;

    std::mutex mutex;
    std::condition_variable signal;
    std::deque<Item> queue;

    void Send(const Item& item) {
        // Cible non-Discord (webhook JSON générique) : contenu brut tel quel.
        if (!discordMode) {
            std::string command = "curl -s -o /dev/null -m 10 -X POST "
                                  "-H 'Content-Type: application/json' -d " +
                                  ShellQuote(item.content) + " '" + url + "'";
            RunCurl(command);
            return;
        }

        if (!item.payload.empty()) {
            std::string command = "curl -s -o /dev/null -m 10 -X POST "
                                  "-H 'Content-Type: application/json' -d " +
                                  ShellQuote(item.payload) + " '" + url + "'";
            RunCurl(command);
            return;
        }

        std::string payload = "{\"username\":\"" + Json::Escape(username) +
                              "\",\"content\":\"" + Json::Escape(item.content) +
                              "\",\"allowed_mentions\":{\"parse\":[]}}";

        if (item.gifPath.empty()) {
            std::string command = "curl -s -o /dev/null -m 10 -X POST "
                                  "-H 'Content-Type: application/json' -d " +
                                  ShellQuote(payload) + " '" + url + "'";
            RunCurl(command);
            return;
        }

        // Discord multipart: JSON payload + GIF attachment.
        std::string command = "curl -s -o /dev/null -m 15 -X POST " +
                              std::string("-F ") + ShellQuote("payload_json=" + payload) +
                              " -F " + ShellQuote("file=@" + item.gifPath + ";type=image/gif") +
                              " '" + url + "'";
        RunCurl(command);
    }

    void Run() {
        while (running) {
            Item item;
            {
                std::unique_lock<std::mutex> lock(mutex);
                signal.wait_for(lock, std::chrono::seconds(2),
                                [this] { return !queue.empty() || !running; });
                if (!running && queue.empty()) return;
                if (queue.empty()) continue;

                item = queue.front();
                queue.pop_front();
            }

            Send(item);

            for (int i = 0; i < 10 && running; i++) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
    }

    void Start(const std::string& webhookUrl, const std::string& webhookUsername,
               bool allowHttp) {
        if (enabled) return;
        if (webhookUrl.empty()) return;

        if (!UrlIsSafe(webhookUrl, allowHttp)) {
            LogMessage("Discord: " + name + " webhook rejected, it must be a safe http(s) URL");
            return;
        }

        url = webhookUrl;
        discordMode = url.find("discord.com/api/webhooks") != std::string::npos;
        if (!webhookUsername.empty()) username = webhookUsername;

        enabled = true;
        running = true;
        worker = new std::thread([this] { Run(); });

        LogMessage("Discord: " + name + " webhook enabled");
    }

    void Stop() {
        if (!running) return;
        running = false;
        signal.notify_all();

        if (worker && worker->joinable()) {
            worker->join();
            delete worker;
            worker = nullptr;
        }
        enabled = false;
    }

    void Post(const std::string& line, const std::string& gifPath = "",
              const std::string& payload = "") {
        if (!enabled || (line.empty() && payload.empty())) return;
        if (!gifPath.empty() && !PathIsSafe(gifPath)) {
            LogMessage("Discord: " + name + " GIF path rejected (unsafe characters)");
            return;
        }

        std::lock_guard<std::mutex> lock(mutex);
        if (queue.size() >= 200) queue.pop_front();
        queue.push_back({line, gifPath, payload});
        signal.notify_all();
    }
};

Sender g_Chat{"chat"};
Sender g_Deaths{"death"};
Sender g_Events{"events"};

Sender& Get(Channel channel) {
    return channel == Channel::Deaths ? g_Deaths : g_Chat;
}

}

bool IsEnabled(Channel channel) { return Get(channel).enabled; }

void Start(Channel channel, const std::string& webhookUrl, const std::string& username) {
    Get(channel).Start(webhookUrl, username, /*allowHttp=*/false);
}

void StartEvents(const std::string& url) {
    g_Events.Start(url, "", /*allowHttp=*/true);
}

void Stop() {
    g_Chat.Stop();
    g_Deaths.Stop();
    g_Events.Stop();
}

void Post(Channel channel, const std::string& line) {
    Get(channel).Post(line);
}

void PostDeath(const std::string& line, const std::string& gifPath) {
    g_Deaths.Post(line, gifPath);
}

void PostEvent(const std::string& jsonLine, const std::string& discordPayload) {
    g_Events.Post(jsonLine, "", discordPayload);
}

}
