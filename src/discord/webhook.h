#pragma once
#include <string>

namespace Discord {

enum class Channel { Chat, Deaths };

void Start(Channel channel, const std::string& webhookUrl, const std::string& username);
void StartEvents(const std::string& url);
void Stop();
bool IsEnabled(Channel channel);

void Post(Channel channel, const std::string& line);
void PostDeath(const std::string& line, const std::string& gifPath);
void PostEvent(const std::string& jsonLine, const std::string& discordPayload);

}
