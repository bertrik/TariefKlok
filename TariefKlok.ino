#include <Arduino.h>

#include <ESP8266HTTPClient.h>

#include "WiFiManager.h"
#include <ArduinoJson.h>
#include <FastLED.h>

#include "editline.h"
#include "cmdproc.h"

#define NUM_LEDS        24

#define PIN_LED_OUT    D3       // to DIN on the LED strip
#define PIN_LED_IN     D2       // to DOUT on the LED strip

#define printf Serial.printf

static WiFiManager wifiManager;
static WiFiClient wifiClient;
static char line[100];
static CRGB leds[NUM_LEDS];
static char espid[64];

static void show_help(const cmd_t * cmds)
{
    for (const cmd_t * cmd = cmds; cmd->cmd != NULL; cmd++) {
        printf("%10s: %s\n", cmd->name, cmd->help);
    }
}

static int do_help(int argc, char *argv[]);

static bool fetch_url(const char *host, int port, const char *path, String & response)
{
    HTTPClient httpClient;
    httpClient.begin(wifiClient, host, port, path, false);
    httpClient.setTimeout(20000);
    httpClient.setUserAgent(espid);

    printf("> GET http://%s:%d%s\n", host, port, path);
    int res = httpClient.GET();

    // evaluate result
    bool result = (res == HTTP_CODE_OK);
    response = result ? httpClient.getString() : httpClient.errorToString(res);
    httpClient.end();
    printf("< %d: %s\n", res, response.c_str());
    return result;
}

static int do_get(int argc, char *argv[])
{
    String response;
    if (fetch_url("stofradar.nl", 9001, "/electricity/price", response)) {
        printf("fetched!\n");
    }

    return CMD_OK;
}

const cmd_t commands[] = {
    { "help", do_help, "Show help" },
    { "get", do_get, "[url] GET URL" },
    { NULL, NULL, NULL }
};

static int do_help(int argc, char *argv[])
{
    show_help(commands);
    return CMD_OK;
}

void setup(void)
{
    Serial.begin(115200);
    Serial.println("\nTariefKlok");

    EditInit(line, sizeof(line));
    snprintf(espid, sizeof(espid), "esp8266-tariefklok-%06x", ESP.getChipId());

    FastLED.addLeds < WS2812B, PIN_LED_OUT, GRB > (leds, NUM_LEDS).setCorrection(TypicalSMD5050);

    wifiManager.autoConnect("ESP-TARIEFKLOK");
}

void loop(void)
{
    // parse command line
    while (Serial.available()) {
        char c = Serial.read();
        bool haveLine = EditLine(c, &c);
        Serial.write(c);
        if (haveLine) {
            int result = cmd_process(commands, line);
            switch (result) {
            case CMD_OK:
                printf("OK\n");
                break;
            case CMD_NO_CMD:
                break;
            case CMD_UNKNOWN:
                printf("Unknown command, available commands:\n");
                show_help(commands);
                break;
            default:
                printf("%d\n", result);
                break;
            }
            printf(">");
        }
    }
}
