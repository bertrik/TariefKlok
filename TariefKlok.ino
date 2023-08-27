#include <Arduino.h>

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

static void show_help(const cmd_t * cmds)
{
    for (const cmd_t * cmd = cmds; cmd->cmd != NULL; cmd++) {
        printf("%10s: %s\n", cmd->name, cmd->help);
    }
}

static int do_help(int argc, char *argv[]);

const cmd_t commands[] = {
    { "help", do_help, "Show help" },
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

