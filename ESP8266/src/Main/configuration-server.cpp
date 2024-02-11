#include "./configuration-server.h"
#include "./server-page-html.h"

void setupConfigurationServer();

void ConfigurationServer::setup() {
  this->server.on("/", [this]() {
    String content = CONFIG_PAGE_HTML(this->config);
    
    this->server.send(200, "text/html", content);  
  });

  this->server.on("/settings", [this]() {
    auto tzHourOffsetStr = server.arg("tz-hour-offset");
    auto tzMinOffsetStr = server.arg("tz-minutes-offset");
    auto daylightSavingStr = server.arg("daylight-saving");

    auto brightnessStr = server.arg("brightness");
    auto decreaseBrightnessStr = server.arg("decrease-brightness");

    auto tzHourOffset = tzHourOffsetStr.toInt();
    auto tzMinOffset = tzMinOffsetStr.toInt();
    auto daylightSaving = daylightSavingStr != NULL;

    auto brightness = brightnessStr.toInt();
    auto decreaseBrightnessDuringBight = decreaseBrightnessStr != NULL;

    Serial.print("TZ Hour Offset: ");
    Serial.println(tzHourOffset);

    Serial.print("TZ Minutes Offset: ");
    Serial.println(tzMinOffset);

    Serial.print("Daylight saving: ");
    Serial.println(daylightSaving);

    Serial.print("Brightness: ");
    Serial.println(brightness);

    Serial.print("Decrease bright during night: ");
    Serial.println(decreaseBrightnessDuringBight);

    if(tzHourOffset >= -12 && tzHourOffset <= 12) {
      this->config.timezoneHoursOffset = tzHourOffset;
      this->mustUpdateTime = true;
    }

    if(tzMinOffset >= 0 && tzMinOffset <= 59) {
      this->config.timezoneMinutesOffset = tzMinOffset;
      this->mustUpdateTime = true;
    }

    if(brightness > 0 && brightness <= 255) {
      this->config.displayBrightness = (uint8) brightness;
    }

    this->config.isInDaylightSaving = daylightSaving;
    this->config.decreaseBrightnessDuringNight = decreaseBrightnessDuringBight;

    String content = SAVED_PAGE_HTML;
    this->server.send(200, "text/html", content);
  });

  this->server.begin();
}

bool ConfigurationServer::handleClient() {
  this->server.handleClient();

  auto result = this->mustUpdateTime;
  this->mustUpdateTime = false;

  return result;
}

