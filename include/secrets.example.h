#ifndef SECRETS_EXAMPLE_H
#define SECRETS_EXAMPLE_H

struct WifiCred
{
    const char *ssid;
    const char *pass;
};

static const WifiCred wifiCreds[] = {
    {"SSID_1", "PASSWORD_1"},
    {"SSID_2", "PASSWORD_2"},
    {"SSID_3", "PASSWORD_3"}};

static const size_t wifiCredsCount = sizeof(wifiCreds) / sizeof(wifiCreds[0]);

#endif
